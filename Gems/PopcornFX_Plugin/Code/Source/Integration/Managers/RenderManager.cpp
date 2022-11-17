//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "RenderManager.h"

#if defined(O3DE_USE_PK)

#include "Integration/Render/CameraInfo.h"
#include "Integration/Render/AtomIntegration/PopcornFXFeatureProcessor.h"
#include "Integration/PopcornFXUtils.h"
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RHI/Factory.h>
#include <Atom/RHI/RHISystemInterface.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <AzCore/Console/IConsole.h>
#include <pk_particles/include/ps_event_map.h>

namespace PopcornFX {

AZ_CVAR(AZ::u32, p_PopcornFXMaxSlices, 1,
		null,
		AZ::ConsoleFunctorFlags::DontReplicate,
		"How many times a draw-call can be sliced to avoid sorting artefacts.");

//----------------------------------------------------------------------------

void	CRenderManager::Activate(CParticleMediumCollection *mediumCollection, const AZStd::string &packPath)
{
	m_RenderContext.m_RenderManager = this;
	m_RenderBatchFactory.SetPackPath(packPath.c_str());

	const u32	enabledRenderers =	(1U << ERendererClass::Renderer_Billboard) |
									(1U << ERendererClass::Renderer_Ribbon) |
									(1U << ERendererClass::Renderer_Light) |
									(1U << ERendererClass::Renderer_Mesh); /* |
									(1U << ERendererClass::Renderer_Decal) |
									(1U << ERendererClass::Renderer_Sound)*/;

	CFrameCollector::SFrameCollectorInit	init(enabledRenderers,
											CbNewBatchDrawer(&m_RenderBatchFactory, &CAtomRenderDataFactory::CreateBatchDrawer),
											CbNewRendererCache(&m_RenderBatchFactory, &CAtomRenderDataFactory::CreateRendererCache),
											2,
											false);
	m_FrameCollector.Initialize(init);
	m_FrameCollector.InstallToMediumCollection(mediumCollection);

	AZ::RHI::RHISystemInterface	*rhiSystem = AZ::RHI::RHISystemInterface::Get();

	AZ::RHI::BufferPoolDescriptor dynamicPoolDescriptor;
	dynamicPoolDescriptor.m_heapMemoryLevel = AZ::RHI::HeapMemoryLevel::Device;
	dynamicPoolDescriptor.m_hostMemoryAccess = AZ::RHI::HostMemoryAccess::Write;
	dynamicPoolDescriptor.m_bindFlags = AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead;
	dynamicPoolDescriptor.m_largestPooledAllocationSizeInBytes = 0x100000;

	m_BufferPool = AZ::RHI::Factory::Get().CreateBufferPool();
	m_BufferPool->SetName(AZ::Name("PopcornFXParticlesBufferPool"));
	AZ::RHI::ResultCode resultCode = m_BufferPool->Init(*rhiSystem->GetDevice(), dynamicPoolDescriptor);

	m_RenderBatchFactory.SetRenderManager(this);
	if (resultCode != AZ::RHI::ResultCode::Success)
	{
		AZ_Error("DynamicPrimitiveProcessor", false, "Failed to initialize AuxGeom dynamic primitive buffer pool");
		return;
	}
}

void	CRenderManager::Deactivate()
{
	Reset();
	m_RenderContext.m_RenderManager = null;
}

void	CRenderManager::Reset()
{
	// Should release the already collected frames.
	// Not a big issue for now...
}

void	CRenderManager::SetPackPath(const AZStd::string &packPath)
{
	if (!packPath.empty())
		m_RenderBatchFactory.SetPackPath(packPath.c_str());
}

void	CRenderManager::StartUpdate(CParticleMediumCollection *mediumCollection, const SSceneViews *sceneViews)
{
	// Activate distortion passes. We can have several if we have several pipelines.
	// Replace this with a callback when pipeline is recreated if possible
	// or change the pass json to be active by default.

	AZStd::vector<AZ::RPI::Pass*>	distortionPasses;
	AZ::RPI::PassFilter				passFilter = AZ::RPI::PassFilter::CreateWithPassName(AZ::Name("DistortionPass"), m_FeatureProcessor->GetParentScene());
	AZ::RPI::PassSystemInterface::Get()->ForEachPass(passFilter, [&distortionPasses](AZ::RPI::Pass *pass) -> AZ::RPI::PassFilterExecutionFlow
	{
		distortionPasses.push_back(pass);
		return AZ::RPI::PassFilterExecutionFlow::ContinueVisitingPasses;
	});

	AZ_ErrorOnce("PopcornFX", distortionPasses.size() == 0, "RenderManager.cpp: could not find distortion pass.");
	for (auto pass : distortionPasses)
	{
		if (!pass->IsEnabled())
			pass->SetEnabled(true);
	}

	if (sceneViews->m_Views.Empty())
		return;

	m_RenderContext.Clear(m_FeatureProcessor->GetParentScene());
	m_SceneViews = sceneViews;
	mediumCollection->m_OnUpdateComplete += FastDelegate<void(CParticleMediumCollection*)>(this, &CRenderManager::CollectFrame);
}

AZ::RHI::Ptr<AZ::RHI::Buffer>	CRenderManager::ResizeOrCreateBufferIFN(AZ::RHI::Ptr<AZ::RHI::Buffer> buffer, AZ::u64 bufferSize, AZ::u32 alignBufferSize)
{
	if (buffer == null)
		return AllocBuffer(bufferSize, AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead, alignBufferSize);
	if (buffer->GetDescriptor().m_byteCount < bufferSize)
		return AllocBuffer(bufferSize, AZ::RHI::BufferBindFlags::InputAssembly | AZ::RHI::BufferBindFlags::ShaderRead, alignBufferSize);
	return buffer;
}

void	*CRenderManager::MapBuffer(AZ::RHI::Ptr<AZ::RHI::Buffer> buffer, AZ::u64 sizeToMap)
{
	if (buffer == null)
		return null;

	AZ::RHI::BufferMapRequest	mapRequest;
	mapRequest.m_buffer = buffer.get();
	mapRequest.m_byteOffset = 0;
	mapRequest.m_byteCount = sizeToMap;

	AZ::RHI::BufferMapResponse	mapResponse;
	m_BufferPool->MapBuffer(mapRequest, mapResponse);

	return mapResponse.m_data;
}

void	CRenderManager::UnmapBuffer(AZ::RHI::Ptr<AZ::RHI::Buffer> buffer)
{
	if (buffer != null)
		m_BufferPool->UnmapBuffer(*buffer.get());
}

void	CRenderManager::StopUpdate(CParticleMediumCollection *mediumCollection)
{
	mediumCollection->m_OnUpdateComplete -= FastDelegate<void(CParticleMediumCollection*)>(this, &CRenderManager::CollectFrame);

	SParticleCollectedFrameToRender2	*newToRender = m_FrameCollector.GetLastCollectedFrame();
	if (newToRender != null && PK_VERIFY(m_SceneViews != null))
	{
		m_FrameCollector.BuildNewFrame(newToRender);

		const AZ::u32	slicesMaxCount = p_PopcornFXMaxSlices;

		m_FrameCollector.SetDrawCallsSortMethod(slicesMaxCount == 1 ? Sort_None : Sort_Slices);


		PopcornFX::TStaticArray<SSceneView, 1>	viewsArray;
		viewsArray[0].m_InvViewMatrix = m_SceneViews->m_Views[0].m_InvViewMatrix;
		viewsArray[0].m_MaxSliceCount = 1;
		viewsArray[0].m_NeedsSortedIndices = true;
		m_RenderContext.m_Views = viewsArray;

		if (m_FrameCollector.BeginRenderBuiltFrame(m_RenderContext))
			m_FrameCollector.EndRenderBuiltFrame(m_RenderContext, true);

		// To remove once O3DE moves the simple point light processor GPU buffer update into their Render() function instead of Simulate()
#if 1
		m_FeatureProcessor->AppendLightParticles();
#endif
	}

	m_SceneViews = null;
}

void	CRenderManager::CollectFrame(CParticleMediumCollection *mediumCollection)
{
	AZ_UNUSED(mediumCollection);
	m_FrameCollector.CollectFrame();
}

//----------------------------------------------------------------------------

AZ::RHI::Ptr<AZ::RHI::Buffer>	CRenderManager::AllocBuffer(AZ::u64 bufferSize, AZ::RHI::BufferBindFlags binding, AZ::u32 alignSizeOn)
{
	AZ::u64		alignedBufferSize = Mem::Align(bufferSize, alignSizeOn);
	AZ::RHI::Ptr<AZ::RHI::Buffer> outBuffer = AZ::RHI::Factory::Get().CreateBuffer();
	AZ::RHI::BufferInitRequest bufferRequest;
	bufferRequest.m_descriptor = AZ::RHI::BufferDescriptor{ binding, alignedBufferSize };
	bufferRequest.m_buffer = outBuffer.get();
	AZ::RHI::ResultCode result = m_BufferPool->InitBuffer(bufferRequest);

	if (result != AZ::RHI::ResultCode::Success)
	{
		AZ_Error("PopcornFXRenderManager", false, "Failed to create GPU buffers for PopcornFX");
		return null;
	}
	return outBuffer;
}

//----------------------------------------------------------------------------

void	CRenderManager::UnregisterEffectMaterials(const PParticleEffect &effect)
{
	if (PK_VERIFY(effect->EventConnectionMap() != null))
	{
		TMemoryView<const CEventConnectionMap::SLayerDefinition> layers = effect->EventConnectionMap()->m_LayerSlots;

		for (const CEventConnectionMap::SLayerDefinition &layer : layers)
		{
			TMemoryView<const PRendererDataBase>	renderers = layer.m_ParentDescriptor->Renderers();
			//for (const PRendererDataBase &renderer : renderers)
			//{
			//	m_RenderBatchFactory.DecMaterialCacheRef(renderer);
			//}
		}
	}
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
