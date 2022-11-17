//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/EBus/EBus.h>
#include <AzToolsFramework/AssetBrowser/AssetBrowserBus.h>
#include <AzToolsFramework/AssetBrowser/Previewer/PreviewerBus.h>

#if defined(O3DE_USE_PK)
#include "Previewer/PopcornFXEffectPreviewerFactory.h"
#endif

namespace PopcornFX {

	class PopcornFXPreviewersSystemComponent
		: public AZ::Component
		, public AzToolsFramework::AssetBrowser::AssetBrowserInteractionNotificationBus::Handler
		, private AzToolsFramework::AssetBrowser::PreviewerRequestBus::Handler
	{
	public:
		AZ_COMPONENT(PopcornFXPreviewersSystemComponent, "{BD8742C8-E9EF-4184-B5BB-8A33D7D71D72}");

		PopcornFXPreviewersSystemComponent() = default;
		~PopcornFXPreviewersSystemComponent() override = default;

		static void	Reflect(AZ::ReflectContext *context) { AZ_UNUSED(context); }
		static void	GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided) { AZ_UNUSED(provided); }
		static void	GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible) { AZ_UNUSED(incompatible); }
		static void	GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required) { AZ_UNUSED(required); }
		static void	GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &dependent) { AZ_UNUSED(dependent); }

	private:
		AZStd::unique_ptr<const PopcornFXEffectPreviewerFactory>	m_PreviewerFactory;

		PopcornFXPreviewersSystemComponent(const PopcornFXPreviewersSystemComponent&) = delete;

	protected:
		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void	Init() override;
		void	Activate() override;
		void	Deactivate() override;
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// AssetBrowserInteractionNotificationBus::Handler
		AzToolsFramework::AssetBrowser::SourceFileDetails		GetSourceFileDetails(const char *fullSourceFileName) override;
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// AzToolsFramework::AssetBrowser::PreviewerRequestBus::Handler
		const AzToolsFramework::AssetBrowser::PreviewerFactory	*GetPreviewerFactory(const AzToolsFramework::AssetBrowser::AssetBrowserEntry *entry) const override;
		////////////////////////////////////////////////////////////////////////
	};

}

