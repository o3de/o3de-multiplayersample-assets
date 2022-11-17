//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "ParticleBuffers.h"
#include <pk_render_helpers/include/batch_jobs/rh_batch_jobs_light_std.h>
#include <pk_render_helpers/include/draw_requests/rh_draw_requests.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

class	CLightBatchDrawer :	public CRendererBatchJobs_Light_Std,
							public CBatchDrawerCommon
{
public:
	typedef CRendererBatchJobs_Light_Std	Super;

	CLightBatchDrawer();
	virtual ~CLightBatchDrawer();

	virtual bool	AreRenderersCompatible(const CRendererDataBase *rendererA, const CRendererDataBase *rendererB) const override;
	virtual bool	AllocBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	MapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	UnmapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	EmitDrawCall(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass, const SDrawCallDesc &toEmit) override;
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
