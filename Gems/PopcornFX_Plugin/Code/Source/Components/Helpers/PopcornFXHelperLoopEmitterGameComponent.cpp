//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXHelperLoopEmitterGameComponent.h"

#include <AzCore/Serialization/SerializeContext.h>

namespace PopcornFX {

PopcornFXHelperLoopEmitterGameComponent::PopcornFXHelperLoopEmitterGameComponent()
{
}

void	PopcornFXHelperLoopEmitterGameComponent::Activate()
{
	AZ::TickBus::Handler::BusConnect();
}

void	PopcornFXHelperLoopEmitterGameComponent::Deactivate()
{
	if (AZ::TickBus::Handler::BusIsConnected())
		AZ::TickBus::Handler::BusDisconnect();
}

void	PopcornFXHelperLoopEmitterGameComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	(void)time;

	bool enabled = false;
	EBUS_EVENT_ID_RESULT(enabled, GetEntityId(), PopcornFXEmitterComponentRequestBus, IsEnabled);
	if (!enabled)
	{
		m_CurrentTime = 0.0f;
		return;
	}

	m_CurrentTime += deltaTime;
	if (m_CurrentTime >= m_Delay)
	{
		EBUS_EVENT_ID(GetEntityId(), PopcornFXEmitterComponentRequestBus, Restart, m_KillOnRestart);
		m_CurrentTime = 0.0f;
	}
}

void	PopcornFXHelperLoopEmitterGameComponent::Reflect(AZ::ReflectContext *reflection)
{
	if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		// Serializer:
		serializeContext->Class<PopcornFXHelperLoopEmitterGameComponent, AZ::Component>()
			->Version(1)
			->Field("Delay", &PopcornFXHelperLoopEmitterGameComponent::m_Delay)
			->Field("KillOnRestart", &PopcornFXHelperLoopEmitterGameComponent::m_KillOnRestart)
			;
	}
}

}
