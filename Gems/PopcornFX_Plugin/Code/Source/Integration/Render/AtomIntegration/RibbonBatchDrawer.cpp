//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "Integration/Managers/RenderManager.h"
#include "RibbonBatchDrawer.h"

#if defined(O3DE_USE_PK)

#include <pk_render_helpers/include/render_features/rh_features_basic.h>
#include <pk_particles/include/Renderers/ps_renderer_feature_fields.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

CRibbonBatchDrawer::CRibbonBatchDrawer()
{
}

//----------------------------------------------------------------------------

CRibbonBatchDrawer::~CRibbonBatchDrawer()
{
}

//----------------------------------------------------------------------------

bool	CRibbonBatchDrawer::AreRenderersCompatible(const CRendererDataBase *rendererA, const CRendererDataBase *rendererB) const
{
	const CAtomRendererCache	*firstAtomCache = static_cast<const CAtomRendererCache*>(rendererA->m_RendererCache.Get());
	const CAtomRendererCache	*secondAtomCache = static_cast<const CAtomRendererCache*>(rendererB->m_RendererCache.Get());

	// Here, you need to implement a proper renderer cache compatibility check
	// Same materials/shader/uniforms/.. should return true
	return *firstAtomCache == *secondAtomCache; // overloaded == operator (see AtomRendererCache.cpp)
}

//----------------------------------------------------------------------------

bool	CRibbonBatchDrawer::AllocBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	PK_SCOPEDPROFILE();

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

	const SRendererBatchDrawPass_Ribbon_CPUBB	*ribbonDrawPass = static_cast<const SRendererBatchDrawPass_Ribbon_CPUBB*>(&drawPass);

	if (!PK_VERIFY(_AllocViewIndependentBuffers(drawPass,
												ribbonDrawPass->m_TotalParticleCount,
												ribbonDrawPass->m_TotalIndexCount,
												ribbonDrawPass->m_TotalVertexCount,
												false)))
		return false;
	if (!PK_VERIFY(_AllocViewDependentBuffers(	drawPass,
												ribbonDrawPass->m_TotalParticleCount,
												ribbonDrawPass->m_TotalIndexCount,
												ribbonDrawPass->m_TotalVertexCount)))
		return false;
	return true;
}

//----------------------------------------------------------------------------

