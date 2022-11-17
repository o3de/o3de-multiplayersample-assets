//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "MeshResourceHandler.h"

#include <Atom/RPI.Public/Model/Model.h>
#include <AzFramework/Asset/AssetSystemBus.h>
#include <pk_defs.h>
#include <pk_kernel/include/kr_file.h>
#include <pk_kernel/include/kr_refcounted_buffer.h>

namespace PopcornFX {

CMeshResourceHandler::CMeshResourceHandler()
{
}

CMeshResourceHandler::~CMeshResourceHandler()
{
}

void	*CMeshResourceHandler::Load(	const CResourceManager	*resourceManager,
										u32						resourceTypeID,
										const CString			&resourcePath,
										bool					pathNotVirtual,
										const SResourceLoadCtl	&loadCtl,
										CMessageStream			&loadReport,
										SAsyncLoadStatus		*asyncLoadStatus)
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)loadCtl;

	PK_ASSERT(resourceTypeID == TResourceRouter<CResourceMesh>::ResourceTypeID());

	PK_SCOPEDLOCK(m_Lock);

	if (resourcePath.Empty())
	{
		loadReport.ThrowError("resourcePath is empty");
		return null;
	}

	IFileSystem	*fs = File::DefaultFileSystem();

	const CString	fullPath = pathNotVirtual ? resourcePath : fs->VirtualToPhysical(resourcePath, IFileSystem::Access_Read);

	AzFramework::AssetSystem::AssetStatus	status = AzFramework::AssetSystem::AssetStatus_Unknown;
	AzFramework::AssetSystemRequestBus::BroadcastResult(status, &AzFramework::AssetSystemRequestBus::Events::CompileAssetSync, fullPath.Data());

	if (!PK_VERIFY(status == AzFramework::AssetSystem::AssetStatus_Compiled))
	{
		loadReport.ThrowError("Could not compile mesh at '%s'", fullPath.Data());
		return null;
	}

	AZ::Data::AssetId	assetId;
	AZ::Data::AssetCatalogRequestBus::BroadcastResult(	assetId, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath,
														fullPath.Data(), AZ::RPI::ModelAsset::RTTI_Type(), false);
	if (!assetId.IsValid())
	{
		loadReport.ThrowError("Failed to get mesh asset id with path '%s'", fullPath.Data());
		return null;
	}

	// Asset already loaded:
	SMeshResource	*resource = m_AssetIdToMesh.Find(assetId);
	if (resource != null)
	{
		++resource->m_RefCount;
		return resource->m_Mesh.Get();
	}

	AZ::Data::Asset<AZ::RPI::ModelAsset>	modelAsset = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::ModelAsset>(
															assetId, AZ::Data::AssetLoadBehavior::PreLoad);
															modelAsset.BlockUntilLoadComplete();

	if (!modelAsset.IsReady())
	{
		loadReport.ThrowError("Failed to get the model asset at path '%s'", fullPath.Data());
		return null;
	}

	const AZ::Data::Asset<AZ::RPI::ModelLodAsset>	&model = modelAsset->GetLodAssets()[0];
	const AZ::RPI::ModelLodAsset::Mesh				&mesh = model->GetMeshes()[0];

	u32		totalVertexCount = u32(mesh.GetVertexCount());
	u32		totalIndexCount = u32(mesh.GetIndexCount());

	if (totalVertexCount == 0 || totalIndexCount == 0)
	{
		return null;
	}

#if PK_O3DE_MAJOR_VERSION >= 2205
	const AZStd::span<const u32>		srcIndices = mesh.GetIndexBufferTyped<u32>();
	const AZStd::span<const CFloat3>	srcPositions = mesh.GetSemanticBufferTyped<CFloat3>(AZ::Name("POSITION"));
	const AZStd::span<const CFloat3>	srcNormals = mesh.GetSemanticBufferTyped<CFloat3>(AZ::Name("NORMAL"));
	const AZStd::span<const CFloat2>	srcUvs = mesh.GetSemanticBufferTyped<CFloat2>(AZ::Name("UV"));
	const AZStd::span<const CFloat3>	srcTangents = mesh.GetSemanticBufferTyped<CFloat3>(AZ::Name("TANGENT"));
	const AZStd::span<const CFloat4>	srcColors = mesh.GetSemanticBufferTyped<CFloat4>(AZ::Name("COLOR"));
