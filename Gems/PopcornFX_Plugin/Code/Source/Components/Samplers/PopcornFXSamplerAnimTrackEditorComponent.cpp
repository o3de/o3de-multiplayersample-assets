//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerAnimTrackEditorComponent.h"
#include "PopcornFXSamplerAnimTrackGameComponent.h"

#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace PopcornFX {

	void	PopcornFXSamplerAnimTrackEditorComponent::Activate()
	{
		m_Sampler.m_AttachedToEntityId = m_entity->GetId();
		m_Sampler.Activate();
	}

	void	PopcornFXSamplerAnimTrackEditorComponent::Deactivate()
	{
		m_Sampler.Deactivate();
	}

	void	PopcornFXSamplerAnimTrackEditorComponent::Reflect(AZ::ReflectContext *reflection)
	{
		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXSamplerAnimTrackEditorComponent, AzToolsFramework::Components::EditorComponentBase>()
				->Version(1)
				->Field("Sampler", &PopcornFXSamplerAnimTrackEditorComponent::m_Sampler)
				;

			// edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXSamplerAnimTrackEditorComponent>("PopcornFX Sampler AnimTrack", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "PopcornFX Sampler AnimTrack")
						->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
						->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/PopcornFX_Sampler_AnimTrack_Icon.svg")
						->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/CParticleSamplerAnimTrack.big.png")
						->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					->DataElement(0, &PopcornFXSamplerAnimTrackEditorComponent::m_Sampler, "", "")
						->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					;
			}
		}
	}

	void	PopcornFXSamplerAnimTrackEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		PopcornFXSamplerAnimTrackGameComponent::GetProvidedServices(provided);
	}

	void	PopcornFXSamplerAnimTrackEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		PopcornFXSamplerAnimTrackGameComponent::GetIncompatibleServices(incompatible);
	}

	void	PopcornFXSamplerAnimTrackEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		PopcornFXSamplerAnimTrackGameComponent::GetRequiredServices(required);
	}

	void	PopcornFXSamplerAnimTrackEditorComponent::BuildGameEntity(AZ::Entity *gameEntity)
	{
		PopcornFXSamplerAnimTrackGameComponent	*component = gameEntity->CreateComponent<PopcornFXSamplerAnimTrackGameComponent>();

		if (component)
		{
			component->m_Sampler.CopyFrom(m_Sampler);
		}
	}

}
