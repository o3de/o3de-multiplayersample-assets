//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "ParticleBuffers.h"
#include <pk_render_helpers/include/batch_jobs/rh_batch_jobs_mesh_cpu.h>
#include <pk_render_helpers/include/draw_requests/rh_draw_requests.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

class	CMeshBatchDrawer :	public CRendererBatchJobs_Mesh_CPUBB,
							public CBatchDrawerCommon
{
public:
	typedef CRendererBatchJobs_Mesh_CPUBB	Super;

	CMeshBatchDrawer();
	virtual ~CMeshBatchDrawer();

	virtual bool	AreRenderersCompatible(const CRendererDataBase *rendererA, const CRendererDataBase *rendererB) const override;
	virtual bool	AllocBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	MapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	UnmapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass) override;
	virtual bool	EmitDrawCall(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass, const SDrawCallDesc &toEmit) override;

private:
	PGeometryCache			m_GeometryCache;

	// Additional fields mapped buffers:
	PopcornFX::TArray<Drawers::SCopyFieldDesc>			m_MappedAdditionalShaderInputs;
	PopcornFX::TArray<Drawers::SCopyFieldDescPerMesh>	m_MappedAdditionalShaderInputsPerMesh;
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