#else
	const AZStd::array_view<u32>		srcIndices = mesh.GetIndexBufferTyped<u32>();
	const AZStd::array_view<CFloat3>	srcPositions = mesh.GetSemanticBufferTyped<CFloat3>(AZ::Name("POSITION"));
	const AZStd::array_view<CFloat3>	srcNormals = mesh.GetSemanticBufferTyped<CFloat3>(AZ::Name("NORMAL"));
	const AZStd::array_view<CFloat2>	srcUvs = mesh.GetSemanticBufferTyped<CFloat2>(AZ::Name("UV"));
	const AZStd::array_view<CFloat3>	srcTangents = mesh.GetSemanticBufferTyped<CFloat3>(AZ::Name("TANGENT"));
	const AZStd::array_view<CFloat4>	srcColors = mesh.GetSemanticBufferTyped<CFloat4>(AZ::Name("COLOR"));
#endif

	if (srcIndices.empty() || srcPositions.empty())
	{
		return null;
	}

	SVertexDeclaration	decl;
	decl.AddStreamCodeIFN(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Position, SVStreamCode::Element_Float3, SVStreamCode::SIMD_Friendly));

	if (!srcNormals.empty())
		decl.AddStreamCodeIFN(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Normal, SVStreamCode::Element_Float3, SVStreamCode::SIMD_Friendly));

	if (!srcUvs.empty())
		decl.AddStreamCodeIFN(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Texcoord, SVStreamCode::Element_Float2, SVStreamCode::SIMD_Friendly));

	if (!srcTangents.empty())
		decl.AddStreamCodeIFN(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Tangent, SVStreamCode::Element_Float4, SVStreamCode::SIMD_Friendly));

	if (!srcColors.empty())
		decl.AddStreamCodeIFN(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Color, SVStreamCode::Element_Float4, SVStreamCode::SIMD_Friendly));

	PMeshNew	loadedMesh = PK_NEW(CMeshNew);
	if (!PK_VERIFY(loadedMesh != null))
		return null;

	CMeshTriangleBatch	&triBatch = loadedMesh->TriangleBatch();
	CMeshVStream		&vstream = triBatch.m_VStream;
	CMeshIStream		&istream = triBatch.m_IStream;

	PK_VERIFY(vstream.Reformat(decl));

	istream.SetPrimitiveType(CMeshIStream::Triangles);
	istream.Reformat(CMeshIStream::U32Indices);

	if (!PK_VERIFY(istream.Resize(totalIndexCount)) || !PK_VERIFY(vstream.Resize(totalVertexCount)))
		return null;

	PK_ASSERT(istream.IndexByteWidth() == CMeshIStream::U32Indices);
	u32	*dstIndices = reinterpret_cast<u32*>(istream.RawStreamForWriting());
	Mem::Copy(dstIndices, srcIndices.data(), totalIndexCount * sizeof(u32));

	const TStridedMemoryView<CFloat3>	dstPositions = vstream.Positions();
	PK_RELEASE_ASSERT(!dstPositions.Empty());

	TStridedMemoryView<CFloat3>	dstNormals;
	if (!srcNormals.empty())
	{
		dstNormals = vstream.Normals();
		PK_RELEASE_ASSERT(!dstNormals.Empty());
		PK_RELEASE_ASSERT(Mem::IsAligned<0x10>(dstNormals.Data()) & Mem::IsAligned<0x10>(dstNormals.Stride()));
	}

	TStridedMemoryView<CFloat2>	dstUvs;
	if (!srcUvs.empty())
	{
		dstUvs = vstream.Texcoords();
		PK_RELEASE_ASSERT(!dstUvs.Empty());
	}

	TStridedMemoryView<CFloat4>	dstTangents;
	if (!srcTangents.empty())
	{
		dstTangents = vstream.Tangents();
		PK_RELEASE_ASSERT(!dstTangents.Empty());
		PK_RELEASE_ASSERT(Mem::IsAligned<0x10>(dstTangents.Data()) & Mem::IsAligned<0x10>(dstTangents.Stride()));
	}

	TStridedMemoryView<CFloat4>	dstColors;
	if (!srcColors.empty())
		dstColors = vstream.Colors();

	const uint32_t	numVertices = mesh.GetVertexCount();
	for (uint32_t index = 0; index < numVertices; ++index)
	{
		dstPositions[index] = srcPositions[index];

		if (!dstNormals.Empty())
			dstNormals[index] = srcNormals[index];

		if (!dstUvs.Empty())
			dstUvs[index] = srcUvs[index];

		if (!dstTangents.Empty())
			dstTangents[index] = CFloat4(srcTangents[index], 1.f);

		if (!dstColors.Empty())
			dstColors[index] = srcColors[index];
	}

	PK_VERIFY(vstream.ExactResize(numVertices)); // shrink only

	loadedMesh->RebuildBBox();
	loadedMesh->BuildTangentsIFN();
	loadedMesh->SetupRuntimeStructsIFN();

