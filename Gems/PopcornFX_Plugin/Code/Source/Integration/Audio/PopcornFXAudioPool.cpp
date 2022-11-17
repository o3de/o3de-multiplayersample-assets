//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXAudioPool.h"

#if 0//defined(O3DE_USE_PK)

#include "Integration/PopcornFXUtils.h"

namespace PopcornFX {

//----------------------------------------------------------------------------
//
//	SSoundInsertDesc
//
//----------------------------------------------------------------------------

CSoundDescriptor::~CSoundDescriptor()
{
	Clear();
}

//----------------------------------------------------------------------------

void	CSoundDescriptor::Clear()
{
	Unuse();

	m_SoundIds.m_StartAudioTriggerID = INVALID_AUDIO_CONTROL_ID;
	m_SoundIds.m_StopAudioTriggerID = INVALID_AUDIO_CONTROL_ID;
	m_SoundIds.m_AudioRtpcID = INVALID_AUDIO_CONTROL_ID;
}

//----------------------------------------------------------------------------

void	CSoundDescriptor::Update(u32 currentUpdateId, const SSoundInsertDesc &insertDesc)
{
	m_UsedUpdateId = currentUpdateId;

	PK_ASSERT(m_SelfID == insertDesc.m_SelfID);

	if (m_pIAudioProxy != null)
	{
		if (m_SoundIds.m_AudioRtpcID != INVALID_AUDIO_CONTROL_ID && m_LastFXParam != insertDesc.m_FXParam)
		{
			m_pIAudioProxy->SetRtpcValue(m_SoundIds.m_AudioRtpcID, insertDesc.m_FXParam);
		}

		if (m_LastPosition != insertDesc.m_Position)
		{
			m_LastPosition = insertDesc.m_Position;
			m_pIAudioProxy->SetPosition(LYVec3ToAZVec3(ToCry(m_LastPosition)));
		}
	}
}

//----------------------------------------------------------------------------

void	CSoundDescriptor::Spawn(u32 currentUpdateId, const SSoundInsertDesc &insertDesc, const SSoundIds &soundIds)
{
	m_UsedUpdateId = currentUpdateId;
	m_SelfID = insertDesc.m_SelfID;
	m_LastPosition = insertDesc.m_Position;
	m_LastFXParam = insertDesc.m_FXParam;

	m_SoundIds = soundIds;

	if (m_SoundIds.m_StartAudioTriggerID != INVALID_AUDIO_CONTROL_ID || m_SoundIds.m_StopAudioTriggerID != INVALID_AUDIO_CONTROL_ID)
	{
		PK_ASSERT(m_pIAudioProxy == null);
		Audio::AudioSystemRequestBus::BroadcastResult(m_pIAudioProxy, &Audio::AudioSystemRequestBus::Events::GetFreeAudioProxy);

		if (m_pIAudioProxy != null)
		{
			m_pIAudioProxy->Initialize("PopcornFXEmitter");
			m_pIAudioProxy->SetObstructionCalcType(Audio::eAOOCT_SINGLE_RAY);

			if (m_SoundIds.m_AudioRtpcID != INVALID_AUDIO_CONTROL_ID)
			{
				m_pIAudioProxy->SetRtpcValue(m_SoundIds.m_AudioRtpcID, insertDesc.m_FXParam);
			}

			// Execute start trigger immediately.
			if (m_SoundIds.m_StartAudioTriggerID != INVALID_AUDIO_CONTROL_ID)
			{
				m_pIAudioProxy->SetPosition(LYVec3ToAZVec3(ToCry(m_LastPosition)));
				m_pIAudioProxy->SetCurrentEnvironments();
				m_pIAudioProxy->ExecuteTrigger(m_SoundIds.m_StartAudioTriggerID);
			}
		}
	}
}

//----------------------------------------------------------------------------

void	CSoundDescriptor::Unuse()
{
	m_UsedUpdateId = 0;
	m_SelfID = CInt2(0);

	if (m_pIAudioProxy != null)
	{
		m_pIAudioProxy->SetPosition(LYVec3ToAZVec3(ToCry(m_LastPosition)));

		if (m_SoundIds.m_StopAudioTriggerID != INVALID_AUDIO_CONTROL_ID)
		{
			m_pIAudioProxy->ExecuteTrigger(m_SoundIds.m_StopAudioTriggerID);
		}
		else
		{
			assert(m_SoundIds.m_StartAudioTriggerID != INVALID_AUDIO_CONTROL_ID);
			m_pIAudioProxy->StopTrigger(m_SoundIds.m_StartAudioTriggerID);
		}

		m_pIAudioProxy->Release();
		m_pIAudioProxy = null;
	}
}

//----------------------------------------------------------------------------
//
//	CSoundDescriptorPool
//
//----------------------------------------------------------------------------

void	CSoundDescriptorPool::Clear()
{
	for (u32 i = 0, slotCount = m_Slots.Count(); i < slotCount; ++i)
		m_Slots[i].Clear();
	m_SoundsPlaying = 0;
}

//----------------------------------------------------------------------------

bool	CSoundDescriptorPool::Setup(CStringId startTrigger, CStringId stopTrigger, CStringId rtpc)
{
	static const u32		kPoolSize = 10;

	if (!PK_VERIFY(m_Slots.Resize(kPoolSize)))
		return false;

	if (startTrigger.Valid())
	{
		Audio::AudioSystemRequestBus::BroadcastResult(m_SoundIds.m_StartAudioTriggerID, &Audio::AudioSystemRequestBus::Events::GetAudioTriggerID, startTrigger.ToString().Data());
	}

	if (stopTrigger.Valid())
	{
		Audio::AudioSystemRequestBus::BroadcastResult(m_SoundIds.m_StopAudioTriggerID, &Audio::AudioSystemRequestBus::Events::GetAudioTriggerID, stopTrigger.ToString().Data());
	}

	if (rtpc.Valid())
	{
		Audio::AudioSystemRequestBus::BroadcastResult(m_SoundIds.m_AudioRtpcID, &Audio::AudioSystemRequestBus::Events::GetAudioRtpcID, rtpc.ToString().Data());
	}

	return true;
}

//----------------------------------------------------------------------------

void	CSoundDescriptorPool::BeginInsert()
{
	++m_CurrentUpdateId;
	m_SoundsPlaying = 0;
}

//----------------------------------------------------------------------------

void	CSoundDescriptorPool::InsertSoundIFP(const SSoundInsertDesc &insertDesc)
{
	PK_ASSERT(m_LastUpdatedSlotCount <= m_Slots.Count());
	bool	spawnLater = true;
	if (m_SoundsPlaying < m_Slots.Count())
	{
		PopcornFX::CGuid	slot;
		const CInt2			insertSelfID = insertDesc.m_SelfID;
		for (u32 i = 0; i < m_LastUpdatedSlotCount; ++i)
		{
			const CSoundDescriptor		&sd = m_Slots[i];
			if (sd.SelfID() != insertSelfID)
				continue;
			slot = i;
			break;
		}
		if (slot.Valid())
		{
			m_SoundsPlaying++;
			m_Slots[slot].Update(m_CurrentUpdateId, insertDesc);
			spawnLater = false;
		}
	}
	if (spawnLater)
	{
		m_ToSpawn.PushBack(insertDesc);
	}
}

//----------------------------------------------------------------------------

void	CSoundDescriptorPool::EndInsert()
{
	u32					tospawni = 0;
	PopcornFX::CGuid	lastUsed = PopcornFX::CGuid::INVALID;

	PK_ASSERT(m_LastUpdatedSlotCount <= m_Slots.Count());
	u32		i = 0;
	for (; i < m_LastUpdatedSlotCount; ++i) // after m_LastUpdatedSlotCount: already unused
	{
		CSoundDescriptor	&sd = m_Slots[i];
		if (sd.UsedThisUpdate(m_CurrentUpdateId))
		{
			lastUsed = i;
			continue;
		}
		else
		{
			sd.Unuse();
		}

		if (tospawni < m_ToSpawn.Count())
		{
			sd.Spawn(m_CurrentUpdateId, m_ToSpawn[tospawni], m_SoundIds);
			++m_SoundsPlaying;
			++tospawni;
			lastUsed = i;
		}
	}
	// new sounds:
	if (tospawni < m_ToSpawn.Count())
	{
		const u32	remainingToSpawnCount = m_ToSpawn.Count() - tospawni;
		const u32	finalCount = i + remainingToSpawnCount;
		if (m_Slots.Count() < finalCount)
		{
			if (!PK_VERIFY(m_Slots.Resize(finalCount)))
				return;
		}
		for (; tospawni < m_ToSpawn.Count(); ++i)
		{
			CSoundDescriptor	&sd = m_Slots[i];
			sd.Spawn(m_CurrentUpdateId, m_ToSpawn[tospawni], m_SoundIds);
			++tospawni;
			lastUsed = i;
		}
	}

	m_LastUpdatedSlotCount = lastUsed.Valid() ? u32(lastUsed) + 1U : 0U;

	m_ToSpawn.Clear();
}

}

#endif //O3DE_USE_PK
