//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXBroadcastNodeable.h"

#include <AzCore/Component/Entity.h>
#include <ScriptCanvas/Libraries/Libraries.h>

#include "Integration/PopcornFXIntegrationBus.h"

namespace PopcornFX {

	PopcornFXBroadcastNodeable::~PopcornFXBroadcastNodeable()
	{
		PopcornFX::PopcornFXEmitterComponentEventsBus::Handler::BusDisconnect();
	}

	void	PopcornFXBroadcastNodeable::ConnectEvent(AZStd::string eventName, AZ::EntityId entityId)
	{
		PopcornFX::PopcornFXEmitterComponentEventsBus::Handler::BusDisconnect();

		if (entityId.IsValid())
		{
			m_EntityId = entityId;
			m_EventName = eventName;
			PopcornFX::PopcornFXEmitterComponentEventsBus::Handler::BusConnect(entityId);
		}
	}

	void	PopcornFXBroadcastNodeable::OnDeactivate()
	{
		PopcornFX::PopcornFXEmitterComponentEventsBus::Handler::BusDisconnect();
	}

	void	PopcornFXBroadcastNodeable::OnEmitterReady()
	{
		bool ok = false;
		PopcornFX::PopcornFXIntegrationBus::BroadcastResult(ok, &PopcornFX::PopcornFXIntegrationBus::Handler::RegisterToBroadcast, m_EntityId, m_EventName);
		AZ_Error("PopcornFX", ok, "Unable to register event %s to broadcast.", m_EventName.c_str());
	}

	void	PopcornFXBroadcastNodeable::OnEmitterBroadcastEvent(PopcornFX::SBroadcastParams *params)
	{
		//SCRIPT_CANVAS_PERFORMANCE_SCOPE_LATENT(GetScriptCanvasId(), GetAssetId());

		if (params->m_EventName == m_EventName)
			CallBroadcast();
	}

}
