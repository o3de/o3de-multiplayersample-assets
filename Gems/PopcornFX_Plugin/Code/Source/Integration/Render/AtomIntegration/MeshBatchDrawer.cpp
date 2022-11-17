//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "Integration/Managers/RenderManager.h"
#include "MeshBatchDrawer.h"

#if defined(O3DE_USE_PK)

#include <pk_render_helpers/include/render_features/rh_features_basic.h>
#include <pk_particles/include/Renderers/ps_renderer_feature_fields.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

CMeshBatchDrawer::CMeshBatchDrawer()
{
}

//----------------------------------------------------------------------------

CMeshBatchDrawer::~CMeshBatchDrawer()
{
}

//----------------------------------------------------------------------------

bool	CMeshBatchDrawer::AreRenderersCompatible(const CRendererDataBase *rendererA, const CRendererDataBase *rendererB) const
{
	const CAtomRendererCache	*firstAtomCache = static_cast<const CAtomRendererCache*>(rendererA->m_RendererCache.Get());
	const CAtomRendererCache	*secondAtomCache = static_cast<const CAtomRendererCache*>(rendererB->m_RendererCache.Get());

	// Here, you need to implement a proper renderer cache compatibility check
	// Same materials/shader/uniforms/.. should return true
	return *firstAtomCache == *secondAtomCache; // overloaded == operator (see AtomRendererCache.cpp)
}

//----------------------------------------------------------------------------

bool	CMeshBatchDrawer::AllocBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	PK_SCOPEDPROFILE();
	PK_ASSERT(!drawPass.m_DrawRequests.Empty());
	PK_ASSERT(drawPass.m_DrawRequests.Count() == drawPass.m_RendererCaches.Count());
	PK_ASSERT(drawPass.m_TotalParticleCount > 0);
	PK_ASSERT(drawPass.m_DrawRequests.First() != null && drawPass.m_RendererCaches.First() != null);

	if (!PK_VERIFY(drawPass.m_RendererCaches.Count() > 0))
		return false;

	CAtomRendererCache *rendererCache = static_cast<CAtomRendererCache*>(drawPass.m_RendererCaches.First().Get());
	if (!PK_VERIFY(rendererCache != null))
		return false;

	if (m_RenderContext == null)
	{
		m_RenderContext = static_cast<SAtomRenderContext*>(&ctx);
		PK_ASSERT(m_RenderContext != null);

		m_GeometryCache = rendererCache->m_CacheFactory->FindGeometryCache(rendererCache->m_BasicDescription.m_MeshPath.ToString());
		if (!PK_VERIFY(m_GeometryCache != null))
			return false;
		if (!m_GeometryCache->m_Modified) // not ready yet
			return true;
	}

	++m_FrameIdx;
	m_UseLargeIndices = true; // For now only 32bits indices

	if (m_PipelineCaches.Count() != m_GeometryCache->m_PerGeometryViews.Count())
		m_PipelineCaches.Resize(m_GeometryCache->m_PerGeometryViews.Count());

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

	const u32									particleCount = drawPass.m_TotalParticleCount;

	// Only per particle buffer:
	if (!PK_VERIFY(_AllocViewIndependentBuffers(drawPass,
												particleCount,
												particleCount,
												particleCount,
												false)))
		return false;
	if (!PK_VERIFY(_AllocViewDependentBuffers(	drawPass,
												particleCount,
												particleCount,
												particleCount)))
		return false;
	return true;
}

//----------------------------------------------------------------------------

bool	CMeshBatchDrawer::MapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	AZ_UNUSED(ctx);
	const u32									particleCount = drawPass.m_TotalParticleCount;
	[[maybe_unused]] const u32									drawRequestsCount = drawPass.m_DrawRequests.Count();
	CRenderManager								*renderManager = m_RenderContext->m_RenderManager;
	const CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;

	if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Matrices] != null)
	{
		CFloat4x4	*data = static_cast<CFloat4x4*>(renderManager->MapBuffer(viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Matrices], particleCount * sizeof(CFloat4x4)));
		m_BBJobs_Mesh.m_Exec_Matrices.m_Matrices = TMemoryView<CFloat4x4>(data, particleCount);
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
	m_BBJobs_Mesh.m_Exec_CopyField.m_FieldsToCopy = m_MappedAdditionalShaderInputs.View();
	return true;
}

//----------------------------------------------------------------------------

bool	CMeshBatchDrawer::UnmapBuffers(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass)
{
	AZ_UNUSED(ctx);
	AZ_UNUSED(drawPass);
	CRenderManager	*renderManager = m_RenderContext->m_RenderManager;
	GetCurBuffers().UnmapAll(renderManager);
	return true;
}

//----------------------------------------------------------------------------

