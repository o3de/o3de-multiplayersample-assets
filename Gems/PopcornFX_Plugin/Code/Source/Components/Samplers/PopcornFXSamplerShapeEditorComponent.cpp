//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerShapeEditorComponent.h"
#include "PopcornFXSamplerShapeGameComponent.h"

#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace PopcornFX {

	void	PopcornFXSamplerShapeEditorComponent::Activate()
	{
		m_Sampler.m_AttachedToEntityId = m_entity->GetId();
		m_Sampler.Activate();
	}

	void	PopcornFXSamplerShapeEditorComponent::Deactivate()
	{
		m_Sampler.Deactivate();
	}

	void	PopcornFXSamplerShapeEditorComponent::Reflect(AZ::ReflectContext *reflection)
	{
		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXSamplerShapeEditorComponent, AzToolsFramework::Components::EditorComponentBase>()
				->Version(1)
				->Field("Sampler", &PopcornFXSamplerShapeEditorComponent::m_Sampler)
				;

			// edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXSamplerShapeEditorComponent>("PopcornFX Sampler Shape", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "PopcornFX Sampler Shape")
						->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
						->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/PopcornFX_Sampler_Shape_Icon.svg")
						->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/CParticleSamplerShape.big.png")
						->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					->DataElement(0, &PopcornFXSamplerShapeEditorComponent::m_Sampler, "", "")
						->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					;
			}
		}
	}

	void	PopcornFXSamplerShapeEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		PopcornFXSamplerShapeGameComponent::GetProvidedServices(provided);
	}

	void	PopcornFXSamplerShapeEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		PopcornFXSamplerShapeGameComponent::GetIncompatibleServices(incompatible);
	}

	void	PopcornFXSamplerShapeEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		PopcornFXSamplerShapeGameComponent::GetRequiredServices(required);
	}

	void	PopcornFXSamplerShapeEditorComponent::BuildGameEntity(AZ::Entity *gameEntity)
	{
		PopcornFXSamplerShapeGameComponent	*component = gameEntity->CreateComponent<PopcornFXSamplerShapeGameComponent>();

		if (component)
		{
			component->m_Sampler.CopyFrom(m_Sampler);
		}
	}

}
