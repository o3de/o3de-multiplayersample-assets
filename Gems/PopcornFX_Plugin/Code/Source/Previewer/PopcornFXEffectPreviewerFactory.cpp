//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "Source/Previewer/PopcornFXEffectPreviewer.h"
#include "Source/Previewer/PopcornFXEffectPreviewerFactory.h"
#include "Source/Asset/PopcornFXAsset.h"

#include <AzToolsFramework/AssetBrowser/AssetBrowserEntry.h>

namespace PopcornFX {

AzToolsFramework::AssetBrowser::Previewer *PopcornFXEffectPreviewerFactory::CreatePreviewer(QWidget *parent) const
{
	return new PopcornFXEffectPreviewer(parent);
}

bool PopcornFXEffectPreviewerFactory::IsEntrySupported(const AzToolsFramework::AssetBrowser::AssetBrowserEntry *entry) const
{
	using namespace AzToolsFramework::AssetBrowser;

	const AZ::Data::AssetType	effectType = azrtti_typeid<PopcornFXAsset>();

	switch (entry->GetEntryType())
	{
	case AssetBrowserEntry::AssetEntryType::Source:
	{
		const auto		source = azrtti_cast<const SourceAssetBrowserEntry*>(entry);
		const CString	extension(source->GetExtension().c_str());
		return extension.Compare(".pkfx", CaseInsensitive);
	}
	case AssetBrowserEntry::AssetEntryType::Product:
		const auto product = azrtti_cast<const ProductAssetBrowserEntry*>(entry);
		return product->GetAssetType() == effectType;
	}
	return false;
}

}
