//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "Integration/Managers/RenderManager.h"
#include "BillboardBatchDrawer.h"

#if defined(O3DE_USE_PK)

#include <pk_render_helpers/include/render_features/rh_features_basic.h>
#include <pk_particles/include/Renderers/ps_renderer_feature_fields.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

CBillboardBatchDrawer::CBillboardBatchDrawer()
{
}

//----------------------------------------------------------------------------

CBillboardBatchDrawer::~CBillboardBatchDrawer()
{
}

//----------------------------------------------------------------------------

bool	CBillboardBatchDrawer::AreRenderersCompatible(const CRendererDataBase *rendererA, const CRendererDataBase *rendererB) const
{
	const CAtomRendererCache	*firstAtomCache = static_cast<const CAtomRendererCache*>(rendererA->m_RendererCache.Get());
	const CAtomRendererCache	*secondAtomCache = static_cast<const CAtomRendererCache*>(rendererB->m_RendererCache.Get());

	// Here, you need to implement a proper renderer cache compatibility check
	// Same materials/shader/uniforms/.. should return true
	return *firstAtomCache == *secondAtomCache; // overloaded == operator (see AtomRendererCache.cpp)
}

//----------------------------------------------------------------------------

bool	CBillboardBatchDrawer::AllocBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	PK_NAMEDSCOPEDPROFILE("CAtomBillboardingBatchPolicy::AllocBuffers");
	PK_ASSERT(!drawPass.m_DrawRequests.Empty());
	PK_ASSERT(drawPass.m_DrawRequests.Count() == drawPass.m_RendererCaches.Count());
	PK_ASSERT(drawPass.m_TotalParticleCount > 0);
	PK_ASSERT(drawPass.m_DrawRequests.First() != null && drawPass.m_RendererCaches.First() != null);

	if (m_RenderContext == null)
	{
		m_RenderContext = static_cast<SAtomRenderContext*>(&ctx);
		PK_ASSERT(m_RenderContext != null);
		CAtomRendererCache	*rendererCache = static_cast<CAtomRendererCache*>(drawPass.m_RendererCaches.First().Get());
		PK_ASSERT(rendererCache != null);
		if (rendererCache->m_Atlas != null)
		{
			m_AtlasSubRectsCount = rendererCache->m_Atlas->m_RectsFp32.Count();
			m_AtlasDefinition = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(m_AtlasDefinition, m_AtlasSubRectsCount * sizeof(CFloat4), 0x10);
			if (!PK_VERIFY(m_AtlasDefinition != null))
				return false;
			CFloat4	*atlasSubRects = static_cast<CFloat4*>(m_RenderContext->m_RenderManager->MapBuffer(m_AtlasDefinition, m_AtlasSubRectsCount * sizeof(CFloat4)));
			if (!PK_VERIFY(atlasSubRects != null))
				return false;
			Mem::Copy(atlasSubRects, rendererCache->m_Atlas->m_RectsFp32.RawDataPointer(), m_AtlasSubRectsCount * sizeof(CFloat4));
			m_RenderContext->m_RenderManager->UnmapBuffer(m_AtlasDefinition);
		}

		m_PipelineCaches.Resize(1);

		if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Capsules))
		{
			m_DrawInstanceIdxCount = 12;
			m_DrawInstanceVtxCount = 6;
		}
		else
		{
			m_DrawInstanceIdxCount = 6;
			m_DrawInstanceVtxCount = 4;
		}

		// Create draw instance buffers:
		m_DrawInstanceVtx = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(m_DrawInstanceVtx, 6 * sizeof(CFloat2), 0x10);
		if (!PK_VERIFY(m_DrawInstanceVtx != null))
			return false;

		float	*texCoords = static_cast<float*>(m_RenderContext->m_RenderManager->MapBuffer(m_DrawInstanceVtx, 6 * sizeof(CFloat2)));
		if (!PK_VERIFY(texCoords != null))
			return false;

		texCoords[0] = -1.0f; // Lower left corner
		texCoords[1] = -1.0f;
		texCoords[2] = -1.0f; // Upper left corner
		texCoords[3] = 1.0f;
		texCoords[4] = 1.0f; // Upper right corner
		texCoords[5] = 1.0f;
		texCoords[6] = 1.0f; // Lower right corner
		texCoords[7] = -1.0f;
		texCoords[8] = 0.0f; // Capsule up
		texCoords[9] = 2.0f;
		texCoords[10] = 0.0f; // Capsule down
		texCoords[11] = -2.0f;

		m_RenderContext->m_RenderManager->UnmapBuffer(m_DrawInstanceVtx);

		m_DrawInstanceIdx = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(m_DrawInstanceIdx, 12 * sizeof(u16), 0x10);
		if (!PK_VERIFY(m_DrawInstanceIdx != null))
			return false;

		u16	*indices = static_cast<u16*>(m_RenderContext->m_RenderManager->MapBuffer(m_DrawInstanceIdx, 12 * sizeof(u16)));
		if (!PK_VERIFY(indices != null))
			return false;

		// In O3DE, counter clockwise winding order defines front face
		indices[0] = 0;
		indices[1] = 3;
		indices[2] = 1;
		indices[3] = 1;
		indices[4] = 3;
		indices[5] = 2;
		indices[6] = 0;
		indices[7] = 5;
		indices[8] = 3;
		indices[9] = 1;
		indices[10] = 2;
		indices[11] = 4;

		m_RenderContext->m_RenderManager->UnmapBuffer(m_DrawInstanceIdx);
	}

	++m_FrameIdx;
	m_UseLargeIndices = true; // For now only 32bits indices

	if (!PK_VERIFY(drawPass.m_RendererCaches.Count() > 0))
		return false;

	CAtomRendererCache *rendererCache = static_cast<CAtomRendererCache*>(drawPass.m_RendererCaches.First().Get());

	if (!PK_VERIFY(rendererCache != null))
		return false;

	for (auto &pipelineCache : m_PipelineCaches)
		pipelineCache.InitFromRendererCacheIFN(rendererCache);

	{
		PK_NAMEDSCOPEDPROFILE("CAtomBillboardingBatchPolicy::AllocBuffers Alloc additional inputs");
		const u32	additionalShaderInputsCount = drawPass.m_ToGenerate.m_AdditionalGeneratedInputs.Count();

		for (u32 i = 0; i < additionalShaderInputsCount; ++i)
		{
			const SRendererFeatureFieldDefinition	&curAdditionalShaderInput = drawPass.m_ToGenerate.m_AdditionalGeneratedInputs[i];
			SHandledAdditionalFields	*addFieldDesc = m_HandledAdditionalFields.Find(SHandledAdditionalFields(curAdditionalShaderInput.m_Name, curAdditionalShaderInput.m_Type));
			if (addFieldDesc != null)
			{
				const AZ::u64	typeByteSize = CBaseTypeTraits::Traits(curAdditionalShaderInput.m_Type).Size;
				GetCurBuffers().ResizeOrCreateAdditionalFieldBufferIFN(curAdditionalShaderInput.m_Name, m_RenderContext->m_RenderManager, drawPass.m_TotalParticleCount * typeByteSize);
			}
		}
	}

	const u32	particleCount = drawPass.m_TotalParticleCount;

	// Only per particle buffer:
	if (!PK_VERIFY(_AllocViewIndependentBuffers(drawPass,
												particleCount,
												particleCount,
												particleCount,
												true)))
		return false;
	if (!PK_VERIFY(_AllocViewDependentBuffers(	drawPass,
												particleCount,
												particleCount,
												particleCount)))
		return false;
	return true;
}

