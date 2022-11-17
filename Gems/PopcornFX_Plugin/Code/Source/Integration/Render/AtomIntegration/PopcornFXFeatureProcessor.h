//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <Atom/RPI.Public/FeatureProcessor.h>
#include <Integration/Managers/RenderManager.h>
#include <Integration/Render/CameraInfo.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

class CPopcornFXFeatureProcessorInterface : public AZ::RPI::FeatureProcessor
{
public:
	AZ_RTTI(CPopcornFXFeatureProcessorInterface, "{D86216E4-92A8-43BE-A5E4-8853489C7BA1}", AZ::RPI::FeatureProcessor);
};


class CPopcornFXFeatureProcessor : public CPopcornFXFeatureProcessorInterface
{
public:
	AZ_RTTI(CPopcornFXFeatureProcessor, "{D86216E4-92A8-43BE-123F-883489C75BA1}", CPopcornFXFeatureProcessorInterface);

	CPopcornFXFeatureProcessor();
	virtual ~CPopcornFXFeatureProcessor() = default;

	static void	Reflect(AZ::ReflectContext *context);

	// FeatureProcessor overrides ...
	void		Activate() override;
	void		Deactivate() override;
	void		Simulate(const SimulatePacket &packet) override;
	void		Render(const RenderPacket &packet) override;

	void	Init(CParticleMediumCollection *medCol, const SSceneViews *views);

	void	AppendLightParticles();

	CRenderManager		&GetRenderManager() { return m_RenderManager; }

private:
	const AZ::RHI::DrawPacket	*BuildDrawPacket(	const SAtomRenderContext::SDrawCall &pkfxDrawCall,
													const AZ::RHI::ShaderResourceGroup *viewSrg,
													AZ::RHI::DrawItemSortKey sortKey);

	AZStd::vector<AZStd::unique_ptr<const AZ::RHI::DrawPacket>>	m_drawPackets;
	CRenderManager												m_RenderManager;
	const SSceneViews											*m_SceneViews = null;
	CParticleMediumCollection									*m_MediumCollection = null;
};

//----------------------------------------------------------------------------
}

#endif
