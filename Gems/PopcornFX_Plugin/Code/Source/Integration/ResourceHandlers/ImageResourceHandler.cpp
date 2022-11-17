//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "ImageResourceHandler.h"

#include <Atom/RPI.Public/RPIUtils.h>
#include <AzFramework/Asset/AssetSystemBus.h>
#include <pk_kernel/include/kr_file.h>
#include <pk_kernel/include/kr_refcounted_buffer.h>
#include <pk_defs.h>

namespace PopcornFX {

CImageResourceHandler::CImageResourceHandler()
{

}

CImageResourceHandler::~CImageResourceHandler()
{
}

void	*CImageResourceHandler::Load(	const CResourceManager	*resourceManager,
										u32						resourceTypeID,
										const CString			&resourcePath,
										bool					pathNotVirtual,
										const SResourceLoadCtl	&loadCtl,
										CMessageStream			&loadReport,
										SAsyncLoadStatus		*asyncLoadStatus)
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)pathNotVirtual;
	(void)loadCtl;

	PK_SCOPEDLOCK(m_Lock);

	if (resourcePath.Empty())
	{
		loadReport.ThrowError("resourcePath is empty");
		return null;
	}

	IFileSystem			*fs = File::DefaultFileSystem();
	CString				fullPath = pathNotVirtual ? resourcePath : fs->VirtualToPhysical(resourcePath, IFileSystem::Access_Read);
	AZStd::string_view	path = AZStd::string_view(fullPath.Data(), fullPath.Length());
	AzFramework::AssetSystem::AssetStatus status = AzFramework::AssetSystem::AssetStatus_Unknown;
	AzFramework::AssetSystemRequestBus::BroadcastResult(status, &AzFramework::AssetSystemRequestBus::Events::CompileAssetSync, path);

	if (!PK_VERIFY(status == AzFramework::AssetSystem::AssetStatus_Compiled))
	{
		loadReport.ThrowError("Could not compile mage at '%s'", path.data());
		return null;
	}

	AZ::Data::AssetId streamingImageAssetId;
	AZ::Data::AssetCatalogRequestBus::BroadcastResult(	streamingImageAssetId, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath,
														path.data(), azrtti_typeid<AZ::RPI::StreamingImageAsset>(), false);
	if (!streamingImageAssetId.IsValid())
	{
		loadReport.ThrowError("Failed to get streaming image asset id with path '%s'", path.data());
		return null;
	}

	SImageResource		*resource = m_AssetIdToImg.Find(streamingImageAssetId);

	// Asset already loaded:
	if (resource != null)
	{
		++resource->m_RefCount;
		return resource->m_Image.Get();
	}

	AZ::Data::Asset<AZ::RPI::StreamingImageAsset>	streamingImageAsset = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::StreamingImageAsset>(streamingImageAssetId, AZ::Data::AssetLoadBehavior::PreLoad);

	streamingImageAsset.BlockUntilLoadComplete();

	if (!streamingImageAsset.IsReady())
	{
		loadReport.ThrowError("Failed to get streaming image asset at path '%s'", path.data());
		return null;
	}

	const AZ::RHI::ImageDescriptor	&imgDesc = streamingImageAsset->GetImageDescriptor();

	if (!PK_VERIFY(imgDesc.m_dimension == AZ::RHI::ImageDimension::Image2D))
	{
		loadReport.ThrowError("Wrong number of dimensions (expect Image2D, is Image%uD) on image '%s'", (u32)imgDesc.m_dimension, path.data());
		return null;
	}

#if PK_O3DE_MAJOR_VERSION >= 2205
	AZStd::span<const uint8_t>			imgData = streamingImageAsset->GetSubImageData(0, 0);
#else
	AZStd::array_view<uint8_t>			imgData = streamingImageAsset->GetSubImageData(0, 0);
