//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerText.h"

#include <AzCore/Component/Component.h>

namespace PopcornFX {

	class PopcornFXSamplerTextGameComponent
		: public AZ::Component
	{
	public:
		AZ_COMPONENT(PopcornFXSamplerTextGameComponent, "{10528930-6E0B-402E-82BA-E9630F4E6E89}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);

		PopcornFXSamplerText	m_Sampler;
	};

}