//----------------------------------------------------------------------------

bool	CBillboardBatchDrawer::MapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	AZ_UNUSED(ctx);
	const u32									particleCount = drawPass.m_TotalParticleCount;
	const u32									drawRequestsCount = drawPass.m_DrawRequests.Count();
	CRenderManager								*renderManager = m_RenderContext->m_RenderManager;
	const CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;

	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
	{
		const AZ::u64	idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
		void	*data = renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices], particleCount * idxSize);
		m_BBJobs_Billboard.m_Exec_Indices.m_IndexStream.Setup(data, particleCount, m_UseLargeIndices);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticlePosition] != null)
	{
		Drawers::SVertex_PositionDrId	*data = static_cast<Drawers::SVertex_PositionDrId*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticlePosition], particleCount * sizeof(CFloat4)));
		m_BBJobs_Billboard.m_Exec_CopyBillboardingStreams.m_PositionsDrIds = TMemoryView<Drawers::SVertex_PositionDrId>(data, particleCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleRotation] != null)
	{
		float	*data = static_cast<float*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleRotation], particleCount * sizeof(float)));
		m_BBJobs_Billboard.m_Exec_CopyBillboardingStreams.m_Rotations = TMemoryView<float>(data, particleCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize] != null)
	{
		float	*data = static_cast<float*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize], particleCount * sizeof(float)));
		m_BBJobs_Billboard.m_Exec_CopyBillboardingStreams.m_Sizes = TMemoryView<float>(data, particleCount);
	}
	else if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize2] != null)
	{
		CFloat2	*data = static_cast<CFloat2*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize2], particleCount * sizeof(CFloat2)));
		m_BBJobs_Billboard.m_Exec_CopyBillboardingStreams.m_Sizes2 = TMemoryView<CFloat2>(data, particleCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis0] != null)
	{
		CFloat3	*data = static_cast<CFloat3*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis0], particleCount * sizeof(CFloat3)));
		m_BBJobs_Billboard.m_Exec_CopyBillboardingStreams.m_Axis0 = TMemoryView<CFloat3>(data, particleCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis1] != null)
	{
		CFloat3	*data = static_cast<CFloat3*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis1], particleCount * sizeof(CFloat3)));
		m_BBJobs_Billboard.m_Exec_CopyBillboardingStreams.m_Axis1 = TMemoryView<CFloat3>(data, particleCount);
	}

	const u32	additionalShaderInputsCount = drawPass.m_ToGenerate.m_AdditionalGeneratedInputs.Count();

	m_MappedAdditionalShaderInputs.Clear();
	if (!PK_VERIFY(m_MappedAdditionalShaderInputs.Reserve(m_HandledAdditionalFields.Count())))
		return false;
	for (u32 i = 0; i < additionalShaderInputsCount; ++i)
	{
		const SRendererFeatureFieldDefinition	&curAdditionalShaderInput = drawPass.m_ToGenerate.m_AdditionalGeneratedInputs[i];
		SHandledAdditionalFields	*addFieldDesc = m_HandledAdditionalFields.Find(SHandledAdditionalFields(curAdditionalShaderInput.m_Name, curAdditionalShaderInput.m_Type));
		if (addFieldDesc != null)
		{
			if (!PK_VERIFY(m_MappedAdditionalShaderInputs.PushBack().Valid()))
				return false;
			Drawers::SCopyFieldDesc			&desc = m_MappedAdditionalShaderInputs.Last();
			AZ::RHI::Ptr<AZ::RHI::Buffer>	buff = GetCurBuffers().FindAdditionalFieldBuffer(curAdditionalShaderInput.m_Name);
			const AZ::u64					typeByteSize = CBaseTypeTraits::Traits(curAdditionalShaderInput.m_Type).Size;
			u8								*data = static_cast<u8*>(renderManager->MapBuffer(buff, particleCount * typeByteSize));

			desc.m_AdditionalInputIndex = i;
			desc.m_Storage.m_RawDataPtr = data;
			desc.m_Storage.m_Count = particleCount;
			desc.m_Storage.m_Stride = static_cast<u32>(typeByteSize);
		}
	}
	m_BBJobs_Billboard.m_Exec_CopyAdditionalFields.m_FieldsToCopy = m_MappedAdditionalShaderInputs;

	// Draw request buffer:
	if (viewIndependent.m_DrawRequests != null)
	{
		Drawers::SBillboardDrawRequest *data = static_cast<Drawers::SBillboardDrawRequest*>(renderManager->MapBuffer(viewIndependent.m_DrawRequests, drawRequestsCount * sizeof(Drawers::SBillboardDrawRequest)));
		m_BBJobs_Billboard.m_Exec_GeomBillboardDrawRequests.m_GeomDrawRequests = TMemoryView<Drawers::SBillboardDrawRequest>(data, drawRequestsCount);
	}

	// -----------------------------------------
	// View dependent inputs:
	// -----------------------------------------
	const CParticleBuffers::SViewDependent	&viewDependent = GetCurBuffers().m_ViewDependent;
	const u32								viewCount = PKMin(drawPass.m_ToGenerate.m_PerViewGeneratedInputs.Count(), 1);

	for (u32 i = 0; i < viewCount; ++i)
	{
		//const SGeneratedInputs::SViewGeneratedInputs	&curView = toMap.m_PerViewGeneratedInputs[i];
		SGPUBillboardBatchJobs::SPerView				&curViewJobs = m_BBJobs_Billboard.m_PerView[i];

		if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
		{
			const AZ::u64	idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
			void			*data = renderManager->MapBuffer(viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices], drawPass.m_TotalParticleCount * idxSize);
			curViewJobs.m_Exec_Indices.m_IndexStream.Setup(data, drawPass.m_TotalParticleCount, m_UseLargeIndices);
		}
	}
	return true;
}