bool	CRibbonBatchDrawer::MapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	AZ_UNUSED(ctx);
	const SRendererBatchDrawPass_Ribbon_CPUBB	*ribbonDrawPass = static_cast<const SRendererBatchDrawPass_Ribbon_CPUBB*>(&drawPass);
	const u32									particleCount = drawPass.m_TotalParticleCount;
	const u32									vertexCount = ribbonDrawPass->m_TotalVertexCount;
	const u32									indexCount = ribbonDrawPass->m_TotalIndexCount;
	CRenderManager								*renderManager = m_RenderContext->m_RenderManager;
	const CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;

	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
	{
		const AZ::u64	idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
		void	*data = renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices], indexCount * idxSize);
		m_BBJobs_Ribbon.m_Exec_Indices.m_IndexStream.Setup(data, indexCount, m_UseLargeIndices);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position] != null)
	{
		CFloat3		*data = static_cast<CFloat3*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position], vertexCount * sizeof(CFloat4)));
		m_BBJobs_Ribbon.m_Exec_PNT.m_Positions = TStridedMemoryView<CFloat3, 0x10>(data, vertexCount, 0x10);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal] != null)
	{
		CFloat3		*data = static_cast<CFloat3*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal], vertexCount * sizeof(CFloat4)));
		m_BBJobs_Ribbon.m_Exec_PNT.m_Normals = TStridedMemoryView<CFloat3, 0x10>(data, vertexCount, 0x10);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent] != null)
	{
		CFloat4		*data = static_cast<CFloat4*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent], vertexCount * sizeof(CFloat4)));
		m_BBJobs_Ribbon.m_Exec_PNT.m_Tangents = TStridedMemoryView<CFloat4>(data, vertexCount, 0x10);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV0] != null)
	{
		CFloat2		*data = static_cast<CFloat2*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV0], vertexCount * sizeof(CFloat2)));
		m_BBJobs_Ribbon.m_Exec_Texcoords.m_Texcoords = TMemoryView<CFloat2>(data, vertexCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV1] != null)
	{
		CFloat2		*data = static_cast<CFloat2*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV1], vertexCount * sizeof(CFloat2)));
		m_BBJobs_Ribbon.m_Exec_Texcoords.m_Texcoords2 = TMemoryView<CFloat2>(data, vertexCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_AtlasId] != null)
	{
		float		*data = static_cast<float*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_AtlasId], vertexCount * sizeof(float)));
		m_BBJobs_Ribbon.m_Exec_Texcoords.m_AtlasIds = TMemoryView<float>(data, vertexCount);
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVRemap] != null)
	{
		CFloat4		*data = static_cast<CFloat4*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVRemap], vertexCount * sizeof(CFloat4)));
		m_BBJobs_Ribbon.m_Exec_UVRemap.m_UVRemap = TMemoryView<CFloat4>(data, vertexCount);
		m_BBJobs_Ribbon.m_Exec_Texcoords.m_ForUVFactor = true;
	}
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors] != null)
	{
		CFloat2		*data = static_cast<CFloat2*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors], vertexCount * sizeof(CFloat2)));
		m_BBJobs_Ribbon.m_Exec_PNT.m_UVFactors = TMemoryView<CFloat2>(data, vertexCount);
	}

	m_BBJobs_Ribbon.m_Exec_CopyField.m_PerVertex = false;

	// Additional fields:
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

	m_BBJobs_Ribbon.m_Exec_CopyField.m_FieldsToCopy = m_MappedAdditionalShaderInputs;

	// -----------------------------------------
	// View dependent inputs:
	// -----------------------------------------
	const CParticleBuffers::SViewDependent	&viewDependent = GetCurBuffers().m_ViewDependent;
	const u32								viewCount = PKMin(drawPass.m_ToGenerate.m_PerViewGeneratedInputs.Count(), 1);

	for (u32 i = 0; i < viewCount; ++i)
	{
		//const SGeneratedInputs::SViewGeneratedInputs	&curView = toMap.m_PerViewGeneratedInputs[i];
		SRibbonBatchJobs::SPerView						&curViewJobs = m_BBJobs_Ribbon.m_PerView[i];

		if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
		{
			const AZ::u64	idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
			void			*data = renderManager->MapBuffer(viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices], indexCount * idxSize);
			curViewJobs.m_Exec_Indices.m_IndexStream.Setup(data, indexCount, m_UseLargeIndices);
		}
		if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position] != null)
		{
			CFloat3		*data = static_cast<CFloat3*>(renderManager->MapBuffer(viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position], vertexCount * sizeof(CFloat4)));
			curViewJobs.m_Exec_PNT.m_Positions = TStridedMemoryView<CFloat3, 0x10>(data, vertexCount, 0x10);
		}
		if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal] != null)
		{
			CFloat3		*data = static_cast<CFloat3*>(renderManager->MapBuffer(viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal], vertexCount * sizeof(CFloat4)));
			curViewJobs.m_Exec_PNT.m_Normals = TStridedMemoryView<CFloat3, 0x10>(data, vertexCount, 0x10);
		}
		if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent] != null)
		{
			CFloat4		*data = static_cast<CFloat4*>(renderManager->MapBuffer(viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent], vertexCount * sizeof(CFloat4)));
			curViewJobs.m_Exec_PNT.m_Tangents = TStridedMemoryView<CFloat4>(data, vertexCount, 0x10);
		}
		if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors] != null)
		{
			CFloat2		*data = static_cast<CFloat2*>(renderManager->MapBuffer(viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors], vertexCount * sizeof(CFloat2)));
			curViewJobs.m_Exec_PNT.m_UVFactors = TMemoryView<CFloat2>(data, vertexCount);
		}
	}
	return true;
}

//----------------------------------------------------------------------------

bool	CRibbonBatchDrawer::UnmapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	AZ_UNUSED(ctx);
	AZ_UNUSED(drawPass);
	CRenderManager	*renderManager = m_RenderContext->m_RenderManager;
	GetCurBuffers().UnmapAll(renderManager);
	return true;
}

//----------------------------------------------------------------------------

