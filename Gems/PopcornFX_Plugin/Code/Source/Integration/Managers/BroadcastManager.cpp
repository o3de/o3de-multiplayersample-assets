//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "BroadcastManager.h"

#if defined(O3DE_USE_PK)

#include <PopcornFX/PopcornFXBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>

#include "Components/Emitter/PopcornFXEmitterGameComponent.h"

namespace PopcornFX {

void	CBroadcastManager::Update()
{
#if 1
	for (AZ::u32 poolIdx = 0; poolIdx < m_EventBroadcastPool.Count(); poolIdx++)
	{
		m_CurrentBroadcastParamIdx = poolIdx;
		auto	&event = m_EventBroadcastPool[poolIdx];
		for (AZ::u32 eventIdx = 0; eventIdx < event.m_Count; eventIdx++)
		{
			m_CurrentPayloadIdx = eventIdx;
			for (AZ::u32 entityIdx = 0; entityIdx < m_EntitiesToBroadcast.Count(); entityIdx++)
			{
				auto	&entityToBroadcast = m_EntitiesToBroadcast[entityIdx];
				for (AZ::u32 subIdx = 0; subIdx < entityToBroadcast.m_EntityId.size(); subIdx++)
				{
					if (entityToBroadcast.m_EffectId[subIdx] == event.m_EffectIDs[eventIdx] &&
						CStringId(entityToBroadcast.m_EventName.data()) == event.m_EventName)
					{
#if 0
						AZ_Printf("PopcorFX", "Event broadcasted : poolIdx(%d) eventIdx(%d) entityIdx(%d) effetID %d => name %s to entity[%s]\n", poolIdx, eventIdx, entityIdx, entityToBroadcast.m_EffectId[subIdx], entityToBroadcast.m_EventName.data(), entityToBroadcast.m_EntityId[subIdx].ToString().c_str());
#endif
						PopcornFX::SBroadcastParams	params;
						const AZ::EntityId			entityId = entityToBroadcast.m_EntityId[subIdx];
						params.m_EventName = event.m_EventName.ToStringData();
						PopcornFXEmitterComponentEventsBus::Event(entityId, &PopcornFXEmitterComponentEventsBus::Events::OnEmitterBroadcastEvent, &params);
					}
				}
			}
		}
	}
#else
	for (uint32 poolIdx = 0; poolIdx < m_EventBroadcastPool.Count(); poolIdx++)
	{
		auto	&event = m_EventBroadcastPool[poolIdx];
		AZ_Printf("PopcorFX", "POOL => EventName: %s", event.m_EventName.ToStringData());
		for (uint32 eventIdx = 0; eventIdx < event.m_Count; eventIdx++)
			AZ_Printf("PopcorFX", "     => Effect ID: %d", event.m_EffectIDs[eventIdx]);
	}
	if (!m_EventBroadcastPool.Empty())
	{
		for (uint32 entityIdx = 0; entityIdx < m_EntitiesToBroadcast.Count(); entityIdx++)
		{
			auto	&entityToBroadcast = m_EntitiesToBroadcast[entityIdx];

			AZ_Printf("PopcorFX", "Entities => EventName: %s", entityToBroadcast.m_EventName.data());
			for (uint32 eventIdx = 0; eventIdx < entityToBroadcast.m_EntityId.size(); eventIdx++)
				AZ_Printf("PopcorFX", "     => EffectID: [%d] EntityID: [%s]", entityToBroadcast.m_EffectId[eventIdx], entityToBroadcast.m_EntityId[eventIdx].ToString().c_str());
		}
	}
#endif
	m_EventBroadcastPool.Clear();
}

const SPayloadValue	*CBroadcastManager::GetCurrentPayloadValue(const AZStd::string &payloadName) const
{
	PK_ASSERT(m_CurrentBroadcastParamIdx < m_EventBroadcastPool.Count());
	const SPopcornFXBroadcastParams				&broadcastParam = m_EventBroadcastPool[m_CurrentBroadcastParamIdx];
	const PopcornFX::TArray<SPopcornFXPayload>	&payloads = broadcastParam.m_Payloads;
	CStringId									payloadNameId(payloadName.c_str());

	for (AZ::u32 i = 0; i < payloads.Count(); i++)
	{
		if (payloads[i].m_PayloadName == payloadNameId)
		{
			return &payloads[i].m_Values[m_CurrentPayloadIdx];
		}
	}
	return null;
}

bool	CBroadcastManager::RegisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName)
{
	PopcornFX::CEffectID	effectId;
	CParticleEffect	*effect = GetEffectAndId(entityId, effectId);
	if (!PK_VERIFY(effect != null))
		return false;

	int	foundIdx = -1;
	for (AZ::u32 i = 0; i < m_EntitiesToBroadcast.Count(); i++)
	{
		auto entityToBroadcast = m_EntitiesToBroadcast[i];
		if (entityToBroadcast.m_EventName == eventName && entityToBroadcast.m_Effect == effect)
		{
			foundIdx = i;
			break;
		}
	}
	if (foundIdx >= 0)
	{
		m_EntitiesToBroadcast[foundIdx].m_EffectId.push_back(effectId);
		m_EntitiesToBroadcast[foundIdx].m_EntityId.push_back(entityId);
#if 0
		AZ_Printf("PopcorFX", "RegisterToBroadcast Add => EventName: [%s] EntityID: [%s] EffectID: [%d]", m_EventName.data(), m_EntityID.ToString().c_str(), emitter.GetEmitter()->EffectID());
#endif
	}
	else
	{
		SPopcornFXBroadcastHookedParams	params;
		params.m_EffectId.push_back(effectId);
		params.m_EntityId.push_back(entityId);
		params.m_EventName = eventName;
		params.m_Effect = effect;
		m_EntitiesToBroadcast.PushBack(params);
#if 0
		AZ_Printf("PopcorFX", "RegisterToBroadcast New => EventName: [%s] EntityID: [%s] EffectID: [%d]", params.m_EventName.data(), m_EntityID.ToString().c_str(), emitter.GetEmitter()->EffectID());
#endif
		const PopcornFX::FastDelegate<PopcornFX::CParticleEffect::EventCallback>	broadcastCallback(this, &CBroadcastManager::RegisterToBroadcastCallback);

		if (!effect->RegisterEventCallback(broadcastCallback, PopcornFX::CStringId(eventName.c_str())))
			return false;
	}
	return true;
}