#endif
	AZ::RHI::Format						imgFormat = imgDesc.m_format;
	AZ::RHI::Size						imgSize = imgDesc.m_size;

	CImage::EFormat						pkImageFormat = CImage::Format_Invalid;
	u32									pkImageFlags = 0;
	ToPkImageFormatAndFlags(imgFormat, pkImageFormat, pkImageFlags);

	if (pkImageFormat == CImage::Format_Invalid)
	{
		loadReport.ThrowError("Could not find matching PopcornFX format for image '%s' (format %u)", path.data(), (u32)imgFormat);
		return null;
	}

	PImage		loadedImage = PK_NEW(CImage);

	if (!PK_VERIFY(loadedImage != null))
		return null;

	if (!PK_VERIFY(loadedImage->m_Frames.PushBack().Valid()))
		return null;

	loadedImage->m_Flags = pkImageFlags;
	loadedImage->m_Format = pkImageFormat;

	CImageFrame		&frame = loadedImage->m_Frames.Last();

	if (!PK_VERIFY(frame.m_Mipmaps.PushBack().Valid()))
		return null;

	CImageMap		&map = frame.m_Mipmaps.Last();

	map.m_Dimensions = CUint3(imgSize.m_width, imgSize.m_height, imgSize.m_depth);
	map.m_RawBuffer = CRefCountedMemoryBuffer::AllocAligned(static_cast<u32>(imgData.size()));

	Mem::Copy(map.m_RawBuffer->Data<void>(), imgData.data(), imgData.size());

	if (asyncLoadStatus != null)
	{
		asyncLoadStatus->m_Resource = loadedImage.Get();
		asyncLoadStatus->m_Done = true;
		asyncLoadStatus->m_Progress = 1.0f;
	}

	SImageResource	imgResource; 
	imgResource.m_Image = loadedImage;
	imgResource.m_RefCount = 1;

	resource = m_AssetIdToImg.Insert(streamingImageAssetId, imgResource);
	if (!PK_VERIFY(resource != null))
		return null;

	if (!PK_VERIFY(m_ImgToAssetId.Insert(loadedImage.Get(), streamingImageAssetId) != null))
		return null;

	return loadedImage.Get();
}

void	*CImageResourceHandler::Load(	const CResourceManager	*resourceManager,
										u32						resourceTypeID,
										const CFilePackPath		&resourcePath,
										const SResourceLoadCtl	&loadCtl,
										CMessageStream			&loadReport,
										SAsyncLoadStatus		*asyncLoadStatus)
{
	return Load(resourceManager, resourceTypeID, resourcePath.Path(), false, loadCtl, loadReport, asyncLoadStatus);
}

void	CImageResourceHandler::Unload(	const CResourceManager	*resourceManager,
										u32						resourceTypeID,			// used to check we are called with the correct type
										void					*resource)
{
	(void)resourceManager;
	(void)resourceTypeID;
	PK_SCOPEDLOCK(m_Lock);

	CImage						*toUnload = static_cast<CImage*>(resource);
	const AZ::Data::AssetId		*imgId = m_ImgToAssetId.Find(toUnload);

	if (!PK_VERIFY(imgId != null))
	{
		CLog::Log(PK_WARN, "CImageResourceHandler::Unload: Could not find image asset Id");
		return;
	}

	SImageResource	*imageResource = m_AssetIdToImg.Find(*imgId);

	if (!PK_VERIFY(imageResource != null))
	{
		CLog::Log(PK_WARN, "CImageResourceHandler::Unload: Could not find image resource");
		return;
	}

	--imageResource->m_RefCount;

	if (imageResource->m_RefCount == 0)
	{
		PK_VERIFY(m_AssetIdToImg.Remove(*imgId));
		PK_VERIFY(m_ImgToAssetId.Remove(toUnload));
	}
}

void	CImageResourceHandler::AppendDependencies(	const CResourceManager		*resourceManager,
													u32							resourceTypeID,
													void						*resource,
													PopcornFX::TArray<CString>	&outResourcePaths) const
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)resource;
	(void)outResourcePaths;
}

