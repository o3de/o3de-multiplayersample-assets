//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerText.h"

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace PopcornFX {

	class PopcornFXSamplerTextEditorComponent
		: public AzToolsFramework::Components::EditorComponentBase
	{
	public:
		AZ_EDITOR_COMPONENT(PopcornFXSamplerTextEditorComponent, "{742B8AB2-DBBB-424F-AA8B-9BBFC4D8BDB3}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);

		void			BuildGameEntity(AZ::Entity *gameEntity) override;

	protected:
		PopcornFXSamplerText	m_Sampler;
	};

}
