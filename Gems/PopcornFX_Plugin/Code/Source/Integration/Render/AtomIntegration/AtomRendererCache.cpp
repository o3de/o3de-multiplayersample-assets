//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "AtomRendererCache.h"

#if defined(O3DE_USE_PK)

#include "Integration/PopcornFXIntegrationBus.h"

#include "Integration/Render/BasicEditorRendererFeatures.h"

#include <pk_render_helpers/include/frame_collector/rh_particle_render_data_factory.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

const char	*kPopcornFXShaderPaths[] =
{
	"shaders/Billboard/Default/Billboard.azshader",							// Billboard_Shader
	"shaders/Billboard/Legacy/Billboard_Legacy.azshader",					// BillboardLegacy_Shader
	"shaders/Billboard/Legacy/BillboardLit_Legacy.azshader",				// BillboardLitLegacy_Shader
	"shaders/Billboard/Default/BillboardDistortion.azshader",				// BillboardDistortion_Shader
	"shaders/Ribbon/Legacy/Ribbon_Legacy.azshader",							// RibbonLegacy_Shader
	"shaders/Ribbon/Legacy/RibbonAtlasBlend_Legacy.azshader",				// RibbonAtlasBlendLegacy_Shader
	"shaders/Ribbon/Legacy/RibbonCorrectDeformation_Legacy.azshader",		// RibbonCorrectDeformationLegacy_Shader
	"shaders/Ribbon/Legacy/RibbonLit_Legacy.azshader",						// RibbonLitLegacy_Shader
	"shaders/Ribbon/Legacy/RibbonLitAtlasBlend_Legacy.azshader",			// RibbonLitAtlasBlendLegacy_Shader
	"shaders/Ribbon/Legacy/RibbonLitCorrectDeformation_Legacy.azshader",	// RibbonLitCorrectDeformationLegacy_Shader
	"shaders/Ribbon/Default/RibbonDistortion.azshader",						// RibbonDistortion_Shader
	"shaders/Mesh/Legacy/Mesh_Legacy.azshader",								// Mesh_Shader
	"shaders/Mesh/Legacy/MeshLit_Legacy.azshader",							// MeshLit_Shader
};

//----------------------------------------------------------------------------

bool IsLitShader(EPopcornFXShader shader)
{
	return	shader == BillboardLitLegacy_Shader ||
			shader == RibbonLitLegacy_Shader ||
			shader == RibbonLitAtlasBlendLegacy_Shader ||
			shader == RibbonLitCorrectDeformationLegacy_Shader ||
			shader == MeshLitLegacy_Shader;
}

//----------------------------------------------------------------------------

bool IsBillboardShader(EPopcornFXShader shader)
{
	return	shader == BillboardLitLegacy_Shader ||
			shader == BillboardLegacy_Shader ||
			shader == Billboard_Shader ||
			shader == BillboardDistortion_Shader;
}

//----------------------------------------------------------------------------

bool IsRibbonShader(EPopcornFXShader shader)
{
	return	shader == RibbonLegacy_Shader ||
			shader == RibbonAtlasBlendLegacy_Shader ||
			shader == RibbonCorrectDeformationLegacy_Shader ||
			shader == RibbonLitLegacy_Shader ||
			shader == RibbonLitAtlasBlendLegacy_Shader ||
			shader == RibbonLitCorrectDeformationLegacy_Shader ||
			shader == RibbonDistortion_Shader;
}

//----------------------------------------------------------------------------

bool IsLegacyShader(EPopcornFXShader shader)
{
	return	shader == BillboardLegacy_Shader ||
			shader == BillboardLitLegacy_Shader ||
			shader == RibbonLegacy_Shader ||
			shader == RibbonAtlasBlendLegacy_Shader ||
			shader == RibbonCorrectDeformationLegacy_Shader ||
			shader == RibbonLitLegacy_Shader ||
			shader == RibbonLitAtlasBlendLegacy_Shader ||
			shader == RibbonLitCorrectDeformationLegacy_Shader ||
			shader == BillboardDistortion_Shader ||
			shader == RibbonDistortion_Shader ||
			shader == MeshLegacy_Shader;
}

//----------------------------------------------------------------------------

bool IsMeshShader(EPopcornFXShader shader)
{
	return	shader == MeshLegacy_Shader ||
			shader == MeshLitLegacy_Shader;
}
//----------------------------------------------------------------------------

