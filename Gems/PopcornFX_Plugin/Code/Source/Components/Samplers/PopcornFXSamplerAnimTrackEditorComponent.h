//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerAnimTrack.h"

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace PopcornFX {

	class PopcornFXSamplerAnimTrackEditorComponent
		: public AzToolsFramework::Components::EditorComponentBase
	{
	public:
		AZ_EDITOR_COMPONENT(PopcornFXSamplerAnimTrackEditorComponent, "{9237AEB0-968E-4B36-8BB8-C7DEFB65D505}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);

		void			BuildGameEntity(AZ::Entity *gameEntity) override;

	protected:
		PopcornFXSamplerAnimTrack	m_Sampler;
	};

}
