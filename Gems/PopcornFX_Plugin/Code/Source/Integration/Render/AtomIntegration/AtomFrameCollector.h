//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "AtomFrameCollectorTypes.h"

#include <pk_render_helpers/include/frame_collector/rh_frame_collector.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

// Frame collector is templated with user data:
class	CAtomFrameCollector : public CFrameCollector
{
public:
	CAtomFrameCollector();
	virtual ~CAtomFrameCollector();

	TMemoryView<const PParticleRenderMedium>	RenderMediums() const { return m_ParticleRenderMediums.View(); }

private:
	// Early Cull: Culls an entire medium on the update thread (when collecting the frame)
	virtual bool	EarlyCull(const PopcornFX::CAABB &bbox) const override;

	// Late Cull: Cull draw requests or individual pages on the render thread (when collecting draw calls)
	// You can use this method if you don't have render thread views available from the update thread
	// Ideally, cull in EarlyCull, but you should implement both methods
	// Late cull also allows for finer culling (per draw request / per page)
	virtual bool	LateCull(const PopcornFX::CAABB &bbox) const override { return EarlyCull(bbox); }
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
