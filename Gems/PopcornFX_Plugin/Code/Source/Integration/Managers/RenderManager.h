//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "Integration/Render/AtomIntegration/AtomFrameCollector.h"
#include "Integration/Render/AtomIntegration/AtomRenderDataFactory.h"

#include <Atom/RHI/BufferPool.h>
#include <Atom/RPI.Public/Pass/Pass.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

class CPopcornFXFeatureProcessor;
struct SSceneViews;

class CRenderManager
{
public:
	void	Activate(CParticleMediumCollection *mediumCollection, const AZStd::string &packPath);
	void	Deactivate();
	void	Reset();
	void	SetFeatureProcessor(CPopcornFXFeatureProcessor *featureProcessor) { m_FeatureProcessor = featureProcessor; }
	void	SetPackPath(const AZStd::string &packPath);
	void	StartUpdate(CParticleMediumCollection *mediumCollection, const SSceneViews *sceneViews);
	void	StopUpdate(CParticleMediumCollection *mediumCollection);

	AZ::RHI::Ptr<AZ::RHI::Buffer>	ResizeOrCreateBufferIFN(AZ::RHI::Ptr<AZ::RHI::Buffer> buffer, AZ::u64 bufferSize, AZ::u32 alignBufferSize = 0x1000);
	void							*MapBuffer(AZ::RHI::Ptr<AZ::RHI::Buffer> buffer, AZ::u64 sizeToMap);
	void							UnmapBuffer(AZ::RHI::Ptr<AZ::RHI::Buffer> buffer);

	const CAtomFrameCollector		&GetFrameCollector() const { return m_FrameCollector; }

	const SAtomRenderContext		&GetRenderContext() const { return m_RenderContext; }
	void							UnregisterEffectMaterials(const PParticleEffect &effect);

private:
	void	CollectFrame(CParticleMediumCollection *mediumCollection);

	AZ::RHI::Ptr<AZ::RHI::Buffer>	AllocBuffer(AZ::u64 bufferSize, AZ::RHI::BufferBindFlags binding, AZ::u32 alignSizeOn = 0x10);

	AZ::RHI::Ptr<AZ::RHI::BufferPool>	m_BufferPool;

	CAtomFrameCollector				m_FrameCollector;
	CAtomRenderDataFactory			m_RenderBatchFactory;
	SAtomRenderContext				m_RenderContext;
	CPopcornFXFeatureProcessor		*m_FeatureProcessor = null;

	const SSceneViews				*m_SceneViews = null; // Used by CollectFrame
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