bool	CBroadcastManager::UnregisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName)
{
	const PopcornFX::FastDelegate<PopcornFX::CParticleEffect::EventCallback>	broadcastCallback(this, &CBroadcastManager::RegisterToBroadcastCallback);

	CParticleEffect	*effect = GetEffect(entityId);
	if (!PK_VERIFY(effect != null))
		return false;

	effect->UnregisterEventCallback(broadcastCallback, PopcornFX::CStringId(eventName.data()));

	for (AZ::u32 i = 0; i < m_EntitiesToBroadcast.Count(); i++)
	{
		AZStd::vector<AZ::EntityId>	&entitiesId = m_EntitiesToBroadcast[i].m_EntityId;
		for (AZ::u32 j = 0; j < entitiesId.size(); j++)
		{
			if (entitiesId[j] == entityId)
			{
				entitiesId.erase(&entitiesId[j]);
				break;
			}
		}
		if (entitiesId.empty())
			m_EntitiesToBroadcast.Remove_AndKeepOrder(i);
	}
	return true;
}

void	CBroadcastManager::RegisterToBroadcastCallback(	PopcornFX::Threads::SThreadContext	*threadCtx,
														PopcornFX::CParticleMedium			*parentMedium,
														u32									eventID,
														PopcornFX::CStringId				eventName,
														u32									count,
														const SUpdateTimeArgs				&timeArgs,
														const TMemoryView<const float>		&spawnDtToEnd,
														const TMemoryView<const CEffectID>	&effectIDs,
														const PopcornFX::SPayloadView		&payloadView)
{
	(void)threadCtx; (void)parentMedium; (void)eventID; (void)timeArgs;

	SPopcornFXBroadcastParams	params;

	params.m_EventName = eventName;
	params.m_Count = count;
	params.m_SpawnDtToEnd.Resize(count);
	params.m_EffectIDs.Resize(count);
	Mem::Copy(params.m_SpawnDtToEnd.RawDataPointer(), spawnDtToEnd.Data(), spawnDtToEnd.CoveredBytes());
	Mem::Copy(params.m_EffectIDs.RawDataPointer(), effectIDs.Data(), effectIDs.CoveredBytes());
	RetrievePayloadElements(payloadView, params.m_Payloads);

	m_EventBroadcastPool.PushBack(params);

#if 0
	AZ_Printf("PopcorFX", "REGISTER POOL => EventName: %s", eventName.ToStringData());
	for (uint32 eventIdx = 0; eventIdx < count; eventIdx++)
		AZ_Printf("PopcorFX", "              => Effect ID: %d", effectIDs[eventIdx]);
#endif
}

template <typename _OutType>
void	CBroadcastManager::FillPayload(const PopcornFX::SPayloadElementView &srcPayloadElementData, SPopcornFXPayload &dstPayload)
{
	const u32		valueOffset = dstPayload.m_Values.Count();
	const u32		particleCount = srcPayloadElementData.m_Data.m_Count;
	const u32		particleStride = srcPayloadElementData.m_Data.m_Stride;
	const u32		byteCount = sizeof(_OutType);

	if (!PK_VERIFY(dstPayload.m_Values.Resize(valueOffset + particleCount)))
		return;

	for (u32 iParticle = 0; iParticle < particleCount; ++iParticle)
	{
		SPayloadValue	&value = dstPayload.m_Values[valueOffset + iParticle];

		Mem::Copy(&value.m_ValueBool[0], srcPayloadElementData.m_Data.m_RawDataPtr + iParticle * particleStride, byteCount);
	}
}

