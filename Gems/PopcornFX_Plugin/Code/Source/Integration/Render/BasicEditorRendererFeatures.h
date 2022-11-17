//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

// This file was generated from the PopcornFX editor:

#pragma once

#include <pk_kernel/include/kr_string_id.h>

__PK_API_BEGIN

namespace BasicFeatures
{
	// Feature EnableRendering:
	extern CStringId	SID_EnableRendering(); // bool - feature
	// General.Enabled: Controls if the renderer is enabled or disabled.
	// If 'true', the particle will be drawn, if 'false', it won't.
	extern CStringId	SID_EnableRendering_Enabled(); // bool - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature GeometryBillboard:
	extern CStringId	SID_GeometryBillboard(); // bool - feature
	// General.Position: 
	extern CStringId	SID_GeometryBillboard_Position(); // float3 - link (per-particle)
	// General.EnableSize2D: 
	extern CStringId	SID_GeometryBillboard_EnableSize2D(); // bool - property (per-renderer)
	// General.Size: 
	extern CStringId	SID_GeometryBillboard_Size(); // float - link (per-particle)
	// General.Size2: 
	extern CStringId	SID_GeometryBillboard_Size2(); // float2 - link (per-particle)
	// General.Rotation: 
	extern CStringId	SID_GeometryBillboard_Rotation(); // float - link (per-particle)
	// General.Axis: 
	extern CStringId	SID_GeometryBillboard_Axis(); // float3 - link (per-particle)
	// General.NormalAxis: 
	extern CStringId	SID_GeometryBillboard_NormalAxis(); // float3 - link (per-particle)
	// General.BillboardingMode: 
	namespace GeometryBillboardBillboardingMode
	{
		enum EGeometryBillboardBillboardingMode
		{
			ScreenAligned,
			ViewposAligned,
			AxisAligned,
			AxisAlignedSpheroid,
			AxisAlignedCapsule,
			PlaneAligned,
		};
	}
	extern CStringId	SID_GeometryBillboard_BillboardingMode(); // int (EGeometryBillboardBillboardingMode) - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature GeometryRibbon:
	extern CStringId	SID_GeometryRibbon(); // bool - feature
	// General.Position: 
	extern CStringId	SID_GeometryRibbon_Position(); // float3 - link (per-particle)
	// General.Size: 
	extern CStringId	SID_GeometryRibbon_Size(); // float - link (per-particle)
	// General.Axis: 
	extern CStringId	SID_GeometryRibbon_Axis(); // float3 - link (per-particle)
	// General.BillboardingMode: 
	namespace GeometryRibbonBillboardingMode
	{
		enum EGeometryRibbonBillboardingMode
		{
			ViewposAligned,
			NormalAxisAligned,
			SideAxisAligned,
		};
	}
	extern CStringId	SID_GeometryRibbon_BillboardingMode(); // int (EGeometryRibbonBillboardingMode) - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature GeometryMesh:
	extern CStringId	SID_GeometryMesh(); // bool - feature
	// General.Position: 
	extern CStringId	SID_GeometryMesh_Position(); // float3 - link (per-particle)
	// General.Scale: 
	extern CStringId	SID_GeometryMesh_Scale(); // float3 - link (per-particle)
	// General.Orientation: 
	extern CStringId	SID_GeometryMesh_Orientation(); // orientation - link (per-particle)
	// General.Mesh: Mesh resource
	extern CStringId	SID_GeometryMesh_Mesh(); // dataGeometry - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature GeometryLight:
	extern CStringId	SID_GeometryLight(); // bool - feature
	// General.Position: 
	extern CStringId	SID_GeometryLight_Position(); // float3 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature GeometrySound:
	extern CStringId	SID_GeometrySound(); // bool - feature
	// General.Position: 
	extern CStringId	SID_GeometrySound_Position(); // float3 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature GeometryTriangle:
	extern CStringId	SID_GeometryTriangle(); // bool - feature
	// General.Position1: 
	extern CStringId	SID_GeometryTriangle_Position1(); // float3 - link (per-particle)
	// General.Position2: 
	extern CStringId	SID_GeometryTriangle_Position2(); // float3 - link (per-particle)
	// General.Position3: 
	extern CStringId	SID_GeometryTriangle_Position3(); // float3 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature GeometryDecal:
	extern CStringId	SID_GeometryDecal(); // bool - feature
	// General.Position: 
	extern CStringId	SID_GeometryDecal_Position(); // float3 - link (per-particle)
	// General.Scale: 
	extern CStringId	SID_GeometryDecal_Scale(); // float3 - link (per-particle)
	// General.Orientation: 
	extern CStringId	SID_GeometryDecal_Orientation(); // orientation - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature Transparent:
	extern CStringId	SID_Transparent(); // bool - feature
	// Transparent.Type: Transparent blending mode
	namespace TransparentType
	{
		enum ETransparentType
		{
			Additive,
			AdditiveNoAlpha,
			AlphaBlend,
			PremultipliedAlpha,
		};
	}
	extern CStringId	SID_Transparent_Type(); // int (ETransparentType) - property (per-renderer)
	// Transparent.SortMode: 
	namespace TransparentSortMode
	{
		enum ETransparentSortMode
		{
			CameraDistance,
			ByCustomValue,
		};
	}
	extern CStringId	SID_Transparent_SortMode(); // int (ETransparentSortMode) - property (per-renderer)
	// Transparent.SortKey: 
	extern CStringId	SID_Transparent_SortKey(); // float - link (per-particle)
	// Transparent.GlobalSortOverride: Global sort order of the draw calls.
	// Different values for 2 renderers will separate their draw call.
	// Smaller values are drawn first (visually behind higher values). The value can be negative.
	extern CStringId	SID_Transparent_GlobalSortOverride(); // int - property (per-renderer)
	// Transparent.CameraSortOffset: The depth bias allows to offset the draw call bbox along the camera axis (in meters).
	// Different values for 2 renderers will separate their drawcall.
	extern CStringId	SID_Transparent_CameraSortOffset(); // float - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature Opaque:
	extern CStringId	SID_Opaque(); // bool - feature
	// Opaque.Type: Opaque mode
	namespace OpaqueType
	{
		enum EOpaqueType
		{
			Solid,
			Masked,
		};
	}
	extern CStringId	SID_Opaque_Type(); // int (EOpaqueType) - property (per-renderer)
	// Opaque.MaskThreshold: Opacity mask threshold value.
	// If alpha value is below this value, the pixel is discarded.
	extern CStringId	SID_Opaque_MaskThreshold(); // float - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature Diffuse:
	extern CStringId	SID_Diffuse(); // bool - feature
	// Diffuse.DiffuseMap: Source map for diffuse lighting component
	extern CStringId	SID_Diffuse_DiffuseMap(); // dataImage - property (per-renderer)
	// Diffuse.Color: 
	extern CStringId	SID_Diffuse_Color(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature SoftParticles:
	extern CStringId	SID_SoftParticles(); // bool - feature
	// SoftParticles.SoftnessDistance: Fade distance of particles in world units
	extern CStringId	SID_SoftParticles_SoftnessDistance(); // float - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature CastShadows:
	extern CStringId	SID_CastShadows(); // bool - feature
	//----------------------------------------------------------------------------
	// Feature Distortion:
	extern CStringId	SID_Distortion(); // bool - feature
	// Distortion.DistortionMap: Source map for distortion
	extern CStringId	SID_Distortion_DistortionMap(); // dataImage - property (per-renderer)
	// Distortion.Color: 
	extern CStringId	SID_Distortion_Color(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature CorrectDeformation:
	extern CStringId	SID_CorrectDeformation(); // bool - feature
	//----------------------------------------------------------------------------
	// Feature AlphaRemap:
	extern CStringId	SID_AlphaRemap(); // bool - feature
	// AlphaRemap.AlphaMap: Source map for alpha remap
	extern CStringId	SID_AlphaRemap_AlphaMap(); // dataImage - property (per-renderer)
	// AlphaRemap.Cursor: 
	extern CStringId	SID_AlphaRemap_Cursor(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature Atlas:
	extern CStringId	SID_Atlas(); // bool - feature
	// Atlas.Source: Atlas source type
	namespace AtlasSource
	{
		enum EAtlasSource
		{
			External,
			Procedural,
		};
	}
	extern CStringId	SID_Atlas_Source(); // int (EAtlasSource) - property (per-renderer)
	// Atlas.Definition: Atlas definition (.pkat)
	extern CStringId	SID_Atlas_Definition(); // dataImageAtlas - property (per-renderer)
	// Atlas.SubDiv: Subdivision count
	extern CStringId	SID_Atlas_SubDiv(); // int2 - property (per-renderer)
	// Atlas.Blending: Frame blending type
	namespace AtlasBlending
	{
		enum EAtlasBlending
		{
			None,
			Linear,
			MotionVectors,
		};
	}
	extern CStringId	SID_Atlas_Blending(); // int (EAtlasBlending) - property (per-renderer)
	// Atlas.MotionVectorsMap: 
	extern CStringId	SID_Atlas_MotionVectorsMap(); // dataImage - property (per-renderer)
	// Atlas.DistortionStrength: Multiplier over the uv distortion
	// Distortion strength to apply for Houdini exported maps is: -(1.0 / FPS / ColRow).
	extern CStringId	SID_Atlas_DistortionStrength(); // float2 - property (per-renderer)
	// Atlas.TextureID: Index of the sub-rect to use inside the atlas
	extern CStringId	SID_Atlas_TextureID(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature Light:
	extern CStringId	SID_Light(); // bool - feature
	// Light.Color: 
	extern CStringId	SID_Light_Color(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature LightAttenuation:
	extern CStringId	SID_LightAttenuation(); // bool - feature
	// LightAttenuation.AttenuationSteepness: Controls how fast the light intensity falls off from the center of the light
	extern CStringId	SID_LightAttenuation_AttenuationSteepness(); // float - property (per-renderer)
	// LightAttenuation.Range: 
	extern CStringId	SID_LightAttenuation_Range(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature LightTranslucent:
	extern CStringId	SID_LightTranslucent(); // bool - feature
	//----------------------------------------------------------------------------
	// Feature Sound:
	extern CStringId	SID_Sound(); // bool - feature
	// Sound.SoundData: Path to the sound to be played
	extern CStringId	SID_Sound_SoundData(); // dataAudio - property (per-renderer)
	// Sound.Volume: 
	extern CStringId	SID_Sound_Volume(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature SoundAttenuation:
	extern CStringId	SID_SoundAttenuation(); // bool - feature
	// SoundAttenuation.AttenuationModel: Controls how fast the sound intensity falls off from the source
	namespace SoundAttenuationAttenuationModel
	{
		enum ESoundAttenuationAttenuationModel
		{
			Linear,
			InverseLinear,
		};
	}
	extern CStringId	SID_SoundAttenuation_AttenuationModel(); // int (ESoundAttenuationAttenuationModel) - property (per-renderer)
	// SoundAttenuation.Range: 
	extern CStringId	SID_SoundAttenuation_Range(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature Doppler:
	extern CStringId	SID_Doppler(); // bool - feature
	// Doppler.DopplerFactor: Controls the strength of the doppler effect
	extern CStringId	SID_Doppler_DopplerFactor(); // float - property (per-renderer)
	// Doppler.Velocity: 
	extern CStringId	SID_Doppler_Velocity(); // float3 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature CustomTextureU:
	extern CStringId	SID_CustomTextureU(); // bool - feature
	// CustomTextureU.TextureU: 
	extern CStringId	SID_CustomTextureU_TextureU(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature TextureUVs:
	extern CStringId	SID_TextureUVs(); // bool - feature
	// TextureUVs.FlipU: Flips the texture UVs horizontally
	extern CStringId	SID_TextureUVs_FlipU(); // bool - property (per-renderer)
	// TextureUVs.FlipV: Flips the texture UVs vertically
	extern CStringId	SID_TextureUVs_FlipV(); // bool - property (per-renderer)
	// TextureUVs.RotateTexture: Rotates texcoords 90 degrees clockwise (TextureU, FlipU, and FlipV are rotated too)
	extern CStringId	SID_TextureUVs_RotateTexture(); // bool - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature FlipUVs:
	extern CStringId	SID_FlipUVs(); // bool - feature
	//----------------------------------------------------------------------------
	// Feature TextureRepeat:
	extern CStringId	SID_TextureRepeat(); // bool - feature
	//----------------------------------------------------------------------------
	// Feature ShaderInput0:
	extern CStringId	SID_ShaderInput0(); // bool - feature
	// ShaderInput0.Input0: Data sent to the shader, remaps to DynamicParameter0 in UE4
	extern CStringId	SID_ShaderInput0_Input0(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature ShaderInput1:
	extern CStringId	SID_ShaderInput1(); // bool - feature
	// ShaderInput1.Input1: Data sent to the shader, remaps to DynamicParameter1 in UE4
	extern CStringId	SID_ShaderInput1_Input1(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature ShaderInput2:
	extern CStringId	SID_ShaderInput2(); // bool - feature
	// ShaderInput2.Input2: Data sent to the shader, remaps to DynamicParameter2 in UE4
	extern CStringId	SID_ShaderInput2_Input2(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature ShaderInput3:
	extern CStringId	SID_ShaderInput3(); // bool - feature
	// ShaderInput3.Input3: Data sent to the shader, remaps to DynamicParameter3 in UE4
	extern CStringId	SID_ShaderInput3_Input3(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature SphereLight:
	extern CStringId	SID_SphereLight(); // bool - feature
	// AreaLight.SphereRadius: Controls the radius of the light sphere
	extern CStringId	SID_SphereLight_SphereRadius(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature Lit:
	extern CStringId	SID_Lit(); // bool - feature
	// Lit.NormalMap: Source map used to compute normals for lighting at render
	extern CStringId	SID_Lit_NormalMap(); // dataImage - property (per-renderer)
	// Lit.CastShadows: Can cast shadows
	extern CStringId	SID_Lit_CastShadows(); // bool - property (per-renderer)
	// Lit.Roughness: Roughness for the material.
	extern CStringId	SID_Lit_Roughness(); // float - property (per-renderer)
	// Lit.Metalness: Metalness for the material.
	extern CStringId	SID_Lit_Metalness(); // float - property (per-renderer)
	// Lit.RoughMetalMap: Roughness (R) and Metalness (G) packed in a RGBA texture
	extern CStringId	SID_Lit_RoughMetalMap(); // dataImage - property (per-renderer)
	// Lit.Type: 
	namespace LitType
	{
		enum ELitType
		{
			Solid,
			Transparent,
		};
	}
	extern CStringId	SID_Lit_Type(); // int (ELitType) - property (per-renderer)
	// Lit.LitMaskMap: 
	extern CStringId	SID_Lit_LitMaskMap(); // dataImage - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature LegacyLit:
	extern CStringId	SID_LegacyLit(); // bool - feature
	// LegacyLit.NormalMap: Source map used to compute normals used for lighting
	extern CStringId	SID_LegacyLit_NormalMap(); // dataImage - property (per-renderer)
	// LegacyLit.NormalWrapFactor: Remap the 'n dot l' of the lighting equation from [-1,1] to [-1+factor,1]
	extern CStringId	SID_LegacyLit_NormalWrapFactor(); // float - property (per-renderer)
	// LegacyLit.LightExponent: Exponentiation of the lighting term
	extern CStringId	SID_LegacyLit_LightExponent(); // float - property (per-renderer)
	// LegacyLit.LightScale: Multiplier applied to the incoming light
	extern CStringId	SID_LegacyLit_LightScale(); // float3 - property (per-renderer)
	// LegacyLit.AmbientLight: Additional uniform light
	extern CStringId	SID_LegacyLit_AmbientLight(); // float3 - property (per-renderer)
	// LegacyLit.CastShadows: Can cast shadows
	extern CStringId	SID_LegacyLit_CastShadows(); // bool - property (per-renderer)
	// LegacyLit.SpecularMap: Source map used in specular lighting computation
	extern CStringId	SID_LegacyLit_SpecularMap(); // dataImage - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature NormalWrap:
	extern CStringId	SID_NormalWrap(); // bool - feature
	// NormalWrap.WrapFactor: Fake sub-surface scattering on the particles by remapping the N.L in the lighting equation
	extern CStringId	SID_NormalWrap_WrapFactor(); // float - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature NormalBend:
	extern CStringId	SID_NormalBend(); // bool - feature
	// NormalBend.NormalBendingFactor: Factor to bend normals generated at billboard. 0 means no bending.
	extern CStringId	SID_NormalBend_NormalBendingFactor(); // float - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature MeshAtlas:
	extern CStringId	SID_MeshAtlas(); // bool - feature
	// General.MeshID: Index of mesh selected to render these particles
	extern CStringId	SID_MeshAtlas_MeshID(); // float - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature DiffuseRamp:
	extern CStringId	SID_DiffuseRamp(); // bool - feature
	// DiffuseRamp.RampMap: Source map for diffuse remap
	extern CStringId	SID_DiffuseRamp_RampMap(); // dataImage - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature Emissive:
	extern CStringId	SID_Emissive(); // bool - feature
	// Emissive.EmissiveMap: Source map for diffuse lighting component
	extern CStringId	SID_Emissive_EmissiveMap(); // dataImage - property (per-renderer)
	// Emissive.EmissiveColor: 
	extern CStringId	SID_Emissive_EmissiveColor(); // float3 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature EmissiveRamp:
	extern CStringId	SID_EmissiveRamp(); // bool - feature
	// EmissiveRamp.RampMap: Source map for emissive remap
	extern CStringId	SID_EmissiveRamp_RampMap(); // dataImage - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature TransformUVs:
	extern CStringId	SID_TransformUVs(); // bool - feature
	// TransformUVs.UVOffset: 
	extern CStringId	SID_TransformUVs_UVOffset(); // float2 - link (per-particle)
	// TransformUVs.UVRotate: 
	extern CStringId	SID_TransformUVs_UVRotate(); // float - link (per-particle)
	// TransformUVs.UVScale: 
	extern CStringId	SID_TransformUVs_UVScale(); // float2 - link (per-particle)
	// TransformUVs.RGBOnly: 
	extern CStringId	SID_TransformUVs_RGBOnly(); // bool - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature Decal:
	extern CStringId	SID_Decal(); // bool - feature
	// Decal.FadeTop: 
	extern CStringId	SID_Decal_FadeTop(); // float - property (per-renderer)
	// Decal.FadeBottom: 
	extern CStringId	SID_Decal_FadeBottom(); // float - property (per-renderer)
	// Decal.FadeAngle: 
	extern CStringId	SID_Decal_FadeAngle(); // float - property (per-renderer)
	//----------------------------------------------------------------------------
	// Feature TriangleCustomNormals:
	extern CStringId	SID_TriangleCustomNormals(); // bool - feature
	// TriangleCustomNormals.Normal1: 
	extern CStringId	SID_TriangleCustomNormals_Normal1(); // float3 - link (per-particle)
	// TriangleCustomNormals.Normal2: 
	extern CStringId	SID_TriangleCustomNormals_Normal2(); // float3 - link (per-particle)
	// TriangleCustomNormals.Normal3: 
	extern CStringId	SID_TriangleCustomNormals_Normal3(); // float3 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature TriangleCustomUVs:
	extern CStringId	SID_TriangleCustomUVs(); // bool - feature
	// TriangleCustomUVs.UV1: 
	extern CStringId	SID_TriangleCustomUVs_UV1(); // float2 - link (per-particle)
	// TriangleCustomUVs.UV2: 
	extern CStringId	SID_TriangleCustomUVs_UV2(); // float2 - link (per-particle)
	// TriangleCustomUVs.UV3: 
	extern CStringId	SID_TriangleCustomUVs_UV3(); // float2 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature Tint:
	extern CStringId	SID_Tint(); // bool - feature
	// Tint.TintMap: 
	extern CStringId	SID_Tint_TintMap(); // dataImage - property (per-renderer)
	// Tint.Color: 
	extern CStringId	SID_Tint_Color(); // float4 - link (per-particle)
	//----------------------------------------------------------------------------
	// Feature UseVertexColors:
	extern CStringId	SID_UseVertexColors(); // bool - feature
	//----------------------------------------------------------------------------

	// Setup renderer interface properties:
	void	Startup();
	void	Shutdown();
}

__PK_API_END