const char	*GetPopornFXUsedShaderPath(EPopcornFXShader shader)
{
	if (shader >= EPopcornFXShader::__Shader_Count)
		return null;
	return kPopcornFXShaderPaths[(u32)shader];
}

//----------------------------------------------------------------------------

AZ::RPI::ShaderVariantId	SPipelineStateCacheKey::GetShaderVariantId(const AZ::RPI::Shader &shader, bool precompiledOptions, bool depthOnly) const
{
	static const AZ::Name valueTrue = AZ::Name("true");
	static const AZ::Name valueFalse = AZ::Name("false");
	AZ::RPI::ShaderOptionGroup shaderOptions = shader.CreateShaderOptionGroup();

	const bool hasCapsules = (m_PipelineStateRendererFlags & RendererFlags::Has_Capsules) != 0;
	const bool hasAtlas = (m_PipelineStateRendererFlags & RendererFlags::Has_Atlas) != 0;
	const bool hasDiffuse = (m_PipelineStateRendererFlags & RendererFlags::Has_Diffuse) != 0;
	const bool hasEmissive = (m_PipelineStateRendererFlags & RendererFlags::Has_Emissive) != 0;

	if (IsBillboardShader(m_UsedShader) && depthOnly)
	{
		shaderOptions.SetValue(AZ::Name("o_hasBillboardCapsules"), hasCapsules ? valueTrue : valueFalse);
		return shaderOptions.GetShaderVariantId();
	}

	// Blendmode shader options. The actual pipeline blend mode is set in PopcornFXRendererLoader.cpp.
	// This must match AlphaUtils.azli option
	switch (m_BlendMode)
	{
	case BlendMode::Additive:
	case BlendMode::AdditiveNoAlpha:
	case BlendMode::AlphaBlend:
	case BlendMode::PremultipliedAlpha:
		shaderOptions.SetValue(AZ::Name("o_opacity_mode"), AZ::Name("OpacityMode::Blended"));
		break;
	case BlendMode::Solid:
		shaderOptions.SetValue(AZ::Name("o_opacity_mode"), AZ::Name("OpacityMode::Opaque"));
	case BlendMode::Masked:
		shaderOptions.SetValue(AZ::Name("o_opacity_mode"), AZ::Name("OpacityMode::Cutout"));
		break;
	default:
		break;
	}

	if (IsBillboardShader(m_UsedShader))
	{
		shaderOptions.SetValue(AZ::Name("o_hasAtlas"), hasAtlas ? valueTrue : valueFalse);
		shaderOptions.SetValue(AZ::Name("o_hasBillboardCapsules"), hasCapsules ? valueTrue : valueFalse);
	}
	if (IsLitShader(m_UsedShader) && !precompiledOptions)
	{
		shaderOptions.SetValue(AZ::Name("o_enablePunctualLights"), valueTrue);
		shaderOptions.SetValue(AZ::Name("o_enableAreaLights"), valueTrue);
		shaderOptions.SetValue(AZ::Name("o_enableDirectionalLights"), valueTrue);
		shaderOptions.SetValue(AZ::Name("o_enableIBL"), valueTrue);
		shaderOptions.SetValue(AZ::Name("o_enableShadows"), valueTrue);

		static const AZ::Name none = AZ::Name("TransmissionMode::None");
		static const AZ::Name thinObj = AZ::Name("TransmissionMode::ThinObject");
		static const AZ::Name thickObj = AZ::Name("TransmissionMode::ThickObject");
		shaderOptions.SetValue(AZ::Name("o_transmission_mode"), none);
	}
	if (!IsLegacyShader(m_UsedShader))
	{
		shaderOptions.SetValue(AZ::Name("o_hasDiffuse"), hasDiffuse ? valueTrue : valueFalse);
		shaderOptions.SetValue(AZ::Name("o_hasEmissive"), hasEmissive ? valueTrue : valueFalse);
	}

	return shaderOptions.GetShaderVariantId();
}

//----------------------------------------------------------------------------

bool	SPipelineStateCacheKey::operator == (const SPipelineStateCacheKey &oth) const
{
	return	m_PipelineStateRendererFlags == oth.m_PipelineStateRendererFlags &&
			m_BlendMode == oth.m_BlendMode &&
			m_UsedShader == oth.m_UsedShader;
}

