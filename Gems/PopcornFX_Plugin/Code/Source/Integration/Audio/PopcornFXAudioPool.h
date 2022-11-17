//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if 0//defined(O3DE_USE_PK)

#include <pk_kernel/include/kr_string_id.h>

namespace Audio
{
	struct IAudioProxy;
} // namespace Audio

namespace PopcornFX {

struct	SSoundInsertDesc
{
	CInt2					m_SelfID;
	CFloat3					m_Position;
	float					m_FXParam;
};

struct	SSoundIds
{
	Audio::TAudioControlID	m_StartAudioTriggerID = INVALID_AUDIO_CONTROL_ID;
	Audio::TAudioControlID	m_StopAudioTriggerID = INVALID_AUDIO_CONTROL_ID;
	Audio::TAudioControlID	m_AudioRtpcID = INVALID_AUDIO_CONTROL_ID;
};


class	CSoundDescriptor
{
public:
	CSoundDescriptor() { }
	~CSoundDescriptor();

	void		Clear();
	bool		UsedThisUpdate(u32 currentUpdateId) const { return currentUpdateId == m_UsedUpdateId; }
	CInt2		SelfID() const { return m_SelfID; }

	void		Update(u32 currentUpdateId, const SSoundInsertDesc &insertDesc);
	void		Spawn(u32 currentUpdateId, const SSoundInsertDesc &insertDesc, const SSoundIds &soundIds);
	void		Unuse();

private:
	CInt2					m_SelfID = CInt2(0);
	CFloat3					m_LastPosition;
	float					m_LastFXParam;
	u32						m_UsedUpdateId = 0;

	SSoundIds				m_SoundIds;
	Audio::IAudioProxy		*m_pIAudioProxy = null;
};

class	CSoundDescriptorPoolCollection;

class	CSoundDescriptorPool
{
public:
	CSoundDescriptorPool() {}

	void			Clear();
	bool			Setup(CStringId startTrigger, CStringId stopTrigger, CStringId rtpc);

	void			BeginInsert();
	void			InsertSoundIFP(const SSoundInsertDesc &insertDesc);
	void			EndInsert();

private:
	u32						m_CurrentUpdateId = 0;
	u32						m_SoundsPlaying = 0;
	u32						m_LastUpdatedSlotCount = 0;
	SSoundIds				m_SoundIds;

	PopcornFX::TArray<CSoundDescriptor>		m_Slots;
	PopcornFX::TArray<SSoundInsertDesc>		m_ToSpawn;
};

}

#endif //O3DE_USE_PK
