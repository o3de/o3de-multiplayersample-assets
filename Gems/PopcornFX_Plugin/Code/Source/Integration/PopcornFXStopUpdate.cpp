//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXStopUpdate.h"
#include "PopcornFXIntegration.h"

#if defined(O3DE_USE_PK)

namespace PopcornFX {

void	PopcornFXStopUpdate::Activate(PopcornFXIntegration *integration)
{
	AZ::TickBus::Handler::BusConnect();
	m_Integration = integration;
}

void	PopcornFXStopUpdate::Deactivate()
{
	AZ::TickBus::Handler::BusDisconnect();
	m_Integration = null;
}

int		PopcornFXStopUpdate::GetTickOrder()
{
	return AZ::TICK_LAST;
}

void	PopcornFXStopUpdate::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	(void)deltaTime; (void)time;
	m_Integration->StopUpdate();
}

}

#endif //O3DE_USE_PK
