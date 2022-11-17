//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerTextGameComponent.h"

#include <AzCore/Component/Entity.h>

namespace PopcornFX {

	void	PopcornFXSamplerTextGameComponent::Activate()
	{
		m_Sampler.m_AttachedToEntityId = m_entity->GetId();
		m_Sampler.Activate();
	}

	void	PopcornFXSamplerTextGameComponent::Deactivate()
	{
		m_Sampler.Deactivate();
	}

	void	PopcornFXSamplerTextGameComponent::Reflect(AZ::ReflectContext *reflection)
	{
		PopcornFXSamplerText::Reflect(reflection);

		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXSamplerTextGameComponent, AZ::Component>()
				->Version(1)
				->Field("Sampler", &PopcornFXSamplerTextGameComponent::m_Sampler)
				;
		}
	}

	void	PopcornFXSamplerTextGameComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		provided.push_back(AZ_CRC("PopcornFXSamplerTextService"));
	}

	void	PopcornFXSamplerTextGameComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		incompatible.push_back(AZ_CRC("PopcornFXEmitterService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerImageService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerShapeService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerAnimTrackService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerTextService"));
	}

	void	PopcornFXSamplerTextGameComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		(void)required;
	}

}