//----------------------------------------------------------------------------

bool	CBillboardBatchDrawer::UnmapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	AZ_UNUSED(ctx);
	AZ_UNUSED(drawPass);
	CRenderManager	*renderManager = m_RenderContext->m_RenderManager;
	GetCurBuffers().UnmapAll(renderManager);
	return true;
}

//----------------------------------------------------------------------------

bool	CBillboardBatchDrawer::EmitDrawCall(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass, const SDrawCallDesc &toEmit)
{
	AZ_UNUSED(ctx);
	SAtomRenderContext::SDrawCall		dc;

	for (const auto &pipelineCache : m_PipelineCaches)
	{
		if (!pipelineCache.IsInitialized())
			return true;
	}

	const u32									particleCount = drawPass.m_TotalParticleCount;
	const u32									drawRequestsCount = drawPass.m_DrawRequests.Count();
	const CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;
	const CParticleBuffers::SViewDependent		&viewDependent = GetCurBuffers().m_ViewDependent;
	const CAtomRendererCache					*rendererCache = static_cast<const CAtomRendererCache*>(toEmit.m_RendererCaches.First().Get());

	if (!PK_VERIFY(rendererCache != null))
		return false;

	dc.m_RendererType = Renderer_Billboard;
	// For billboard renderer, we use a draw instance
	if (!PK_VERIFY(m_PipelineCaches.Count() == 1))
		return false;

	// ----------------------------------------------------------------
	// The pipeline cache handles the buffer bindings for the draw call:
	// Add draw instance vertex and index buffers:
	m_PipelineCaches[0].SetVertexInputBuffer(DrawInstanceTexCoords_Vertex, AZ::RHI::StreamBufferView(*m_DrawInstanceVtx, 0, m_DrawInstanceVtxCount * sizeof(CFloat2), sizeof(CFloat2)));
	m_PipelineCaches[0].SetIndexBuffer(AZ::RHI::IndexBufferView(*m_DrawInstanceIdx, 0, m_DrawInstanceIdxCount * sizeof(u16), AZ::RHI::IndexFormat::Uint16));
	m_PipelineCaches[0].SetBillboardingSrgConstantValue(BillboardSrg::RendererFlags_ShaderRead, rendererCache->m_BasicDescription.m_RendererFlags);

	if (m_AtlasDefinition != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = m_AtlasDefinition->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, m_AtlasSubRectsCount, sizeof(CFloat4)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::AtlasSubRects_ShaderRead, buff);
		m_PipelineCaches[0].SetBillboardingSrgConstantValue(BillboardSrg::AtlasSubRectsCount_ShaderRead, m_AtlasSubRectsCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
	{
		// Unsorted indices:
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(u32)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleIndices_ShaderRead, buff);
	}
	else if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
	{
		// Sorted indices:
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(u32)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleIndices_ShaderRead, buff);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticlePosition] != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticlePosition]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(CFloat4)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticlePositions_ShaderRead, buff);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleRotation] != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleRotation]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(float)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleRotations_ShaderRead, buff);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis0] != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis0]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount * 3, sizeof(float))); // Cannot use a float3 stride
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleAxis0_ShaderRead, buff);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis1] != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleAxis1]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount * 3, sizeof(float))); // Cannot use a float3 stride
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleAxis1_ShaderRead, buff);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize] != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(float)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleSizes_ShaderRead, buff);
	}
	else if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize2] != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_ParticleSize2]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(CFloat2)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleSizes2_ShaderRead, buff);
	}
	// Additional field buffers:
	AZ::RHI::Ptr<AZ::RHI::Buffer>	textureID = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Atlas_TextureID());
	if (textureID != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = textureID->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(float)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleAtlasID_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	diffuseColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Diffuse_Color());
	if (diffuseColor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = diffuseColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(CFloat4)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleDiffuseColor_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	emissiveColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Emissive_EmissiveColor());
	if (emissiveColor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = emissiveColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount * 3, sizeof(float)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleEmissiveColor_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	distortionColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Distortion_Color());
	if (distortionColor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = distortionColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(CFloat4)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleDistortionColor_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	alphaRemapCursor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_AlphaRemap_Cursor());
	if (alphaRemapCursor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = alphaRemapCursor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(float)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::ParticleAlphaCursor_ShaderRead, buff);
	}

	// Draw requests:
	if (viewIndependent.m_DrawRequests != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_DrawRequests->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, drawRequestsCount, sizeof(Drawers::SBillboardDrawRequest)));
		m_PipelineCaches[0].SetBillboardingSrgBuffer(BillboardSrg::DrawRequest_ShaderRead, buff);
	}

	// Fill the draw call:
	m_PipelineCaches[0].ConfigureDrawCall(dc);
	// ----------------------------------------------------------------

	// Draw call bounding box:
	dc.m_BoundingBox = toEmit.m_BBox;

	// Draw call description:
	dc.m_DrawIndexed.m_indexCount = m_DrawInstanceIdxCount;
	dc.m_DrawIndexed.m_indexOffset = 0;
	dc.m_DrawIndexed.m_instanceCount = particleCount;
	dc.m_DrawIndexed.m_instanceOffset = 0;
	dc.m_DrawIndexed.m_vertexOffset = 0;

	// Draw instance indices and tex-coords:
	return PK_VERIFY(m_RenderContext->m_DrawCalls.PushBack(dc).Valid());
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
