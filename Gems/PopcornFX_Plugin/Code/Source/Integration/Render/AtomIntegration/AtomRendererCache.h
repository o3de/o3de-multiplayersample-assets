//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <pk_render_helpers/include/frame_collector/rh_particle_render_data_factory.h>

#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RHI/PipelineState.h>
#include <Atom/RPI.Public/Model/Model.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

// The EMaterialVariations flags change the actual shader variation loaded:
namespace	RendererFlags
{
    // /!\ ERendererFlags should match the flags defined in PopcornFX/Assets/shaders/Common/RendererSrg.azsli
	enum	ERendererFlags
	{
		// Basic color features:
		Has_Diffuse					= (1 << 0),
		Has_Emissive			    = (1 << 1),
		Has_Tint				    = (1 << 2),
		Has_Distortion			    = (1 << 3),
		// Lit particles:
		Has_Lighting			    = (1 << 4),
		// Soft particles:
		Has_Soft				    = (1 << 5),
		// Atlas and UVs:
		Has_Atlas				    = (1 << 6),
		Has_CorrectDeformation	    = (1 << 7),
		// Geometry:
		Has_Capsules			    = (1 << 8),
		Has_Size2					= (1 << 9),

		Has_NormalMap				= (1 << 10),
        Has_AnimBlend               = (1 << 11),
		Has_AnimBlend_Linear		= (1 << 12),
		Has_AnimBlend_MotionVectors	= (1 << 13),
		Has_DiffuseRamp				= (1 << 14),
		Has_EmissiveRamp			= (1 << 15),
		Has_AlphaRemap				= (1 << 16),

        HAS_RotateUV				= (1 << 17),
		HAS_FlipU					= (1 << 18),
		HAS_FlipV					= (1 << 19),

		HAS_Masked					= (1 << 20),

		HAS_Axis0					= (1 << 21),
		HAS_Axis1					= (1 << 22),
	};
}

// This is the blending mode used to render the particles (with the Transparent.Type renderer feature)
// If no Transparent.Type is specified, the blending will be BlendMode_Count
// and will change depending on the other rendering features enabled
namespace	BlendMode
{
	enum	EBlendMode
	{
		// Transparent
		Additive,
		AdditiveNoAlpha,
		AlphaBlend,
		PremultipliedAlpha,
		// Opaque:
		Solid,
		Masked,
		BlendMode_Count
	};
}

//----------------------------------------------------------------------------

// Select which shader to use:
enum	EPopcornFXShader
{
	Billboard_Shader,
	BillboardLegacy_Shader,
	BillboardLitLegacy_Shader,
	BillboardDistortion_Shader,
	RibbonLegacy_Shader,
	RibbonAtlasBlendLegacy_Shader,
	RibbonCorrectDeformationLegacy_Shader,
	RibbonLitLegacy_Shader,
	RibbonLitAtlasBlendLegacy_Shader,
	RibbonLitCorrectDeformationLegacy_Shader,
	RibbonDistortion_Shader,
	MeshLegacy_Shader,
	MeshLitLegacy_Shader,
	__Shader_Count
};

//----------------------------------------------------------------------------

extern const char	*kPopcornFXShaderPaths[__Shader_Count];

bool		IsLitShader(EPopcornFXShader shader);
bool		IsBillboardShader(EPopcornFXShader shader);
bool		IsRibbonShader(EPopcornFXShader shader);
bool		IsLegacyShader(EPopcornFXShader shader);
bool		IsMeshShader(EPopcornFXShader shader);
const char	*GetPopornFXUsedShaderPath(EPopcornFXShader shader);

//----------------------------------------------------------------------------

struct	SPipelineStateCacheKey
{
	// Which shader variation to use:
	u32							m_PipelineStateRendererFlags;
	// Blending mode to use:
	BlendMode::EBlendMode		m_BlendMode;
	// Shader to use:
	EPopcornFXShader			m_UsedShader;

	SPipelineStateCacheKey() : m_PipelineStateRendererFlags(0), m_BlendMode(BlendMode::BlendMode_Count), m_UsedShader(__Shader_Count) { }
	AZ::RPI::ShaderVariantId	GetShaderVariantId(const AZ::RPI::Shader &shader, bool precompiledOptions, bool depthOnly) const;
	bool						operator == (const SPipelineStateCacheKey &oth) const;
};

//----------------------------------------------------------------------------

class	CPipelineStateCache : public CRefCountedObject
{
public:
	// Actual resources:
	AZ::Data::Instance<AZ::RPI::Shader>			m_MaterialShader;
	AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	m_MaterialPipelineState;

	AZ::Data::Instance<AZ::RPI::Shader>			m_OpaqueDepthShader;
	AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	m_OpaqueDepthPipelineState;

	AZ::Data::Instance<AZ::RPI::Shader>			m_TransparentDepthMinShader;
	AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	m_TransparentDepthMinPipelineState;

	AZ::Data::Instance<AZ::RPI::Shader>			m_TransparentDepthMaxShader;
	AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	m_TransparentDepthMaxPipelineState;
	
	bool										m_Modified;
	Threads::CRWLock							m_Lock;

	CPipelineStateCache() : m_Modified(false) { }
};
PK_DECLARE_REFPTRCLASS(PipelineStateCache);