#if (PK_GEOMETRICS_BUILD_MESH_SAMPLER_SURFACE != 0)
	loadedMesh->SetupDefaultSurfaceSamplingAccelStructsIFN(false);
#endif
#if (PK_GEOMETRICS_BUILD_MESH_SAMPLER_VOLUME != 0)
	loadedMesh->SetupDefaultVolumeSamplingAccelStructsIFN(false);
#endif

	PResourceMesh	resMesh = PK_NEW(CResourceMesh);
	if (!PK_VERIFY(resMesh != null))
		return null;

	if (asyncLoadStatus != null)
	{
		asyncLoadStatus->m_Resource = resMesh.Get();
		asyncLoadStatus->m_Done = true;
		asyncLoadStatus->m_Progress = 1.0f;
	}

	SMeshResource	meshResource;
	meshResource.m_Mesh = resMesh;
	meshResource.m_RefCount = 1;

	if (!PK_VERIFY(m_AssetIdToMesh.Insert(assetId, meshResource) != null) ||
		!PK_VERIFY(m_MeshToAssetId.Insert(resMesh.Get(), assetId) != null) ||
		!PK_VERIFY(resMesh->AddBatch("mat", loadedMesh).Valid()))
		return null;

	return resMesh.Get();
}

void	*CMeshResourceHandler::Load(	const CResourceManager	*resourceManager,
										u32						resourceTypeID,
										const CFilePackPath		&resourcePath,
										const SResourceLoadCtl	&loadCtl,
										CMessageStream			&loadReport,
										SAsyncLoadStatus		*asyncLoadStatus)
{
	return Load(resourceManager, resourceTypeID, resourcePath.Path(), false, loadCtl, loadReport, asyncLoadStatus);
}

void	CMeshResourceHandler::Unload(	const CResourceManager	*resourceManager,
										u32						resourceTypeID,		// used to check we are called with the correct type
										void					*resource)
{
	(void)resourceManager;
	(void)resourceTypeID;
	PK_SCOPEDLOCK(m_Lock);

	CResourceMesh	*toUnload = static_cast<CResourceMesh*>(resource);

	const AZ::Data::AssetId	*meshId = m_MeshToAssetId.Find(toUnload);

	if (!PK_VERIFY(meshId != null))
	{
		CLog::Log(PK_WARN, "CMeshResourceHandler::Unload: Could not find mesh asset Id");
		return;
	}

	SMeshResource	*meshResource = m_AssetIdToMesh.Find(*meshId);

	if (!PK_VERIFY(meshResource != null))
	{
		CLog::Log(PK_WARN, "CMeshResourceHandler::Unload: Could not find mesh resource");
		return;
	}

	--meshResource->m_RefCount;

	if (meshResource->m_RefCount == 0)
	{
		PK_VERIFY(m_AssetIdToMesh.Remove(*meshId));
		PK_VERIFY(m_MeshToAssetId.Remove(toUnload));
	}
}

void	CMeshResourceHandler::AppendDependencies(	const CResourceManager		*resourceManager,
													u32							resourceTypeID,
													void						*resource,
													PopcornFX::TArray<CString>	&outResourcePaths) const
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)resource;
	(void)outResourcePaths;
}

void	CMeshResourceHandler::AppendDependencies(	const CResourceManager		*resourceManager,
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

void	CMeshResourceHandler::AppendDependencies(	const CResourceManager		*resourceManager,
													u32							resourceTypeID,
													const CFilePackPath			&resourcePath,
													PopcornFX::TArray<CString>	&outResourcePaths) const
{
	(void)resourceManager;
	(void)resourceTypeID;
	(void)resourcePath;
	(void)outResourcePaths;
}

void	CMeshResourceHandler::BroadcastResourceChanged(const CResourceManager *resourceManager, const CFilePackPath &resourcePath)
{
	(void)resourceManager;
	(void)resourcePath;
}

}