bool	CMeshBatchDrawer::EmitDrawCall(SRenderContext &ctx, const SRendererBatchDrawPass &drawPass, const SDrawCallDesc &toEmit)
{
	AZ_UNUSED(ctx);
	SAtomRenderContext::SDrawCall		dc;

	for (const auto &pipelineCache : m_PipelineCaches)
	{
		if (!pipelineCache.IsInitialized())
			return true;
	}

	const u32									particleCount = drawPass.m_TotalParticleCount;
	const SRendererBatchDrawPass_Mesh_CPUBB		*meshDrawPass = static_cast<const SRendererBatchDrawPass_Mesh_CPUBB*>(&drawPass);
	[[maybe_unused]] const u32					drawRequestsCount = drawPass.m_DrawRequests.Count();
	const CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;
	const CAtomRendererCache					*rendererCache = static_cast<const CAtomRendererCache*>(toEmit.m_RendererCaches.First().Get());

	if (!PK_VERIFY(rendererCache != null))
		return false;

	dc.m_RendererType = Renderer_Mesh;

	if (!PK_VERIFY(m_GeometryCache != null))
		return false;

	u32		particleOffset = 0;
	const bool	meshAtlas = rendererCache->m_BasicDescription.m_UseMeshAtlas;

	for (u32 i = 0; i < m_GeometryCache->m_PerGeometryViews.Count(); i++)
	{
		const u32 meshParticleCount = meshAtlas ? meshDrawPass->m_PerMeshParticleCount[i] : particleCount;
		if (meshParticleCount <= 0)
			continue;
		const CGeometryCache::GPUBufferViews &views = m_GeometryCache->m_PerGeometryViews[i];

		// Setup index and vertex buffers
		// TODO: For mesh and bb draw instanced, vertex buffers are constants. This could be moved
		// in CAtomPipelineCache::InitFromRendererCacheIFN and done once
		// (clear must removed of m_PipelineCache.ConfigureDrawCall(dc))
		// At least finding the geometry cache can be done once
		m_PipelineCaches[i].SetIndexBuffer(views.m_IndexBuffer);

		if (views.m_PositionBuffer.GetBuffer() != null)
			m_PipelineCaches[i].SetVertexInputBuffer(Position_Vertex, views.m_PositionBuffer);
		if (views.m_UVBuffer.GetBuffer() != null)
			m_PipelineCaches[i].SetVertexInputBuffer(TexCoord0_Vertex, views.m_UVBuffer);
		if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Lighting))
		{
			if (views.m_NormalBuffer.GetBuffer() != null)
				m_PipelineCaches[i].SetVertexInputBuffer(Normal_Vertex, views.m_NormalBuffer);
			if (views.m_TangentBuffer.GetBuffer() != null)
				m_PipelineCaches[i].SetVertexInputBuffer(Tangent_Vertex, views.m_TangentBuffer);
		}

		// Constant data: setup renderer flags
		m_PipelineCaches[i].SetMeshSrgConstantValue(MeshSrg::RendererFlags_ShaderRead, rendererCache->m_BasicDescription.m_RendererFlags);

		// Non constant data: matrices
		if (viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Matrices] != null)
		{
			AZ::RHI::Ptr<AZ::RHI::BufferView> buff = viewIndependent.m_GenBuffers[CParticleBuffers::GenBuffer_Matrices]->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(particleOffset, particleCount, sizeof(CFloat4x4)));
			m_PipelineCaches[i].SetMeshSrgBuffer(MeshSrg::Matrices_ShaderRead, buff);
		}

		// Non constant data: additionnal inputs
		AZ::RHI::Ptr<AZ::RHI::Buffer>	diffuseColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Diffuse_Color());
		if (diffuseColor != null)
		{
			AZ::RHI::Ptr<AZ::RHI::BufferView> buff = diffuseColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(particleOffset, meshParticleCount, sizeof(CFloat4)));
			m_PipelineCaches[i].SetMeshSrgBuffer(MeshSrg::ParticleDiffuseColor_ShaderRead, buff);
		}
		AZ::RHI::Ptr<AZ::RHI::Buffer>	emissiveColor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_Emissive_EmissiveColor());
		if (emissiveColor != null)
		{
			AZ::RHI::Ptr<AZ::RHI::BufferView> buff = emissiveColor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(particleOffset * 3, meshParticleCount * 3, sizeof(float)));
			m_PipelineCaches[i].SetMeshSrgBuffer(MeshSrg::ParticleEmissiveColor_ShaderRead, buff);
		}
		AZ::RHI::Ptr<AZ::RHI::Buffer>	alphaRemapCursor = GetCurBuffers().FindAdditionalFieldBuffer(BasicRendererProperties::SID_AlphaRemap_Cursor());
		if (alphaRemapCursor != null)
		{
			AZ::RHI::Ptr<AZ::RHI::BufferView> buff = alphaRemapCursor->GetBufferView(AZ::RHI::BufferViewDescriptor::CreateStructured(particleOffset, meshParticleCount, sizeof(float)));
			m_PipelineCaches[i].SetMeshSrgBuffer(MeshSrg::ParticleAlphaCursor_ShaderRead, buff);
		}

		m_PipelineCaches[i].ConfigureDrawCall(dc);

		// Draw call bounding box:
		dc.m_BoundingBox = toEmit.m_BBox;

		// Draw call description:
		dc.m_DrawIndexed.m_indexCount = views.m_IndexCount;
		dc.m_DrawIndexed.m_indexOffset = 0;
		dc.m_DrawIndexed.m_instanceCount = meshParticleCount;
		dc.m_DrawIndexed.m_instanceOffset = 0;
		dc.m_DrawIndexed.m_vertexOffset = 0;

		// Draw instance indices and tex-coords:
		if (!PK_VERIFY(m_RenderContext->m_DrawCalls.PushBack(dc).Valid()))
			return false;
		particleOffset += meshAtlas ? meshParticleCount : 0;
	}
	return true;
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
