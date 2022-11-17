//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "MediumCollectionManager.h"

#if defined(O3DE_USE_PK)

#include <pk_particles/include/ps_mediums.h>

#include "Integration/SceneInterface/SceneInterface.h"

namespace PopcornFX {
//----------------------------------------------------------------------------

bool	CMediumCollectionManager::Activate(CSceneInterface *sceneInterface)
{
	// if necessary, lazy-create the medium collection
	if (m_ParticleMediumCollection == null)
	{
		m_ParticleMediumCollection = PK_NEW(CParticleMediumCollection);
	}

	if (m_ParticleMediumCollection != null)
	{
		m_ParticleMediumCollection->EnableBounds(true);
		m_ParticleMediumCollection->Clear();
		m_ParticleMediumCollection->SetScene(sceneInterface);
		m_ParticleMediumCollection->RegisterView();
		m_ParticleMediumCollection->EnableInstanceDataMultiBuffering(true);
	}
	return m_ParticleMediumCollection != null;
}

void	CMediumCollectionManager::Deactivate()
{
	if (m_ParticleMediumCollection != null)
	{
		PK_DELETE(m_ParticleMediumCollection);
		m_ParticleMediumCollection = null;
	}
}

void	CMediumCollectionManager::Reset()
{
	m_ParticleMediumCollection->Clear();
}

void	CMediumCollectionManager::StartUpdate(float deltaTime)
{
	m_ParticleMediumCollection->Update(deltaTime);		// launch the update

}

void	CMediumCollectionManager::StopUpdate()
{
	m_ParticleMediumCollection->UpdateFence();
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