//----------------------------------------------------------------------------

bool	SMaterialCacheKey::operator == (const SMaterialCacheKey &oth) const
{
	return	m_DiffuseMapPath == oth.m_DiffuseMapPath &&
			m_DiffuseRampMapPath == oth.m_DiffuseRampMapPath &&
			m_EmissiveMapPath == oth.m_EmissiveMapPath &&
			m_NormalMapPath == oth.m_NormalMapPath &&
			m_AlphaMapPath == oth.m_AlphaMapPath &&
			m_DistortionMapPath == oth.m_DistortionMapPath;
}

//----------------------------------------------------------------------------
//
//	SParticleMaterialBasicDesc
//
//----------------------------------------------------------------------------

SParticleMaterialBasicDesc::SParticleMaterialBasicDesc()
:	m_RendererFlags(0)
,	m_RendererFlagsBatchMask(0)
,	m_UseMeshAtlas(false)
,	m_SoundControlFXParam(CGuid::INVALID)
,	m_CastShadows(false)
,	m_InvSoftnessDistance(0)
,	m_InvNearPlaneFadeDistance(0)
,	m_InvFarFadeDistance(0)
,	m_Metalness(0)
,	m_Roughness(0)
,	m_MaskThreshold(0)
,	m_SortDepthOffset(0)
,	m_MotionVectorsScale(CFloat2::ZERO)
{
}

//----------------------------------------------------------------------------

