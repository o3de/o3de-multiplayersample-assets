//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzToolsFramework/AssetBrowser/Previewer/PreviewerFactory.h>

#include <QString>

namespace PopcornFX {

	class PopcornFXEffectPreviewerFactory final
		: public AzToolsFramework::AssetBrowser::PreviewerFactory
	{
	public:
		AZ_CLASS_ALLOCATOR(PopcornFXEffectPreviewerFactory, AZ::SystemAllocator, 0);

		PopcornFXEffectPreviewerFactory() = default;
		~PopcornFXEffectPreviewerFactory() = default;

		//! AzToolsFramework::AssetBrowser::PreviewerFactory overrides
		AzToolsFramework::AssetBrowser::Previewer	*CreatePreviewer(QWidget *parent = nullptr) const override;

		bool			IsEntrySupported(const AzToolsFramework::AssetBrowser::AssetBrowserEntry *entry) const override;
		const QString	&GetName() const override { return m_name; }

	private:
		QString	m_name = "PopcornFXEffectPreviewer";
	};

}
