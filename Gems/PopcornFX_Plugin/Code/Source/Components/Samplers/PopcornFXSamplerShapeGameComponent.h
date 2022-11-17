//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSamplerShape.h"

#include <AzCore/Component/Component.h>

namespace PopcornFX {

	class PopcornFXSamplerShapeGameComponent
		: public AZ::Component
{
	public:
		AZ_COMPONENT(PopcornFXSamplerShapeGameComponent, "{B89FD122-D785-4196-B023-2EA940EACF01}");

		virtual void	Activate() override;
		virtual void	Deactivate() override;

		static void		Reflect(AZ::ReflectContext *reflection);
		static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
		static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
		static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);
		static void		GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &dependent);

		PopcornFXSamplerShape	m_Sampler;
	};

}