void	SParticleMaterialBasicDesc::InitFromRenderer(const CRendererDataBase &renderer, CAtomRendererCache *rendererCache)
{
	PK_ASSERT(	renderer.m_RendererType == Renderer_Billboard ||
				renderer.m_RendererType == Renderer_Ribbon ||
				renderer.m_RendererType == Renderer_Mesh ||
				renderer.m_RendererType == Renderer_Light ||
				renderer.m_RendererType == Renderer_Sound ||
				renderer.m_RendererType == Renderer_Decal);

	//-----------------------------
	// Choose the material variation:
	//-----------------------------
	const SRendererFeaturePropertyValue	*bbMode = renderer.m_Declaration.FindProperty(BasicFeatures::SID_GeometryBillboard_BillboardingMode());
	const SRendererFeaturePropertyValue	*atlas = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Atlas());
	const SRendererFeaturePropertyValue	*billboardSize2 = renderer.m_Declaration.FindProperty(BasicFeatures::SID_GeometryBillboard_EnableSize2D());
	// Diffuse feature:
	const SRendererFeaturePropertyValue	*diffuse = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Diffuse());
	const SRendererFeaturePropertyValue	*diffuseMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Diffuse_DiffuseMap());
	const SRendererFeaturePropertyValue	*diffuseRamp = renderer.m_Declaration.FindProperty(BasicFeatures::SID_DiffuseRamp());
	const SRendererFeaturePropertyValue	*diffuseRampMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_DiffuseRamp_RampMap());
	CGuid								diffuseColorInput = renderer.m_Declaration.FindAdditionalFieldIndex(BasicFeatures::SID_Diffuse_Color());
	// Emissive feature:
	const SRendererFeaturePropertyValue	*emissive = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Emissive());
	const SRendererFeaturePropertyValue	*emissiveMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Emissive_EmissiveMap());
	const SRendererFeaturePropertyValue	*emissiveRamp = renderer.m_Declaration.FindProperty(BasicFeatures::SID_EmissiveRamp());
	const SRendererFeaturePropertyValue	*emissiveRampMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_EmissiveRamp_RampMap());
	CGuid								emissiveColorInput = renderer.m_Declaration.FindAdditionalFieldIndex(BasicFeatures::SID_Emissive_EmissiveColor());
	// Tint feature:
	// const SRendererFeaturePropertyValue	*tint = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Tint());
	// const SRendererFeaturePropertyValue	*tintMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Tint_TintMap());
	// CGuid								tintColorInput = renderer.m_Declaration.FindAdditionalFieldIndex(BasicFeatures::SID_Tint_TintMap());

	// Distortion feature:
	const SRendererFeaturePropertyValue	*distortion = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Distortion());
	const SRendererFeaturePropertyValue	*distortionMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Distortion_DistortionMap());
	CGuid								distortionColorInput = renderer.m_Declaration.FindAdditionalFieldIndex(BasicFeatures::SID_Distortion_Color());

	const SRendererFeaturePropertyValue	*linearAtlasBlending = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Atlas_Blending());
	const SRendererFeaturePropertyValue	*motionVectorsMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Atlas_MotionVectorsMap());
	const SRendererFeaturePropertyValue	*motionVectorsScale = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Atlas_DistortionStrength());
	const SRendererFeaturePropertyValue	*alphaRemap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_AlphaRemap());
	const SRendererFeaturePropertyValue	*alphaRemapAlphaMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_AlphaRemap_AlphaMap());
	CGuid								alphaRemapCursor = renderer.m_Declaration.FindAdditionalFieldIndex(BasicFeatures::SID_AlphaRemap_Cursor());
	const SRendererFeaturePropertyValue	*lit = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Lit());
	const SRendererFeaturePropertyValue	*normalMap = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Lit_NormalMap());

	const SRendererFeaturePropertyValue	*litRoughness = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Lit_Roughness());
	const SRendererFeaturePropertyValue	*litMetalness = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Lit_Metalness());
	const SRendererFeaturePropertyValue	*softParticles = renderer.m_Declaration.FindProperty(BasicFeatures::SID_SoftParticles());
	const SRendererFeaturePropertyValue	*softnessDistance = renderer.m_Declaration.FindProperty(BasicFeatures::SID_SoftParticles_SoftnessDistance());

	// For ribbons only:
	const SRendererFeaturePropertyValue	*correctDeformation = renderer.m_Declaration.FindProperty(BasicFeatures::SID_CorrectDeformation());
	// For meshes only:
	const SRendererFeaturePropertyValue	*meshPath = renderer.m_Declaration.FindProperty(BasicFeatures::SID_GeometryMesh_Mesh());
	const SRendererFeaturePropertyValue	*meshAtlas = renderer.m_Declaration.FindProperty(BasicFeatures::SID_MeshAtlas());

	CString	packPath = rendererCache->m_PackPath;
	if (packPath == ".")
		packPath.Clear();

	_ResetRendererFlags();

	// Has_Capsules:
	if (renderer.m_RendererType == Renderer_Billboard)
	{
		// GPU billboarding shader variations: we set changesPipelineState to true
		if (atlas != null && atlas->ValueB())
			_AddRendererFlags(RendererFlags::Has_Atlas, true, true);
		if (bbMode != null)
		{
			EBillboardMode	mode = Drawers::SBillboard_BillboardingRequest::BillboardProperty_BillboardMode_ToInternal(bbMode->ValueI().x());
			if (mode == BillboardMode_AxisAlignedCapsule)
				_AddRendererFlags(RendererFlags::Has_Capsules, true, true);
			else if (mode == BillboardMode_AxisAligned || mode == BillboardMode_AxisAlignedSpheroid)
				_AddRendererFlags(RendererFlags::HAS_Axis0, true, false);
			else if (mode == BillboardMode_PlaneAligned)
				_AddRendererFlags(RendererFlags::HAS_Axis1, true, false);
		}
		if (billboardSize2 != null && billboardSize2->ValueB())
		{
			_AddRendererFlags(RendererFlags::Has_Size2, false, false);
		}
	}

	// Has_Diffuse:
	if (diffuse != null && diffuse->ValueB() &&
		diffuseMap != null && !diffuseMap->ValuePath().Empty() &&
		diffuseColorInput.Valid())
	{
		// With the new "Experimental" shaders, emissive, diffuse, distortion and tint are features you can disable
		// to handle that we set changesPipelineState to true:
		_AddRendererFlags(RendererFlags::Has_Diffuse, true, true);
		m_MaterialKey.m_DiffuseMapPath = CStringId(packPath / diffuseMap->ValuePath());
		if (diffuseRamp != null && diffuseRamp->ValueB() && diffuseRampMap != null && !diffuseRampMap->ValuePath().Empty())
		{
			_AddRendererFlags(RendererFlags::Has_DiffuseRamp, true, false);
			m_MaterialKey.m_DiffuseRampMapPath = CStringId(packPath / diffuseRampMap->ValuePath());
		}
	}

	// Has_Emissive:
	if (emissive != null && emissive->ValueB() &&
		emissiveMap != null && !emissiveMap->ValuePath().Empty() &&
		emissiveColorInput.Valid())
	{
		// With the new "Experimental" shaders, emissive, diffuse, distortion and tint are features you can disable
		// to handle that we set changesPipelineState to true:
		_AddRendererFlags(RendererFlags::Has_Emissive, true, true);
		m_MaterialKey.m_EmissiveMapPath = CStringId(packPath / emissiveMap->ValuePath());
		if (emissiveRamp != null && emissiveRamp->ValueB() && emissiveRampMap != null && !emissiveRampMap->ValuePath().Empty())
		{
			_AddRendererFlags(RendererFlags::Has_EmissiveRamp, true, false);
			m_MaterialKey.m_EmissiveRampMapPath = CStringId(packPath / emissiveRampMap->ValuePath());
		}
	}

	// Has_Tint:

	// Has_AtlasBlending:
	if (linearAtlasBlending != null)
	{
		BasicFeatures::AtlasBlending::EAtlasBlending	atlasBlending = static_cast<BasicFeatures::AtlasBlending::EAtlasBlending>(linearAtlasBlending->ValueI().x());

		// For billboards, the atlas blending uses the same shader (changesPipelineState set to false):
		if (atlasBlending != BasicFeatures::AtlasBlending::None)
		{
			const bool	changesPipelineState = renderer.m_RendererType != ERendererClass::Renderer_Billboard;
			_AddRendererFlags(RendererFlags::Has_AnimBlend, true, changesPipelineState);
		}
		if (atlasBlending == BasicFeatures::AtlasBlending::Linear)
			_AddRendererFlags(RendererFlags::Has_AnimBlend_Linear, true, false);
		else if (atlasBlending == BasicFeatures::AtlasBlending::MotionVectors && motionVectorsMap != null && !motionVectorsMap->ValuePath().Empty())
		{
			_AddRendererFlags(RendererFlags::Has_AnimBlend_MotionVectors, true, false);
			m_MaterialKey.m_MotionVectorsMapPath = CStringId(packPath / motionVectorsMap->ValuePath());
			if (motionVectorsScale != null)
				m_MotionVectorsScale = motionVectorsScale->ValueF().xy();
			else
				m_MotionVectorsScale = CFloat2::ONE;
		}
	}
	// Has_CorrectDeformation:
	if (correctDeformation != null && correctDeformation->ValueB())
	{
		// We use a different shader for correct deformation, so changesPipelineState is set to true:
		_AddRendererFlags(RendererFlags::Has_CorrectDeformation, true, true);
	}

	// Has_AlphaRemap:
	if (alphaRemap != null && alphaRemap->ValueB() && alphaRemapAlphaMap != null && !alphaRemapAlphaMap->ValuePath().Empty() && alphaRemapCursor.Valid())
	{
		_AddRendererFlags(RendererFlags::Has_AlphaRemap, true, false);
		m_MaterialKey.m_AlphaMapPath = CStringId(packPath / alphaRemapAlphaMap->ValuePath());
	}
	if (lit != null && lit->ValueB())
	{
		_AddRendererFlags(RendererFlags::Has_Lighting, true, true);
		if (litRoughness != null)
			m_Roughness = litRoughness->ValueF().x();
		if (litMetalness != null)
			m_Metalness = litMetalness->ValueF().x();
		if (normalMap != null && !normalMap->ValuePath().Empty())
		{
			_AddRendererFlags(RendererFlags::Has_NormalMap, true, false);
			m_MaterialKey.m_NormalMapPath = CStringId(packPath / normalMap->ValuePath());
		}
	}
	if (softParticles != null && softParticles->ValueB() && softnessDistance != null && softnessDistance->ValueF().x() != 0.0f)
	{
		_AddRendererFlags(RendererFlags::Has_Soft, true, true);
		m_InvSoftnessDistance = 1.0f / softnessDistance->ValueF().x(); 
	}

	// Has_Distortion:
	if (distortion != null && distortion->ValueB() && distortionMap != null && !distortionMap->ValuePath().Empty() && distortionColorInput.Valid())
	{
		_AddRendererFlags(RendererFlags::Has_Distortion, true, true);
		m_MaterialKey.m_DistortionMapPath = CStringId(packPath / distortionMap->ValuePath());
	}

	//-----------------------------
	// Get the UV generation flags:
	//-----------------------------
	const SRendererFeaturePropertyValue	*texUV = renderer.m_Declaration.FindProperty(BasicFeatures::SID_TextureUVs());
	const SRendererFeaturePropertyValue	*flipU = renderer.m_Declaration.FindProperty(BasicFeatures::SID_TextureUVs_FlipU());
	const SRendererFeaturePropertyValue	*flipV = renderer.m_Declaration.FindProperty(BasicFeatures::SID_TextureUVs_FlipV());
	const SRendererFeaturePropertyValue	*flipUV = renderer.m_Declaration.FindProperty(BasicFeatures::SID_FlipUVs());
	const SRendererFeaturePropertyValue	*rotateTexture = renderer.m_Declaration.FindProperty(BasicFeatures::SID_TextureUVs_RotateTexture());

	const bool	isTexUVActivated = texUV != null ? texUV->ValueB() : false;
	if (isTexUVActivated)
	{
		u32		genUVFlags = 0;
		PK_ASSERT(renderer.m_RendererType == Renderer_Ribbon);
		genUVFlags |= (flipU != null && flipU->ValueB()) ? RendererFlags::HAS_FlipU : 0;
		genUVFlags |= (flipV != null && flipV->ValueB()) ? RendererFlags::HAS_FlipV : 0;
		genUVFlags |= (rotateTexture != null && rotateTexture->ValueB()) ? RendererFlags::HAS_RotateUV : 0;
		_AddRendererFlags(genUVFlags, true, false);
	}

	const bool	isUVFlipped = flipUV != null ? flipUV->ValueB() : false;
	if (isUVFlipped)
	{
		u32		genUVFlags = 0;
		PK_ASSERT(renderer.m_RendererType == Renderer_Billboard);
		genUVFlags |= RendererFlags::HAS_FlipU;
		genUVFlags |= RendererFlags::HAS_FlipV;
		_AddRendererFlags(genUVFlags, false, false);
	}

	if (meshPath != null && !meshPath->ValuePath().Empty())
	{
		m_UseMeshAtlas = (meshAtlas != null && meshAtlas->ValueB());
		m_MeshPath = CStringId(packPath / meshPath->ValuePath());
	}

	//-----------------------------
	// Choose the blending mode:
	//-----------------------------
	const SRendererFeaturePropertyValue	*transparent = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Transparent());
	const SRendererFeaturePropertyValue	*transparentType = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Transparent_Type());
	const SRendererFeaturePropertyValue	*transparentCameraSortOffset = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Transparent_CameraSortOffset());
	const SRendererFeaturePropertyValue	*opaque = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Opaque());
	const SRendererFeaturePropertyValue	*opaqueType = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Opaque_Type());
	const SRendererFeaturePropertyValue	*opaqueMaskThreshold = renderer.m_Declaration.FindProperty(BasicFeatures::SID_Opaque_MaskThreshold());

	if (transparent != null && transparent->ValueB() && transparentType != null)
	{
		if (transparentType->ValueI().x() == BasicFeatures::TransparentType::Additive)
		{
			m_PipelineStateKey.m_BlendMode = BlendMode::Additive;
		}
		else if (transparentType->ValueI().x() == BasicFeatures::TransparentType::AdditiveNoAlpha)
		{
			m_PipelineStateKey.m_BlendMode = BlendMode::AdditiveNoAlpha;
		}
		else if (transparentType->ValueI().x() == BasicFeatures::TransparentType::AlphaBlend)
		{
			m_PipelineStateKey.m_BlendMode = BlendMode::AlphaBlend;
		}
		else if (transparentType->ValueI().x() == BasicFeatures::TransparentType::PremultipliedAlpha)
		{
			m_PipelineStateKey.m_BlendMode = BlendMode::PremultipliedAlpha;
		}

		if (transparentCameraSortOffset != null)
			m_SortDepthOffset += transparentCameraSortOffset->ValueF().x();
	}
	else if (opaque != null && opaque->ValueB() && opaqueType != null)
	{
		if (opaqueType->ValueI().x() == BasicFeatures::OpaqueType::Solid)
		{
			m_PipelineStateKey.m_BlendMode = BlendMode::Solid;
		}
		else if (opaqueType->ValueI().x() == BasicFeatures::OpaqueType::Masked)
		{
			m_PipelineStateKey.m_BlendMode = BlendMode::Masked;
			_AddRendererFlags(RendererFlags::HAS_Masked, true, false);
			if (opaqueMaskThreshold != null)
				m_MaskThreshold = opaqueMaskThreshold->ValueF().x();
		}
	}

	// Find the associated shader:
	if (distortion != null && distortion->ValueB())
	{
		if (renderer.m_RendererType == ERendererClass::Renderer_Billboard)
			m_PipelineStateKey.m_UsedShader = EPopcornFXShader::BillboardDistortion_Shader;
		else if (renderer.m_RendererType == ERendererClass::Renderer_Ribbon)
			m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonDistortion_Shader;

		m_PipelineStateKey.m_BlendMode = BlendMode::Additive;
	}
	else if (m_PipelineStateKey.m_BlendMode == BlendMode::BlendMode_Count) // No explicit blend mode, can use new materials...
	{
		if (renderer.m_RendererType == ERendererClass::Renderer_Billboard)
			m_PipelineStateKey.m_UsedShader = EPopcornFXShader::Billboard_Shader;
	}
	else
	{
		if (renderer.m_RendererType == ERendererClass::Renderer_Billboard)
		{
			if (HasOneRendererFlags(RendererFlags::Has_Lighting))
				m_PipelineStateKey.m_UsedShader = EPopcornFXShader::BillboardLitLegacy_Shader;
			else
				m_PipelineStateKey.m_UsedShader = EPopcornFXShader::BillboardLegacy_Shader;
		}
		else if (renderer.m_RendererType == ERendererClass::Renderer_Ribbon)
		{
			if (HasOneRendererFlags(RendererFlags::Has_Lighting))
			{
				if (HasOneRendererFlags(RendererFlags::Has_CorrectDeformation))
					m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonLitCorrectDeformationLegacy_Shader;
				else if (HasOneRendererFlags(RendererFlags::Has_AnimBlend))
					m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonLitAtlasBlendLegacy_Shader;
				else
					m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonLitLegacy_Shader;
			}
			else
			{
				if (HasOneRendererFlags(RendererFlags::Has_CorrectDeformation))
					m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonCorrectDeformationLegacy_Shader;
				else if (HasOneRendererFlags(RendererFlags::Has_AnimBlend))
					m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonAtlasBlendLegacy_Shader;
				else
					m_PipelineStateKey.m_UsedShader = EPopcornFXShader::RibbonLegacy_Shader;
			}
		}
		else if (renderer.m_RendererType == ERendererClass::Renderer_Mesh)
		{
			if (HasOneRendererFlags(RendererFlags::Has_Lighting))
				m_PipelineStateKey.m_UsedShader = EPopcornFXShader::MeshLitLegacy_Shader;
			else
				m_PipelineStateKey.m_UsedShader = EPopcornFXShader::MeshLegacy_Shader;
		}
	}

	PK_ASSERT(m_PipelineStateKey.m_UsedShader != EPopcornFXShader::__Shader_Count);
}

