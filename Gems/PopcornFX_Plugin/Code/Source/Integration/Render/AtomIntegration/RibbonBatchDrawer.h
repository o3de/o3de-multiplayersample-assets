//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "ParticleBuffers.h"
#include <pk_render_helpers/include/batch_jobs/rh_batch_jobs_ribbon_cpu.h>
#include <pk_render_helpers/include/draw_requests/rh_draw_requests.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

class	CRibbonBatchDrawer : public CRendererBatchJobs_Ribbon_CPUBB,
								public CBatchDrawerCommon
{
public:
	typedef CRendererBatchJobs_Ribbon_CPUBB	Super;

	CRibbonBatchDrawer();
	virtual ~CRibbonBatchDrawer();

	virtual bool	AreRenderersCompatible(const CRendererDataBase *rendererA, const CRendererDataBase *rendererB) const override;
	virtual bool	AllocBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	MapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	UnmapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	EmitDrawCall(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass, const SDrawCallDesc &toEmit) override;

private:
	// Additional fields mapped buffers:
	PopcornFX::TArray<Drawers::SCopyFieldDesc>			m_MappedAdditionalShaderInputs;

	// Atlas definition buffer:
	AZ::RHI::Ptr<AZ::RHI::Buffer>				m_AtlasDefinition = null;
	u32											m_AtlasSubRectsCount = 0;
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
