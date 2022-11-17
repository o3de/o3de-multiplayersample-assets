//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXAsset.h"
#include "PopcornFXAssetHandler.h"

#include "Integration/PopcornFXIntegrationBus.h"

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_effect.h>
#endif

#define PKFX_EXT "pkfx"

namespace PopcornFX {

	bool	LoadFromBuffer(PopcornFXAsset *data, char *buffer, size_t bufferSize)
	{
		(void)data; (void)buffer; (void)bufferSize;
		return true;
	}

	PopcornFXAssetHandler::~PopcornFXAssetHandler()
	{
		Unregister();
	}

	AZ::Data::AssetPtr	PopcornFXAssetHandler::CreateAsset(const AZ::Data::AssetId &id, const AZ::Data::AssetType &type)
	{
		(void)type;
		AZ_Assert(type == AZ::AzTypeInfo<PopcornFXAsset>::Uuid(), "Invalid asset type! We handle only 'PopcornFXAsset'");

		if (!CanHandleAsset(id))
		{
			return null;
		}

		return aznew PopcornFXAsset;
	}

	AZ::Data::AssetHandler::LoadResult	PopcornFXAssetHandler::LoadAssetData(	const AZ::Data::Asset<AZ::Data::AssetData> &asset,
																				AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
																				[[maybe_unused]] const AZ::Data::AssetFilterCB &assetLoadFilterCB)
	{
		PopcornFXAsset *popcornFXAsset = asset.GetAs<PopcornFXAsset>();
		if (!popcornFXAsset)
		{
			AZ_Error(
				"PopcornFX Asset", false,
				"This should be a PopcornFX Asset, as this is the only type we process.");
			return AZ::Data::AssetHandler::LoadResult::Error;
		}

		const char				*assetPath = stream->GetFilename();
		const AZ::IO::SizeType	size = stream->GetLength();

		AZStd::vector<AZ::u8> assetData(size);
		stream->Read(size, assetData.data());

		bool loaded = false;
		PopcornFX::PopcornFXLoadBus::BroadcastResult(loaded, &PopcornFX::PopcornFXLoadBus::Handler::LoadEffect, popcornFXAsset, assetPath, assetData.data(), size);

		return loaded ? AZ::Data::AssetHandler::LoadResult::LoadComplete : AZ::Data::AssetHandler::LoadResult::Error;
	}

	void	PopcornFXAssetHandler::DestroyAsset(AZ::Data::AssetPtr ptr)
	{
		PopcornFXAsset		*data = static_cast<PopcornFXAsset*>(ptr);

		PopcornFX::PopcornFXLoadBus::Broadcast(&PopcornFX::PopcornFXLoadBus::Handler::UnloadEffect, data);

		delete ptr;
	}

	void	PopcornFXAssetHandler::GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType> &assetTypes)
	{
		assetTypes.push_back(AZ::AzTypeInfo<PopcornFXAsset>::Uuid());
	}

	void	PopcornFXAssetHandler::Register()
	{
		AZ_Assert(AZ::Data::AssetManager::IsReady(), "Asset manager isn't ready!");
		AZ::Data::AssetManager::Instance().RegisterHandler(this, AZ::AzTypeInfo<PopcornFXAsset>::Uuid());

		AZ::AssetTypeInfoBus::Handler::BusConnect(AZ::AzTypeInfo<PopcornFXAsset>::Uuid());
	}

	void	PopcornFXAssetHandler::Unregister()
	{
		AZ::AssetTypeInfoBus::Handler::BusDisconnect();

		if (AZ::Data::AssetManager::IsReady())
		{
			AZ::Data::AssetManager::Instance().UnregisterHandler(this);
		}
	}

	AZ::Data::AssetType	PopcornFXAssetHandler::GetAssetTypeStatic()
	{
		return azrtti_typeid<PopcornFXAsset>();
	}

	AZ::Data::AssetType	PopcornFXAssetHandler::GetAssetType() const
	{
		return AZ::AzTypeInfo<PopcornFXAsset>::Uuid();
	}

	const char	*PopcornFXAssetHandler::GetAssetTypeDisplayName() const
	{
		return "PopcornFXParticles";
	}

	const char	*PopcornFXAssetHandler::GetGroup() const
	{
		return "PopcornFXParticles";
	}

	const char	*PopcornFXAssetHandler::GetBrowserIcon() const
	{
		return "Gems/PopcornFX/Assets/Icons/PKFX.png";
	}

	AZ::Uuid PopcornFXAssetHandler::GetComponentTypeId() const
	{
		return AZ::Uuid("{B62ED02E-731B-4ACD-BCA1-78EF92528228}");
	}

	void	PopcornFXAssetHandler::GetAssetTypeExtensions(AZStd::vector<AZStd::string> &extensions)
	{
		extensions.push_back(PKFX_EXT);
	}

	bool	PopcornFXAssetHandler::CanHandleAsset(const AZ::Data::AssetId &id) const
	{
		// Look up the asset path to ensure it's actually a pkfx
		AZStd::string assetPath;
		EBUS_EVENT_RESULT(assetPath, AZ::Data::AssetCatalogRequestBus, GetAssetPathById, id);

		return strstr(assetPath.c_str(), "." PKFX_EXT) != null;
	}

}