//----------------------------------------------------------------------------

bool	SParticleMaterialBasicDesc::operator == (const SParticleMaterialBasicDesc &oth) const
{
	return	(m_RendererFlags & m_RendererFlagsBatchMask) == (oth.m_RendererFlags & oth.m_RendererFlagsBatchMask) &&
			m_MeshPath == oth.m_MeshPath &&
			m_UseMeshAtlas == oth.m_UseMeshAtlas &&
			m_SoundStartTrigger == oth.m_SoundStartTrigger &&
			m_SoundStopTrigger == oth.m_SoundStopTrigger &&
			m_SoundControlFXParam == oth.m_SoundControlFXParam &&
			m_SoundControlRTPC == oth.m_SoundControlRTPC &&
			m_CastShadows == oth.m_CastShadows &&
			m_InvSoftnessDistance == oth.m_InvSoftnessDistance &&
			m_InvNearPlaneFadeDistance == oth.m_InvNearPlaneFadeDistance &&
			m_InvFarFadeDistance == oth.m_InvFarFadeDistance &&
			m_Metalness == oth.m_Metalness &&
			m_Roughness == oth.m_Roughness &&
			m_MaskThreshold == oth.m_MaskThreshold &&
			m_SortDepthOffset == oth.m_SortDepthOffset &&
			m_MotionVectorsScale == oth.m_MotionVectorsScale &&
			m_MaterialKey == oth.m_MaterialKey &&
			m_PipelineStateKey == oth.m_PipelineStateKey;
}