void	CImageResourceHandler::AppendDependencies(	const CResourceManager		*resourceManager,
													u32							resourceTypeID,
													const CString				&resourcePath,
													bool						pathNotVirtual,
													PopcornFX::TArray<CString>	&outResourcePaths) const
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)resourcePath;
	(void)pathNotVirtual;
	(void)outResourcePaths;
}

void	CImageResourceHandler::AppendDependencies(	const CResourceManager		*resourceManager,
													u32							resourceTypeID,
													const CFilePackPath			&resourcePath,
													PopcornFX::TArray<CString>	&outResourcePaths) const
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)resourcePath;
	(void)outResourcePaths;
}

void	CImageResourceHandler::BroadcastResourceChanged(const CResourceManager *resourceManager, const CFilePackPath &resourcePath)
{
	(void)resourceManager;
	(void)resourcePath;
}

CImageResourceHandler::SAtomToPkFormat	CImageResourceHandler::m_AtomToPk[(u32)AZ::RHI::Format::Count] =
{
	{ AZ::RHI::Format::Unknown,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32B32A32_FLOAT,				CImage::Format_Fp32RGBA,		CImage::FF_Fp },
	{ AZ::RHI::Format::R32G32B32A32_UINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32B32A32_SINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32B32_FLOAT,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32B32_UINT,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32B32_SINT,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16B16A16_FLOAT,				CImage::Format_Fp16RGBA,		CImage::FF_Fp },
	{ AZ::RHI::Format::R16G16B16A16_UNORM,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16B16A16_UINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16B16A16_SNORM,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16B16A16_SINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32_FLOAT,					CImage::Format_Fp32LumAlpha,	CImage::FF_Fp },
	{ AZ::RHI::Format::R32G32_UINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32G32_SINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::D32_FLOAT_S8X24_UINT,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R10G10B10A2_UNORM,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R10G10B10A2_UINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R11G11B10_FLOAT,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8B8A8_UNORM,					CImage::Format_BGRA8,			0 },
	{ AZ::RHI::Format::R8G8B8A8_UNORM_SRGB,				CImage::Format_BGRA8_sRGB,		CImage::FF_sRGB },
	{ AZ::RHI::Format::R8G8B8A8_UINT,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8B8A8_SNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8B8A8_SINT,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16_FLOAT,					CImage::Format_Fp16LumAlpha,	CImage::FF_Fp },
	{ AZ::RHI::Format::R16G16_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16_UINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16_SNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16G16_SINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::D32_FLOAT,						CImage::Format_Fp32Lum,			CImage::FF_Fp },
	{ AZ::RHI::Format::R32_FLOAT,						CImage::Format_Fp32Lum,			CImage::FF_Fp },
	{ AZ::RHI::Format::R32_UINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R32_SINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::D24_UNORM_S8_UINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8_UNORM,						CImage::Format_LumAlpha8,		0 },
	{ AZ::RHI::Format::R8G8_UNORM_SRGB,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8_UINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8_SNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8_SINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16_FLOAT,						CImage::Format_Fp16Lum,			CImage::FF_Fp },
	{ AZ::RHI::Format::D16_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16_UINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16_SNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R16_SINT,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8_UNORM,						CImage::Format_Lum8,			0 },
	{ AZ::RHI::Format::R8_UNORM_SRGB,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8_UINT,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8_SNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8_SINT,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::A8_UNORM,						CImage::Format_Lum8,			0 },
	{ AZ::RHI::Format::R1_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R9G9B9E5_SHAREDEXP,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R8G8_B8G8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::G8R8_G8B8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC1_UNORM,						CImage::Format_DXT1,			CImage::FF_Compressed | CImage::FF_Compressed_DXT },
	{ AZ::RHI::Format::BC1_UNORM_SRGB,					CImage::Format_DXT1_sRGB,		CImage::FF_Compressed | CImage::FF_Compressed_DXT | CImage::FF_sRGB },
	{ AZ::RHI::Format::BC2_UNORM,						CImage::Format_DXT3,			CImage::FF_Compressed | CImage::FF_Compressed_DXT },
	{ AZ::RHI::Format::BC2_UNORM_SRGB,					CImage::Format_DXT3_sRGB,		CImage::FF_Compressed | CImage::FF_Compressed_DXT | CImage::FF_sRGB },
	{ AZ::RHI::Format::BC3_UNORM,						CImage::Format_DXT5,			CImage::FF_Compressed | CImage::FF_Compressed_DXT },
	{ AZ::RHI::Format::BC3_UNORM_SRGB,					CImage::Format_DXT5_sRGB,		CImage::FF_Compressed | CImage::FF_Compressed_DXT | CImage::FF_sRGB },
	{ AZ::RHI::Format::BC4_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC4_SNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC5_UNORM,						CImage::Format_BC5_UNorm,		CImage::FF_Compressed },
	{ AZ::RHI::Format::BC5_SNORM,						CImage::Format_BC5_SNorm,		CImage::FF_Compressed },
	{ AZ::RHI::Format::B5G6R5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B5G5R5A1_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::A1B5G5R5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B8G8R8A8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B8G8R8X8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R10G10B10_XR_BIAS_A2_UNORM,		CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B8G8R8A8_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B8G8R8X8_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC6H_UF16,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC6H_SF16,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC7_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::BC7_UNORM_SRGB,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::AYUV,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::Y410,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::Y416,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::NV12,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::P010,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::P016,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::YUY2,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::Y210,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::Y216,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::NV11,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::AI44,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::IA44,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::P8,								CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::A8P8,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B4G4R4A4_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R4G4B4A4_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R10G10B10_7E3_A2_FLOAT,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R10G10B10_6E4_A2_FLOAT,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::D16_UNORM_S8_UINT,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::X16_TYPELESS_G8_UINT,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::P208,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::V208,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::V408,							CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::EAC_R11_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::EAC_R11_SNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::EAC_RG11_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::EAC_RG11_SNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ETC2_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ETC2_UNORM_SRGB,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ETC2A_UNORM,						CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ETC2A_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ETC2A1_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ETC2A1_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::PVRTC2_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::PVRTC2_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::PVRTC4_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::PVRTC4_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_4x4_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_4x4_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_5x4_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_5x4_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_5x5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_5x5_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_6x5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_6x5_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_6x6_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_6x6_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_8x5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_8x5_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_8x6_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_8x6_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_8x8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_8x8_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x5_UNORM_SRGB,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x6_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x6_UNORM_SRGB,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x8_UNORM_SRGB,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x10_UNORM,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_10x10_UNORM_SRGB,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_12x10_UNORM,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_12x10_UNORM_SRGB,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_12x12_UNORM,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::ASTC_12x12_UNORM_SRGB,			CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::A8B8G8R8_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::A8B8G8R8_UNORM_SRGB,				CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::A8B8G8R8_SNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::R5G6B5_UNORM,					CImage::Format_Invalid,			0 },
	{ AZ::RHI::Format::B8G8R8A8_SNORM,					CImage::Format_Invalid,			0 },
};

//----------------------------------------------------------------------------

void	ToPkImageFormatAndFlags(const AZ::RHI::Format &imgFormat, PopcornFX::CImage::EFormat &outPkImatFormat, u32 &outPkImageFlags)
{
	outPkImatFormat = CImage::Format_Invalid;
	outPkImageFlags = 0;

	for (u32 i = 0; i < (u32)AZ::RHI::Format::Count; ++i)
	{
		if (CImageResourceHandler::m_AtomToPk[i].m_AtFormat == imgFormat)
		{
			outPkImatFormat = CImageResourceHandler::m_AtomToPk[i].m_PkFormat;
			outPkImageFlags = CImageResourceHandler::m_AtomToPk[i].m_PkFlags;
			break;
		}
	}
}

//----------------------------------------------------------------------------

}