bool	CRibbonBatchDrawer::EmitDrawCall(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass, const SDrawCallDesc &toEmit)
{
	AZ_UNUSED(ctx);
	SAtomRenderContext::SDrawCall		dc;

	for (const auto &pipelineCache : m_PipelineCaches)
	{
		if (!pipelineCache.IsInitialized())
			return true;
	}

	const SRendererBatchDrawPass_Ribbon_CPUBB	*ribbonDrawPass = static_cast<const SRendererBatchDrawPass_Ribbon_CPUBB*>(&drawPass);
	const u32									particleCount = drawPass.m_TotalParticleCount;
	const u32									vertexCount = ribbonDrawPass->m_TotalVertexCount;
	const u32									indexCount = ribbonDrawPass->m_TotalIndexCount;
	const CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;
	const CParticleBuffers::SViewDependent		&viewDependent = GetCurBuffers().m_ViewDependent;
	const CAtomRendererCache					*rendererCache = static_cast<const CAtomRendererCache*>(toEmit.m_RendererCaches.First().Get());

	if (!PK_VERIFY(rendererCache != null))
		return false;

	dc.m_RendererType = Renderer_Ribbon;
	if (!PK_VERIFY(m_PipelineCaches.Count() == 1))
		return false;

	// Position:
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(Position_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));
	else if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(Position_Vertex, AZ::RHI::StreamBufferView(*viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Position], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));
	// Normal:
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(Normal_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));
	else if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(Normal_Vertex, AZ::RHI::StreamBufferView(*viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Normal], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));
	// Tangent:
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(Tangent_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));
	else if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(Tangent_Vertex, AZ::RHI::StreamBufferView(*viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Tangent], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));

	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV0] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(TexCoord0_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV0], 0, vertexCount * sizeof(CFloat2), sizeof(CFloat2)));

	// Linear atlas blending:
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV1] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(TexCoord1_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UV1], 0, vertexCount * sizeof(CFloat2), sizeof(CFloat2)));
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_AtlasId] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(TexFrameLerp_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_AtlasId], 0, vertexCount * sizeof(float), sizeof(float)));

	// Correct deformation:
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVRemap] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(UVRemap_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVRemap], 0, vertexCount * sizeof(CFloat4), sizeof(CFloat4)));
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(UVFactors_Vertex, AZ::RHI::StreamBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors], 0, vertexCount * sizeof(CFloat2), sizeof(CFloat2)));
	else if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors] != null)
		m_PipelineCaches[0].SetVertexInputBuffer(UVFactors_Vertex, AZ::RHI::StreamBufferView(*viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_UVFactors], 0, vertexCount * sizeof(CFloat2), sizeof(CFloat2)));

	const AZ::u32				idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
	const AZ::RHI::IndexFormat	idxFormat = m_UseLargeIndices ? AZ::RHI::IndexFormat::Uint32 : AZ::RHI::IndexFormat::Uint16;
	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
	{
		// Unsorted indices:
		m_PipelineCaches[0].SetIndexBuffer(AZ::RHI::IndexBufferView(*viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices], 0, indexCount * idxSize, idxFormat));
	}
	else if (viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices] != null)
	{
		// Sorted indices:
		m_PipelineCaches[0].SetIndexBuffer(AZ::RHI::IndexBufferView(*viewDependent.m_GenBuffers[CParticleBuffers::GenBuffer_Indices], 0, indexCount * idxSize, idxFormat));
	}

	m_PipelineCaches[0].SetRibbonSrgConstantValue(RibbonSrg::RendererFlags_ShaderRead, rendererCache->m_BasicDescription.m_RendererFlags);
	m_PipelineCaches[0].SetRibbonSrgConstantValue(RibbonSrg::ParticleCount_ShaderRead, particleCount);

	// Additional field buffers:
	AZ::RHI::Ptr<AZ::RHI::Buffer>	diffuseColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Diffuse_Color());
	if (diffuseColor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = diffuseColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(CFloat4)));
		m_PipelineCaches[0].SetRibbonSrgBuffer(RibbonSrg::ParticleDiffuseColor_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	emissiveColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Emissive_EmissiveColor());
	if (emissiveColor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = emissiveColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount * 3, sizeof(float)));
		m_PipelineCaches[0].SetRibbonSrgBuffer(RibbonSrg::ParticleEmissiveColor_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	distortionColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Distortion_Color());
	if (distortionColor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = distortionColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(CFloat4)));
		m_PipelineCaches[0].SetRibbonSrgBuffer(RibbonSrg::ParticleDistortionColor_ShaderRead, buff);
	}
	AZ::RHI::Ptr<AZ::RHI::Buffer>	alphaRemapCursor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_AlphaRemap_Cursor());
	if (alphaRemapCursor != null)
	{
		AZ::RHI::Ptr<AZ::RHI::BufferView> buff = alphaRemapCursor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(0, particleCount, sizeof(float)));
		m_PipelineCaches[0].SetRibbonSrgBuffer(RibbonSrg::ParticleAlphaCursor_ShaderRead, buff);
	}

	// Fill the draw call:
	m_PipelineCaches[0].ConfigureDrawCall(dc);
	// ----------------------------------------------------------------

	// Draw call bounding box:
	dc.m_BoundingBox = toEmit.m_BBox;

	// Draw call description:
	dc.m_DrawIndexed.m_indexCount = indexCount;
	dc.m_DrawIndexed.m_indexOffset = 0;
	dc.m_DrawIndexed.m_instanceCount = 1;
	dc.m_DrawIndexed.m_instanceOffset = 0;
	dc.m_DrawIndexed.m_vertexOffset = 0;

	// Draw instance indices and tex-coords:
	return PK_VERIFY(m_RenderContext->m_DrawCalls.PushBack(dc).Valid());
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
