//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerImage.h"

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace PopcornFX {

	class PopcornFXSamplerImageEditorComponent
		: public AzToolsFramework::Components::EditorComponentBase
	{
	public:
		AZ_EDITOR_COMPONENT(PopcornFXSamplerImageEditorComponent, "{D46ECF8E-8EBB-482C-AC41-A8393BAD525E}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);

		void BuildGameEntity(AZ::Entity *gameEntity) override;

	protected:
		PopcornFXSamplerImage	m_Sampler;
	};

}
