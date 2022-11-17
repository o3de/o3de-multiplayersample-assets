//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/Component/Component.h>

#include "PopcornFXTrackViewAttribute.h"

namespace PopcornFX {

	class PopcornFXTrackViewAttributeGameComponent
		: public AZ::Component
	{
	public:
		AZ_COMPONENT(PopcornFXTrackViewAttributeGameComponent, "{248BF7D0-4384-4CF1-9360-E8CF06AB3A6C}")

		friend class PopcornFXTrackViewAttributeEditorComponent;

		PopcornFXTrackViewAttributeGameComponent();
		~PopcornFXTrackViewAttributeGameComponent();

		// AZ::Component interface implementation
		void	Init() override;
		void	Activate() override;
		void	Deactivate() override;

		static void	GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
		{
			provided.push_back(AZ_CRC("PopcornFXTrackViewAttributeService"));
		}
		static void	GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &dependent)
		{
			(void)dependent;
		}
		static void	GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
		{
			(void)required;
		}
		static void	GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
		{
			incompatible.push_back(AZ_CRC("PopcornFXTrackViewAttributeService"));
		}
		static void	Reflect(AZ::ReflectContext *context);

	private:
		AZStd::string				m_AttributeName;
		PopcornFXTrackViewAttribute	m_TrackViewAttribute;
	};

}