//----------------------------------------------------------------------------

struct	SMaterialCacheKey
{
	CStringId	m_DiffuseMapPath;
	CStringId	m_DiffuseRampMapPath;
	CStringId	m_EmissiveMapPath;
	CStringId	m_EmissiveRampMapPath;
	CStringId	m_NormalMapPath;
	CStringId	m_MotionVectorsMapPath;
	CStringId	m_AlphaMapPath;
	CStringId	m_DistortionMapPath;

	bool	operator == (const SMaterialCacheKey &oth) const;
};

//----------------------------------------------------------------------------

class	CMaterialCache : public CRefCountedObject
{
public:
	// Actual resources:
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_DiffuseMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_DiffuseRampMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_EmissiveMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_EmissiveRampMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_NormalMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_MotionVectorsMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_AlphaMap;
	AZ::Data::Instance<AZ::RPI::StreamingImage>		m_DistortionMap;

	bool											m_Modified;
	Threads::CRWLock								m_Lock;

	CMaterialCache() : m_Modified(false) { }
};
PK_DECLARE_REFPTRCLASS(MaterialCache);

//----------------------------------------------------------------------------

class	CGeometryCache : public CRefCountedObject
{
public:
	struct GPUBufferViews
	{
		AZ::RHI::IndexBufferView 			m_IndexBuffer;
		AZ::RHI::StreamBufferView			m_PositionBuffer;
		AZ::RHI::StreamBufferView			m_TangentBuffer;
		AZ::RHI::StreamBufferView			m_NormalBuffer;
		AZ::RHI::StreamBufferView			m_BitangentBuffer;
		AZ::RHI::StreamBufferView			m_UVBuffer;

		u32									m_IndexCount;
		CAABB								m_Bounds;

		GPUBufferViews()	: m_IndexCount(0)
							, m_Bounds(CAABB::DEGENERATED) {}
	};
	TArray<GPUBufferViews>						m_PerGeometryViews;
	CAABB										m_GlobalBounds;
	AZ::Data::Instance<AZ::RPI::ModelLod>		m_ModelLod;

	bool										m_Modified;
	Threads::CRWLock							m_Lock;
	
	CGeometryCache() : m_Modified(false) { }
};
PK_DECLARE_REFPTRCLASS(GeometryCache);

//----------------------------------------------------------------------------

class	CAtomRendererCache;
class	PopcornFXMeshLoader;

// This is a very basic description of a particle material that is created from a PopcornFX renderer data.
// You can create your own engine material instead of that.
struct	SParticleMaterialBasicDesc
{
	u32							m_RendererFlags;
	u32							m_RendererFlagsBatchMask;

	PopcornFXMeshLoader			*m_MeshLoader;
	CStringId					m_MeshPath;
	bool						m_UseMeshAtlas;

	CStringId					m_SoundStartTrigger;
	CStringId					m_SoundStopTrigger;
	CGuid						m_SoundControlFXParam;
	CStringId					m_SoundControlRTPC;

	bool						m_CastShadows;
	float						m_InvSoftnessDistance;
	float						m_InvNearPlaneFadeDistance;
	float						m_InvFarFadeDistance;
	float						m_Metalness;
	float						m_Roughness;

	float						m_MaskThreshold;

	float						m_SortDepthOffset;

    CFloat2                     m_MotionVectorsScale;

	SMaterialCacheKey			m_MaterialKey;
	SPipelineStateCacheKey		m_PipelineStateKey;

	// Methods:
	SParticleMaterialBasicDesc();
	~SParticleMaterialBasicDesc() {};

	void		InitFromRenderer(const CRendererDataBase &renderer, CAtomRendererCache *rendererCache);
	bool		operator == (const SParticleMaterialBasicDesc &oth) const;

	bool		HasAllRendererFlags(u32 flags) const;
	bool		HasOneRendererFlags(u32 flags) const;

private:
    // Adds renderer flags to m_RendererFlags
    // breaksBatching: if true, SParticleMaterialBasicDesc::operator== will take this flag into account
    // this means there will be a different CAtomBillboardingBatchPolicy for each variation of those flags (no batching)
    // changesPipelineState: if true, it means that those flags are used to choose a shader variation from the .shadervariationlist
	void		_AddRendererFlags(u32 flags, bool breaksBatching, bool changesPipelineState);
	void		_ResetRendererFlags();
};

//----------------------------------------------------------------------------

class CAtomRenderDataFactory;

class	CAtomRendererCache : public CRendererCacheBase
{
public:
	CAtomRendererCache();
	virtual ~CAtomRendererCache();

	virtual void						UpdateThread_BuildBillboardingFlags(const PRendererDataBase &renderer) override;

	void								InitFromRenderer(const CRendererDataBase &renderer, const CString &packPath, const CString &effectPath);
	bool								operator == (const CAtomRendererCache &oth) const;

	// Material description:
	ERendererClass						m_RendererType;
	SParticleMaterialBasicDesc			m_BasicDescription;
	CString								m_ParentEffectPath;
	CString								m_PackPath;
	CAtomRenderDataFactory			*m_CacheFactory;
};
PK_DECLARE_REFPTRCLASS(AtomRendererCache);

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
