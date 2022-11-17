//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerAnimTrack.h"

#include <AzCore/Component/Component.h>

namespace PopcornFX {

	class PopcornFXSamplerAnimTrackGameComponent
		: public AZ::Component
	{
	public:
		AZ_COMPONENT(PopcornFXSamplerAnimTrackGameComponent, "{96B23921-8763-4617-90A0-AD5F0F688A52}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);

		PopcornFXSamplerAnimTrack	m_Sampler;
	};

}
