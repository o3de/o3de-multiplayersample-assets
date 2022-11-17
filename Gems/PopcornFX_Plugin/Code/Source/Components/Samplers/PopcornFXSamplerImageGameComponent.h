//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerImage.h"

#include <AzCore/Component/Component.h>

namespace PopcornFX {

	class PopcornFXSamplerImageGameComponent
		: public AZ::Component
	{
	public:
		AZ_COMPONENT(PopcornFXSamplerImageGameComponent, "{DF86C118-606F-4133-ACEF-7983E63F4C80}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);

		PopcornFXSamplerImage	m_Sampler;
	};

}