//----------------------------------------------------------------------------

bool	SParticleMaterialBasicDesc::HasAllRendererFlags(u32 flags) const
{
	return (m_RendererFlags & flags) == flags;
}

//----------------------------------------------------------------------------

bool	SParticleMaterialBasicDesc::HasOneRendererFlags(u32 flags) const
{
	return (m_RendererFlags & flags) != 0;
}

//----------------------------------------------------------------------------

void	SParticleMaterialBasicDesc::_AddRendererFlags(u32 flags, bool breaksBatching, bool changesPipelineState)
{
	m_RendererFlags |= flags;
	if (breaksBatching)
		m_RendererFlagsBatchMask |= flags;
	if (changesPipelineState)
		m_PipelineStateKey.m_PipelineStateRendererFlags |= flags;
}

void	SParticleMaterialBasicDesc::_ResetRendererFlags()
{
	m_RendererFlags = 0;
	m_RendererFlagsBatchMask = 0;
	m_PipelineStateKey.m_PipelineStateRendererFlags = 0;
}

//----------------------------------------------------------------------------
//
//	CAtomRendererCache
//
//----------------------------------------------------------------------------

CAtomRendererCache::CAtomRendererCache()
:	m_RendererType(Renderer_Invalid)
,	m_CacheFactory(null)
{
}

