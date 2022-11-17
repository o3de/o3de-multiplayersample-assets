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

	//! Counts leading zeros
	uint32_t CountLeadingZeros32(uint32_t x)
	{
		return x == 0 ? 32 : az_clz_u32(x);
	}

	//! Integer log2
	uint32_t IntegerLog2(uint32_t x)
	{
		return 31 - CountLeadingZeros32(x);
	}

	const u32 CParticleBuffers::kBufferStrides[] =
	{
		0,	// GenBuffer_Indices

		// Per vertex:
		sizeof(CFloat4), // GenBuffer_Position
		sizeof(CFloat4), // GenBuffer_Normal
		sizeof(CFloat4), // GenBuffer_Tangent
		sizeof(CFloat2), // GenBuffer_UV0
		sizeof(CFloat2), // GenBuffer_UV1
		sizeof(float), // GenBuffer_AtlasId
		sizeof(CFloat4), // GenBuffer_UVRemap
		sizeof(CFloat2), // GenBuffer_UVFactors

		// Per particle:
		sizeof(CFloat4x4), // GenBuffer_Matrices
		sizeof(Drawers::SVertex_PositionDrId), // GenBuffer_ParticlePosition
		sizeof(float), // GenBuffer_ParticleSize
		sizeof(CFloat2), // GenBuffer_ParticleSize2
		sizeof(float), // GenBuffer_ParticleRotation
		sizeof(CFloat3), // GenBuffer_ParticleAxis0
		sizeof(CFloat3), // GenBuffer_ParticleAxis1
		sizeof(CFloat3), // GenBuffer_ParticlePosition0
		sizeof(CFloat3), // GenBuffer_ParticlePosition1
		sizeof(CFloat3), // GenBuffer_ParticlePosition2
	};

	PK_STATIC_ASSERT(PK_ARRAY_COUNT(CParticleBuffers::kBufferStrides) == CParticleBuffers::__GenBuffer_Count);

	void		CParticleBuffers::ResizeOrCreateAdditionalFieldBufferIFN(CStringId additionalFieldName, CRenderManager *renderManager, AZ::u64 bufferSize)
	{
		SAdditionalField	*field = m_AdditionalFields.Find(additionalFieldName);

		if (field == null)
		{
			if (PK_VERIFY(m_AdditionalFields.PushBack(additionalFieldName).Valid()))
				field = &m_AdditionalFields.Last();
		}
		if (field)
			field->m_Buffer = renderManager->ResizeOrCreateBufferIFN(field->m_Buffer, bufferSize);
	}

	//----------------------------------------------------------------------------

	void		CParticleBuffers::UnmapAllAdditionalFieldBuffers(CRenderManager *renderManager)
	{
		for (u32 i = 0; i < m_AdditionalFields.Count(); ++i)
			renderManager->UnmapBuffer(m_AdditionalFields[i].m_Buffer);
	}

	//----------------------------------------------------------------------------

	AZ::RHI::Ptr<AZ::RHI::Buffer>				CParticleBuffers::FindAdditionalFieldBuffer(CStringId additionalFieldName)
	{
		SAdditionalField *field = m_AdditionalFields.Find(additionalFieldName);

		if (field == null)
			return null;
		return field->m_Buffer;
	}

	void	CParticleBuffers::Reset()
	{
		m_AdditionalFields.Clear();
	}

	//----------------------------------------------------------------------------

	void	CParticleBuffers::UnmapAll(CRenderManager *renderManager)
	{
		for (u32 i = 0; i < __GenBuffer_Count; ++i)
		{
			if (m_ViewIndependent.m_GenBuffers[i] != null)
				renderManager->UnmapBuffer(m_ViewIndependent.m_GenBuffers[i]);
		}
		if (m_ViewIndependent.m_DrawRequests != null)
			renderManager->UnmapBuffer(m_ViewIndependent.m_DrawRequests);
		for (u32 i = 0; i < __GenBuffer_Count; ++i)
		{
			if (m_ViewDependent.m_GenBuffers[i] != null)
				renderManager->UnmapBuffer(m_ViewDependent.m_GenBuffers[i]);
		}
		UnmapAllAdditionalFieldBuffers(renderManager);
	}

	//----------------------------------------------------------------------------

	CBatchDrawerCommon::CBatchDrawerCommon()
	:	m_UseLargeIndices(true)
	,	m_RenderContext(null)
	{
		m_HandledAdditionalFields.PushBack(SHandledAdditionalFields(BasicRendererProperties::SID_Diffuse_Color(), BaseType_Float4));
		m_HandledAdditionalFields.PushBack(SHandledAdditionalFields(BasicRendererProperties::SID_Emissive_EmissiveColor(), BaseType_Float3));
		m_HandledAdditionalFields.PushBack(SHandledAdditionalFields(BasicRendererProperties::SID_Distortion_Color(), BaseType_Float4));
		m_HandledAdditionalFields.PushBack(SHandledAdditionalFields(BasicRendererProperties::SID_AlphaRemap_Cursor(), BaseType_Float));
		m_HandledAdditionalFields.PushBack(SHandledAdditionalFields(BasicRendererProperties::SID_Atlas_TextureID(), BaseType_Float));
	}

	//----------------------------------------------------------------------------

	CBatchDrawerCommon::~CBatchDrawerCommon()
	{
	}

	//----------------------------------------------------------------------------

	bool	CBatchDrawerCommon::_AllocViewIndependentBuffers(	const SRendererBatchDrawPass &drawPass,
																u32 particleCount,
																u32 indexCount,
																u32 vertexCount,
																bool useDrawRequestsBuffer)
	{
		PK_SCOPEDPROFILE();
		bool	success = true;
		// -----------------------------------------
		// View independent inputs:
		// -----------------------------------------
		CParticleBuffers::SViewIndependent	&viewIndependent = GetCurBuffers().m_ViewIndependent;
		u32									inputsToGenerate = drawPass.m_ToGenerate.m_GeneratedInputs;

		while (inputsToGenerate != 0)
		{
			CParticleBuffers::EGenBuffer	currentGenBuffer = static_cast<CParticleBuffers::EGenBuffer>(IntegerLog2(inputsToGenerate));

			PK_ASSERT(currentGenBuffer < CParticleBuffers::EGenBuffer::__GenBuffer_Count);

			AZ::RHI::Ptr<AZ::RHI::Buffer>	&currentBufferPtr = viewIndependent.m_GenBuffers[currentGenBuffer];
			const u32						stride = CParticleBuffers::kBufferStrides[currentGenBuffer];

			if (currentGenBuffer == CParticleBuffers::EGenBuffer::GenBuffer_Indices)
			{
				const AZ::u64				idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
				currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, indexCount * idxSize);
			}
			else if (currentGenBuffer < CParticleBuffers::EGenBuffer::GenBuffer_Matrices)
			{
				currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, vertexCount * stride);
			}
			else
			{
				currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, particleCount * stride);
			}

			success &= currentBufferPtr != null;
			inputsToGenerate ^= (1 << currentGenBuffer);
		}
		// Draw request input:
		if (useDrawRequestsBuffer)
		{
			const u32						drawRequestsCount = drawPass.m_DrawRequests.Count();
			AZ::RHI::Ptr<AZ::RHI::Buffer>	&currentBufferPtr = GetCurBuffers().m_ViewIndependent.m_DrawRequests;
			currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, drawRequestsCount * sizeof(Drawers::SBillboardDrawRequest), 0x100);
			success &= currentBufferPtr != null;
		}
		return success;
	}

	//----------------------------------------------------------------------------

	bool	CBatchDrawerCommon::_AllocViewDependentBuffers(	const SRendererBatchDrawPass &drawPass,
															u32 particleCount,
															u32 indexCount,
															u32 vertexCount)
	{
		PK_SCOPEDPROFILE();
		bool	success = true;
		// -----------------------------------------
		// View dependent inputs:
		// -----------------------------------------
		// Just select the first view for now:
		const u32	viewCount = PKMin(drawPass.m_ToGenerate.m_PerViewGeneratedInputs.Count(), 1);

		for (u32 i = 0; i < viewCount; ++i)
		{
			const SGeneratedInputs::SViewGeneratedInputs	&curView = drawPass.m_ToGenerate.m_PerViewGeneratedInputs[i];

			u32	inputsToGenerate = curView.m_GeneratedInputs;

			while (inputsToGenerate != 0)
			{
				CParticleBuffers::EGenBuffer	currentGenBuffer = static_cast<CParticleBuffers::EGenBuffer>(IntegerLog2(inputsToGenerate));

				PK_ASSERT(currentGenBuffer < CParticleBuffers::EGenBuffer::__GenBuffer_Count);

				AZ::RHI::Ptr<AZ::RHI::Buffer>	&currentBufferPtr = GetCurBuffers().m_ViewDependent.m_GenBuffers[currentGenBuffer];
				const u32						stride = CParticleBuffers::kBufferStrides[currentGenBuffer];

				if (currentGenBuffer == CParticleBuffers::EGenBuffer::GenBuffer_Indices)
				{
					const AZ::u64				idxSize = m_UseLargeIndices ? sizeof(AZ::u32) : sizeof(AZ::u16);
					currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, indexCount * idxSize);
				}
				else if (currentGenBuffer < CParticleBuffers::EGenBuffer::GenBuffer_Matrices)
				{
					currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, vertexCount * stride);
				}
				else
				{
					currentBufferPtr = m_RenderContext->m_RenderManager->ResizeOrCreateBufferIFN(currentBufferPtr, particleCount * stride);
				}

				success &= currentBufferPtr != null;
				inputsToGenerate ^= (1 << currentGenBuffer);
			}
		}
		return success;
	}

	//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
