//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerTextEditorComponent.h"
#include "PopcornFXSamplerTextGameComponent.h"

#include <AzCore/Serialization/EditContext.h>

namespace PopcornFX {

	void	PopcornFXSamplerTextEditorComponent::Activate()
	{
		m_Sampler.m_AttachedToEntityId = m_entity->GetId();
		m_Sampler.Activate();
	}

	void	PopcornFXSamplerTextEditorComponent::Deactivate()
	{
		m_Sampler.Deactivate();
	}

	void	PopcornFXSamplerTextEditorComponent::Reflect(AZ::ReflectContext *reflection)
	{
		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXSamplerTextEditorComponent, AzToolsFramework::Components::EditorComponentBase>()
				->Version(1)
				->Field("Sampler", &PopcornFXSamplerTextEditorComponent::m_Sampler)
				;

			// edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXSamplerTextEditorComponent>("PopcornFX Sampler Text", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "PopcornFX Sampler Sampler")
						->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
						->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/PopcornFX_Sampler_Text_Icon.svg")
						->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/CParticleSamplerText.big.png")
						->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					->DataElement(0, &PopcornFXSamplerTextEditorComponent::m_Sampler, "", "")
						->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					;
			}
		}
	}

	void	PopcornFXSamplerTextEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		PopcornFXSamplerTextGameComponent::GetProvidedServices(provided);
	}

	void	PopcornFXSamplerTextEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		PopcornFXSamplerTextGameComponent::GetIncompatibleServices(incompatible);
	}

	void	PopcornFXSamplerTextEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		PopcornFXSamplerTextGameComponent::GetRequiredServices(required);
	}

	void	PopcornFXSamplerTextEditorComponent::BuildGameEntity(AZ::Entity *gameEntity)
	{
		PopcornFXSamplerTextGameComponent	*component = gameEntity->CreateComponent<PopcornFXSamplerTextGameComponent>();

		if (component)
		{
			component->m_Sampler.CopyFrom(m_Sampler);
		}
	}

}