//----------------------------------------------------------------------------

CAtomRendererCache::~CAtomRendererCache()
{
}

//----------------------------------------------------------------------------

void	CAtomRendererCache::UpdateThread_BuildBillboardingFlags(const PRendererDataBase &renderer)
{
	AZ_UNUSED(renderer);

	// Here we define which geometry we need to render this material.
	// The m_Flags is used to tell PopcornFX what it needs to generate:

	// Enables the sorting of the index buffer per view:
	m_Flags.m_NeedSort = m_BasicDescription.m_PipelineStateKey.m_BlendMode == BlendMode::AlphaBlend || m_BasicDescription.m_PipelineStateKey.m_BlendMode == BlendMode::PremultipliedAlpha;
	m_Flags.m_Slicable = m_BasicDescription.m_PipelineStateKey.m_BlendMode != BlendMode::Masked && m_BasicDescription.m_PipelineStateKey.m_BlendMode != BlendMode::Solid;
	// Enables generating the UVs (and how):
	m_Flags.m_HasUV =	m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Diffuse | RendererFlags::Has_Emissive | RendererFlags::Has_Distortion);
	m_Flags.m_FlipU = m_BasicDescription.HasOneRendererFlags(RendererFlags::HAS_FlipU);
	m_Flags.m_FlipV = m_BasicDescription.HasOneRendererFlags(RendererFlags::HAS_FlipV);
	m_Flags.m_RotateTexture = m_BasicDescription.HasOneRendererFlags(RendererFlags::HAS_RotateUV);
	// Enables generating a second set of UV to be able to lerp between animation frames:
	m_Flags.m_HasAtlasBlending = m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_AnimBlend_Linear | RendererFlags::Has_AnimBlend_MotionVectors);
	// Enables generating some factors used to remap the ribbons UVs and get them to look smooth:
	m_Flags.m_HasRibbonCorrectDeformation = !m_Flags.m_HasAtlasBlending && m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_CorrectDeformation);
	// Enables generating the normals and the tangents of the particles:
	m_Flags.m_HasTangent = m_Flags.m_HasNormal = m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Lighting);
}

//----------------------------------------------------------------------------

void	CAtomRendererCache::InitFromRenderer(const CRendererDataBase &renderer, const CString &packPath, const CString &effectPath)
{
	m_RendererType = renderer.m_RendererType;
	m_PackPath = packPath;
	m_ParentEffectPath = effectPath;
	m_BasicDescription.InitFromRenderer(renderer, this);
}

//----------------------------------------------------------------------------

bool	CAtomRendererCache::operator == (const CAtomRendererCache &oth) const
{
	return	m_RendererType == oth.m_RendererType &&
			m_BasicDescription == oth.m_BasicDescription;
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
