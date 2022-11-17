//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerAnimTrackGameComponent.h"

#include <AzCore/Component/Entity.h>

namespace PopcornFX {

	void	PopcornFXSamplerAnimTrackGameComponent::Activate()
	{
		m_Sampler.m_AttachedToEntityId = m_entity->GetId();
		m_Sampler.Activate();
	}

	void	PopcornFXSamplerAnimTrackGameComponent::Deactivate()
	{
		m_Sampler.Deactivate();
	}

	void	PopcornFXSamplerAnimTrackGameComponent::Reflect(AZ::ReflectContext *reflection)
	{
		PopcornFXSamplerAnimTrack::Reflect(reflection);

		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXSamplerAnimTrackGameComponent, AZ::Component>()
				->Version(1)
				->Field("Sampler", &PopcornFXSamplerAnimTrackGameComponent::m_Sampler)
				;
		}
	}

	void	PopcornFXSamplerAnimTrackGameComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		provided.push_back(AZ_CRC("PopcornFXSamplerAnimTrackService"));
	}

	void	PopcornFXSamplerAnimTrackGameComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		incompatible.push_back(AZ_CRC("PopcornFXEmitterService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerImageService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerShapeService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerAnimTrackService"));
		incompatible.push_back(AZ_CRC("PopcornFXSamplerTextService"));
	}

	void	PopcornFXSamplerAnimTrackGameComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		required.push_back(AZ_CRC("SplineService", 0x2b674d3c));
	}

}
