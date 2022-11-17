//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <AzCore/Asset/AssetTypeInfoBus.h>

namespace PopcornFX {

class PopcornFXPreloader
	: public AZ::Data::AssetBus::MultiHandler
{
public:
	/**
	* Specifies that this class should use AZ::SystemAllocator for memory
	* management by default.
	*/
	AZ_CLASS_ALLOCATOR(PopcornFXPreloader, AZ::SystemAllocator, 0);

	void	Clear();

	bool	PreloadEffect(const AZ::Data::AssetId &assetId);
	bool	IsPreloadingDone();

	// AZ::Data::AssetBus::Handler
	void	OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
	void	OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

private:
	typedef AZ::Data::Asset<AZ::Data::AssetData> AssetType;
	typedef AZStd::unordered_map<AZ::Data::AssetId, AssetType> AssetMap;

	AssetMap				m_AssetsToPreload;
	AZStd::recursive_mutex	m_AssetsToPreloadMutex;
	AssetMap				m_PreloadedAssets;
	AZStd::recursive_mutex	m_PreloadedAssetsMutex;

	bool	PreloadAsset(const AZ::Data::AssetId &assetId, const AZ::Data::AssetType &assetType);
	bool	AssetToPreloadPending(const AZ::Data::AssetId &assetId);
	bool	AssetPreloaded(const AZ::Data::AssetId &assetId);

};

}

#endif //O3DE_USE_PK
