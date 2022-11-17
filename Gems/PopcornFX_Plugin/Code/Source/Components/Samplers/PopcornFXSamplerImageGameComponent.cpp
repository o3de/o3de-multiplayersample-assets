//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerImageGameComponent.h"

#include <AzCore/Component/Entity.h>


namespace PopcornFX {

	void	PopcornFXSamplerImageGameComponent::Activate()
	{
		m_Sampler.m_AttachedToEntityId = m_entity->GetId();
		m_Sampler.Activate();
	}

	void	PopcornFXSamplerImageGameComponent::Deactivate()
	{
		m_Sampler.Deactivate();
	}

	void	PopcornFXSamplerImageGameComponent::Reflect(AZ::ReflectContext *reflection)
	{
		PopcornFXSamplerImage::Reflect(reflection);

		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXSamplerImageGameComponent, AZ::Component>()
				->Version(1)
				->Field("Sampler", &PopcornFXSamplerImageGameComponent::m_Sampler)
				;
		}
	}

	void	PopcornFXSamplerImageGameComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		provided.push_back(AZ_CRC("PopcornFXSamplerImageService"));
	}

	void	PopcornFXSamplerImageGameComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		incompatible.push_back(AZ_CRC("PopcornFXEmitterService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerImageService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerShapeService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerAnimTrackService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerTextService"));
	}

	void	PopcornFXSamplerImageGameComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		(void)required;
	}

}
