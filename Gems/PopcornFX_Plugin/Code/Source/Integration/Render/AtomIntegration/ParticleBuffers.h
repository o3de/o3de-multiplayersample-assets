//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include "AtomFrameCollectorTypes.h"
#include "AtomPipelineCache.h"
#include "AtomRendererCache.h"
#include <Atom/RHI/Buffer.h>
#include <pk_kernel/include/kr_memoryviews.h>
#include <pk_kernel/include/kr_mem.h>
#include <pk_maths/include/pk_numeric_tools.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

struct SRendererBatchDrawPass;

uint32_t	CountLeadingZeros32(uint32_t x);
uint32_t	IntegerLog2(uint32_t x);

class	CParticleBuffers
{
public:
	CParticleBuffers() { }
	~CParticleBuffers()
	{
		Reset();
	}

	void								ResizeOrCreateAdditionalFieldBufferIFN(CStringId additionalFieldName, CRenderManager *renderManager, AZ::u64 bufferSize);
	void								UnmapAllAdditionalFieldBuffers(CRenderManager *renderManager);
	AZ::RHI::Ptr<AZ::RHI::Buffer>		FindAdditionalFieldBuffer(CStringId additionalFieldName);

	void								Reset();
	void								UnmapAll(CRenderManager *renderManager);

	enum	EGenBuffer
	{
		// Per particle for GPU billbording / per vertex for CPU billboarding:
		GenBuffer_Indices,

		// Per vertex:
		GenBuffer_Position,
		GenBuffer_Normal,
		GenBuffer_Tangent,
		GenBuffer_UV0,
		GenBuffer_UV1,
		GenBuffer_AtlasId,
		GenBuffer_UVRemap,
		GenBuffer_UVFactors,

		// Per particle:
		GenBuffer_Matrices,
		GenBuffer_ParticlePosition,
		GenBuffer_ParticleSize,
		GenBuffer_ParticleSize2,
		GenBuffer_ParticleRotation,
		GenBuffer_ParticleAxis0,
		GenBuffer_ParticleAxis1,
		GenBuffer_ParticlePosition0,
		GenBuffer_ParticlePosition1,
		GenBuffer_ParticlePosition2,

		__GenBuffer_Count
	};

	// Element sizes:
	static const u32	kBufferStrides[];

	PK_STATIC_ASSERT(TIntegerTemplateTools::Log2<Drawers::GenInput_ParticlePosition2>::Value + 1 == __GenBuffer_Count);

	struct	SViewIndependent
	{
		AZ::RHI::Ptr<AZ::RHI::Buffer>	m_GenBuffers[__GenBuffer_Count];
		// For GPU billboarding:
		AZ::RHI::Ptr<AZ::RHI::Buffer>	m_DrawRequests;

		SViewIndependent() { Mem::Clear(*this); }
	};

	struct	SViewDependent
	{
		AZ::RHI::Ptr<AZ::RHI::Buffer>	m_GenBuffers[__GenBuffer_Count];

		SViewDependent() { Mem::Clear(*this); }
	};

	// --------------------------------
	// For ribbons:
	// --------------------------------
	SViewIndependent	m_ViewIndependent;
	// We only handle one camera for the moment:
	SViewDependent		m_ViewDependent;
	// --------------------------------

private:
	struct	SAdditionalField
	{
		CStringId						m_Name;
		AZ::RHI::Ptr<AZ::RHI::Buffer>	m_Buffer;

		SAdditionalField() { }
		SAdditionalField(const CStringId &name)
			: m_Name(name)
			, m_Buffer(null)
		{
		}

		bool	operator==(const SAdditionalField &oth) const { return m_Name == oth.m_Name; }
		bool	operator!=(const SAdditionalField &oth) const { return m_Name != oth.m_Name; }
	};

	TArray<SAdditionalField>					m_AdditionalFields;
};

//----------------------------------------------------------------------------

class	CBatchDrawerCommon
{
public:
	CBatchDrawerCommon();
	~CBatchDrawerCommon();

protected:
	// Store the buffers needed to render the particles here...
	bool					m_UseLargeIndices;
	SAtomRenderContext		*m_RenderContext;

	// Multi-buffered GPU buffers:
#define MULTI_BUFFERED_COUNT	1
	CParticleBuffers &GetCurBuffers() { return m_Buffers[m_FrameIdx % MULTI_BUFFERED_COUNT]; }

	u32									m_FrameIdx;
	CParticleBuffers					m_Buffers[MULTI_BUFFERED_COUNT];
	TArray<CAtomPipelineCache>			m_PipelineCaches; // We can have several if mesh atlas

	struct	SHandledAdditionalFields
	{
		CStringId						m_Name;
		EBaseTypeID						m_Type = __MaxBaseTypes;

		SHandledAdditionalFields(CStringId name,
			EBaseTypeID type)
			: m_Name(name)
			, m_Type(type)
		{
		}

		bool	operator==(const SHandledAdditionalFields &oth) const
		{
			return	m_Name == oth.m_Name &&
				m_Type == oth.m_Type;
		}
	};
	TSemiDynamicArray<SHandledAdditionalFields, 6>	m_HandledAdditionalFields;

	bool	_AllocViewIndependentBuffers(	const SRendererBatchDrawPass &drawPass,
											u32 particleCount,
											u32 indexCount,
											u32 vertexCount,
											bool useDrawRequestsBuffer);
	bool	_AllocViewDependentBuffers(	const SRendererBatchDrawPass &drawPass,
										u32 particleCount,
										u32 indexCount,
										u32 vertexCount);
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