void	CBroadcastManager::FillPayloadBool(const PopcornFX::SPayloadElementView &srcPayloadElementData, SPopcornFXPayload &dstPayload, u32 dim)
{
	const u32		valueOffset = dstPayload.m_Values.Count();
	const u32		particleCount = srcPayloadElementData.m_Data.m_Count;
	const u32		particleStride = srcPayloadElementData.m_Data.m_Stride;

	if (!PK_VERIFY(dstPayload.m_Values.Resize(valueOffset + particleCount)))
		return;

	for (u32 iParticle = 0; iParticle < particleCount; ++iParticle)
	{
		SPayloadValue	&value = dstPayload.m_Values[valueOffset + iParticle];
		const u32		*srcPtr = (u32*)&srcPayloadElementData.m_Data.m_RawDataPtr[iParticle * particleStride];

		for (u32 dimIdx = 0; dimIdx < dim; dimIdx++)
			value.m_ValueBool[dimIdx] = (srcPtr[dimIdx] == 0) ? false : true;
	}
}

void	CBroadcastManager::RetrievePayloadElements(const PopcornFX::SPayloadView &srcPayloadView, PopcornFX::TArray<SPopcornFXPayload> &dstPayloadView)
{
	dstPayloadView.Reserve(srcPayloadView.m_PayloadElements.Count());

	// loop on payloads
	const u32			payloadElementsCount = srcPayloadView.m_PayloadElements.Count();
	for (u32 iPayloadElement = 0; iPayloadElement < payloadElementsCount; ++iPayloadElement)
	{
		const PopcornFX::SParticleDeclaration::SEvent::SPayload		&srcPKFXPayload = srcPayloadView.m_EventDesc->m_Payload[iPayloadElement];

		// search for existing payload
		PopcornFX::CGuid											payloadIndex;
		for (AZ::u32 i = 0; i < dstPayloadView.Count(); i++)
		{
			if (dstPayloadView[i].m_PayloadName == srcPKFXPayload.m_NameGUID)
			{
				payloadIndex = i;
				break;
			}
		}

		const EPopcornFXType									payloadElementType = BaseTypeToO3DEPopcornFXType(srcPKFXPayload.m_Type);

		if (!payloadIndex.Valid()) // new payload
		{
			SPopcornFXPayload	payload;
			payload.m_PayloadName = srcPKFXPayload.m_NameGUID;
			payload.m_PayloadType = payloadElementType;
			payloadIndex = dstPayloadView.PushBack(payload);
			if (!PK_VERIFY(payloadIndex.Valid()))
				continue;
		}

		// fill payload element data into m_PayloadViews
		switch (payloadElementType)
		{
		case	EPopcornFXType::Type_Bool:
			FillPayloadBool(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex], 1);
			break;
		case	EPopcornFXType::Type_Bool2:
			FillPayloadBool(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex], 2);
			break;
		case	EPopcornFXType::Type_Bool3:
			FillPayloadBool(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex], 3);
			break;
		case	EPopcornFXType::Type_Bool4:
			FillPayloadBool(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex], 4);
			break;
		case	EPopcornFXType::Type_Float:
			FillPayload<float>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Float2:
			FillPayload<CFloat2>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Float3:
			FillPayload<CFloat3>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Float4:
		case	EPopcornFXType::Type_Quaternion:
			FillPayload<CFloat4>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Int:
			FillPayload<u32>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Int2:
			FillPayload<CInt2>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Int3:
			FillPayload<CInt3>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		case	EPopcornFXType::Type_Int4:
			FillPayload<CInt4>(srcPayloadView.m_PayloadElements[iPayloadElement], dstPayloadView[payloadIndex]);
			break;
		default:
			PK_ASSERT_NOT_REACHED();
			break;
		}
	}
}

CParticleEffect	*CBroadcastManager::GetEffectAndId(AZ::EntityId entityId, PopcornFX::CEffectID &outEmitterId) const
{
	AZ::Entity	*entity = null;
	EBUS_EVENT_RESULT(entity, AZ::ComponentApplicationBus, FindEntity, entityId);
	if (!PK_VERIFY(entity != null))
		return null;

	PopcornFX::PopcornFXEmitterGameComponent	*emitterGameComponent = entity->FindComponent<PopcornFX::PopcornFXEmitterGameComponent>();
	if (!PK_VERIFY(emitterGameComponent != null))
		return null;

	PopcornFX::PopcornFXEmitter	&emitter = emitterGameComponent->m_Emitter.Emitter();

	CParticleEffectInstance	*emitterInstance = emitter.GetEmitter().Get();
	if (!PK_VERIFY(emitterInstance != null))
		return null;

	outEmitterId = emitterInstance->EffectID();
	return emitter.GetEffect().Get();
}

CParticleEffect	*CBroadcastManager::GetEffect(AZ::EntityId entityId) const
{
	CEffectID	dummy;
	return GetEffectAndId(entityId, dummy);
}

}

#endif //O3DE_USE_PK
