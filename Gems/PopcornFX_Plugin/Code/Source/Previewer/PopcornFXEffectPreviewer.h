//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Component/TickBus.h>
#include <AzToolsFramework/AssetBrowser/Previewer/Previewer.h>

#include <Atom/ImageProcessing/ImageObject.h>

#include <QWidget>
#include <QScopedPointer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#endif

namespace Ui
{
	class PopcornFXEffectPreviewerClass;
}

namespace AzToolsFramework
{
	namespace AssetBrowser
	{
		class ProductAssetBrowserEntry;
		class SourceAssetBrowserEntry;
		class AssetBrowserEntry;
	}
}

// Unscoped, otherwise the generated Qt widget is nested in PopcornFX
//namespace PopcornFX {

	class PopcornFXEffectPreviewer
		: public AzToolsFramework::AssetBrowser::Previewer
		, private AZ::SystemTickBus::Handler
	{
		Q_OBJECT
	public:
		AZ_CLASS_ALLOCATOR(PopcornFXEffectPreviewer, AZ::SystemAllocator, 0);

		explicit PopcornFXEffectPreviewer(QWidget *parent = nullptr);
		~PopcornFXEffectPreviewer();

		//! AzToolsFramework::AssetBrowser::Previewer overrides
		void			Clear() const override;
		void			Display(const AzToolsFramework::AssetBrowser::AssetBrowserEntry *entry) override;
		const QString	&GetName() const override { return m_name; }

	private:
		void	DisplayProduct(const AzToolsFramework::AssetBrowser::ProductAssetBrowserEntry *product);
		void	DisplaySource(const AzToolsFramework::AssetBrowser::SourceAssetBrowserEntry *source);

		void	DisplayTextureItem();
		template<class CreateFn>
		void	CreateAndDisplayTextureItemAsync(CreateFn create);

		void	PreviewSubImage(uint32_t mip);

		// SystemTickBus
		void	OnSystemTick() override;

		QScopedPointer<Ui::PopcornFXEffectPreviewerClass>	m_ui;
		QString												m_fileinfo;
		QString												m_name = "PopcornFXEffectPreviewer";

		// Decompressed image in preview. Cache it so we can preview its sub images
		ImageProcessingAtom::IImageObjectPtr				m_previewImageObject;

		// Properties for tracking the status of an asynchronous request to display an asset browser entry
		using CreateDisplayTextureResult = AZStd::pair<ImageProcessingAtom::IImageObjectPtr, QString>;

		QFuture<CreateDisplayTextureResult>	m_createDisplayTextureResult;
	};

//}
