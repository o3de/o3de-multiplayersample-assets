//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "Integration/PopcornFXIntegrationBus.h"
#include "Integration/PopcornFXUtils.h"

#include <AzCore/Component/EntityId.h>

#include <pk_kernel/include/kr_string_id.h>
#include <pk_particles/include/ps_system.h>

namespace PopcornFX
{
	PK_FORWARD_DECLARE(ParticleEffect);
}

namespace PopcornFX {

class CBroadcastManager
{
public:
	struct	SPopcornFXPayload
	{
		PopcornFX::CStringId						m_PayloadName;
		PopcornFX::EPopcornFXType						m_PayloadType;

		PopcornFX::TArray<SPayloadValue>			m_Values;
	};

	void				Update();

	bool				RegisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName);
	bool				UnregisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName);
	const SPayloadValue	*GetCurrentPayloadValue(const AZStd::string &payloadName) const;
	void				RegisterToBroadcastCallback(Threads::SThreadContext				*threadCtx,
													CParticleMedium						*parentMedium,
													u32									eventID,
													CStringId							eventName,
													u32									count,
													const SUpdateTimeArgs				&timeArgs,
													const TMemoryView<const float>		&spawnDtToEnd,
													const TMemoryView<const CEffectID>	&effectIDs,
													const SPayloadView					&payloadView);

private:
	struct	SPopcornFXBroadcastHookedParams
	{
		PopcornFX::PParticleEffect				m_Effect;
		AZStd::string							m_EventName;
		AZStd::vector<AZ::EntityId>				m_EntityId;
		AZStd::vector<PopcornFX::CEffectID>		m_EffectId;
	};

	struct	SPopcornFXBroadcastParams
	{
		PopcornFX::CStringId					m_EventName;
		PopcornFX::u32							m_Count;
		PopcornFX::TArray<float>				m_SpawnDtToEnd;
		PopcornFX::TArray<PopcornFX::CEffectID>	m_EffectIDs;
		PopcornFX::TArray<SPopcornFXPayload>	m_Payloads;
	};

	template <typename _OutType>
	void				FillPayload(const PopcornFX::SPayloadElementView &srcPayloadElementData, SPopcornFXPayload &dstPayload);
	void				FillPayloadBool(const PopcornFX::SPayloadElementView &srcPayloadElementData, SPopcornFXPayload &dstPayload, u32 dim);
	void				RetrievePayloadElements(const PopcornFX::SPayloadView &srcPayloadView, PopcornFX::TArray<SPopcornFXPayload> &dstPayloadView);
	CParticleEffect		*GetEffectAndId(AZ::EntityId entityId, CEffectID &outEmitterId) const;
	CParticleEffect		*GetEffect(AZ::EntityId entityId) const;

private:
	PopcornFX::TArray<SPopcornFXBroadcastHookedParams>		m_EntitiesToBroadcast;
	PopcornFX::TArray<SPopcornFXBroadcastParams>			m_EventBroadcastPool;
	PopcornFX::u32											m_CurrentBroadcastParamIdx = 0;
	PopcornFX::u32											m_CurrentPayloadIdx = 0;
};

}

#endif //O3DE_USE_PK
