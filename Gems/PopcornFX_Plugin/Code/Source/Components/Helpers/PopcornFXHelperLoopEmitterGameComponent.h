//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <PopcornFX/PopcornFXBus.h>


namespace PopcornFX {

class PopcornFXHelperLoopEmitterGameComponent
	: public AZ::Component
	, public AZ::TickBus::Handler
{
public:
	AZ_COMPONENT(PopcornFXHelperLoopEmitterGameComponent, "{F1DF3F60-CD2D-421A-8CF6-AD9851D4C2B2}");

	PopcornFXHelperLoopEmitterGameComponent();

	void	Activate() override;
	void	Deactivate() override;

	static void	Reflect(AZ::ReflectContext *reflection);

	////////////////////////////////////////////////////////////////////////
	// AZ::TickBus::Handler interface implementation
	virtual void	OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
	////////////////////////////////////////////////////////////////////////

	static void	GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		provided.push_back(AZ_CRC("PopcornFXHelperLoopEmitterService"));
	}

	static void	GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		incompatible.push_back(AZ_CRC("PopcornFXHelperLoopEmitterService"));
	}

	static void	GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		required.push_back(AZ_CRC("PopcornFXEmitterService"));
	}

	static void	GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &/*dependent*/)
	{
	}

	float	m_Delay = 2.0f;
	bool	m_KillOnRestart = false;

protected:
	float	m_CurrentTime = 0.0f;
};

}
