//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <pk_particles/include/ps_mediums.h>
#include "Integration/Render/CameraInfo.h"

namespace PopcornFX {
//----------------------------------------------------------------------------

class CSceneViewsManager
{
public:
	void	Activate();
	void	Deactivate();
	void	Update(CParticleMediumCollection *mediumCollection);

	const SSceneViews	*SceneViews() { return &m_SceneViews; }

	SSceneViews					m_SceneViews;
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
