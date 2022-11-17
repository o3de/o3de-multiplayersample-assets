//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "AtomPipelineCache.h"
#include "AtomRenderDataFactory.h"
#include <Atom/RHI/RHISystemInterface.h>

#if defined(O3DE_USE_PK)

namespace PopcornFX {
//----------------------------------------------------------------------------

CAtomPipelineCache::CAtomPipelineCache()
:	m_RendererSrg(null)
,	m_IsInitialized(false)
{
}

//----------------------------------------------------------------------------

CAtomPipelineCache::~CAtomPipelineCache()
{
}

//----------------------------------------------------------------------------

bool	CAtomPipelineCache::InitFromRendererCacheIFN(const CAtomRendererCache *rendererCache)
{
	PK_SCOPEDPROFILE();
	PPipelineStateCache		pipelineStateCache = rendererCache->m_CacheFactory->FindPipelineState(rendererCache->m_BasicDescription.m_PipelineStateKey);
	PMaterialCache			materialCache = rendererCache->m_CacheFactory->FindMaterial(rendererCache->m_BasicDescription.m_MaterialKey);

	if (!PK_VERIFY(pipelineStateCache != null && materialCache != null))
		return false;

	PK_SCOPEDLOCK_WRITE(materialCache->m_Lock);
	PK_SCOPEDLOCK_WRITE(pipelineStateCache->m_Lock);

	if (m_IsInitialized && !pipelineStateCache->m_Modified && !materialCache->m_Modified)
		return true;

	m_IsInitialized = false;
	pipelineStateCache->m_Modified = false;
	materialCache->m_Modified = false;

	if (pipelineStateCache->m_MaterialPipelineState == null || pipelineStateCache->m_MaterialShader == null)
		return true;

	// Only handle billboard renderers for now:
	if (rendererCache->m_RendererType != Renderer_Billboard && rendererCache->m_RendererType != Renderer_Ribbon && rendererCache->m_RendererType != Renderer_Mesh)
		return false;

	// We also need an array of indices to remap vertex inputs:
	_FillVertexInputsIndices(rendererCache); // Here only works for draw instance...

	m_RendererSrg = _CreateShaderResourceGroup(pipelineStateCache->m_MaterialShader, "RendererSrg");

	if (m_RendererSrg != null)
	{
		if (m_RendererSrg->HasShaderVariantKeyFallbackEntry())
		{
			AZ::RPI::ShaderVariantId	variantId = rendererCache->m_BasicDescription.m_PipelineStateKey.GetShaderVariantId(*pipelineStateCache->m_MaterialShader, false, false);
			m_RendererSrg->SetShaderVariantKeyFallbackValue(variantId.m_key);
		}
	}
	else
	{
		CLog::Log(PK_ERROR, "Could not get RendererSrg");
		return false;
	}

	if (rendererCache->m_RendererType == Renderer_Billboard)
	{
		_FillBillboardingSrgBindIndices(rendererCache);
	}
	else if (rendererCache->m_RendererType == Renderer_Ribbon)
	{
		_FillRibbonSrgBindIndices(rendererCache);
	}
	else if (rendererCache->m_RendererType == Renderer_Mesh)
	{
		_FillMeshSrgBindIndices(rendererCache);
	}

	// material:
	m_MaterialDrawList = pipelineStateCache->m_MaterialShader->GetDrawListTag();

	m_MaterialPipelineState = pipelineStateCache->m_MaterialPipelineState;
	if (pipelineStateCache->m_OpaqueDepthShader != null && pipelineStateCache->m_OpaqueDepthPipelineState != null)
	{
		m_MaterialDrawList = AZ::RHI::RHISystemInterface::Get()->GetDrawListTagRegistry()->AcquireTag(AZ::Name("forward"));
		// opaque depth pass:
		m_OpaqueDepthDrawList = pipelineStateCache->m_OpaqueDepthShader->GetDrawListTag();
		m_OpaqueDepthPipelineState = pipelineStateCache->m_OpaqueDepthPipelineState;
	}
	else if (	pipelineStateCache->m_TransparentDepthMinShader != null && pipelineStateCache->m_TransparentDepthMinPipelineState != null &&
				pipelineStateCache->m_TransparentDepthMaxShader != null && pipelineStateCache->m_TransparentDepthMaxPipelineState != null)
	{
		// transparent depth pass:
		m_TransparentDepthMinDrawList = pipelineStateCache->m_TransparentDepthMinShader->GetDrawListTag();
		m_TransparentDepthMinPipelineState = pipelineStateCache->m_TransparentDepthMinPipelineState;
		m_TransparentDepthMaxDrawList = pipelineStateCache->m_TransparentDepthMaxShader->GetDrawListTag();
		m_TransparentDepthMaxPipelineState = pipelineStateCache->m_TransparentDepthMaxPipelineState;
	}
	else if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Lighting))
	{
		CLog::Log(PK_ERROR, "Cannot have lit particles without depth render pass");
		return true;
	}

	// Get the material SRG and fill it with the renderer properties:
	m_MaterialSrg = _CreateShaderResourceGroup(pipelineStateCache->m_MaterialShader, "MaterialSrg");

	if (m_MaterialSrg != null)
		_FillMaterialSrgBindIndices(rendererCache);
	else
	{
		CLog::Log(PK_ERROR, "Could not get MaterialSrg");
		return false;
	}

	// Now we bind the associated textures:
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Diffuse))
	{
		if (materialCache->m_DiffuseMap == null)
			return true;
		SetMaterialSrgTexture(DiffuseMap_ShaderRead, materialCache->m_DiffuseMap->GetImageView());
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_DiffuseRamp))
	{
		if (materialCache->m_DiffuseRampMap == null)
			return true;
		SetMaterialSrgTexture(DiffuseRampMap_ShaderRead, materialCache->m_DiffuseRampMap->GetImageView());
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Emissive))
	{
		if (materialCache->m_EmissiveMap == null)
			return true;
		SetMaterialSrgTexture(EmissiveMap_ShaderRead, materialCache->m_EmissiveMap->GetImageView());
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_EmissiveRamp))
	{
		if (materialCache->m_EmissiveRampMap == null)
			return true;
		SetMaterialSrgTexture(EmissiveRampMap_ShaderRead, materialCache->m_EmissiveRampMap->GetImageView());
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_AnimBlend_MotionVectors))
	{
		if (materialCache->m_MotionVectorsMap == null)
			return true;
		SetMaterialSrgTexture(MotionVectorsMap_ShaderRead, materialCache->m_MotionVectorsMap->GetImageView());
		SetMaterialSrgConstantValue(MotionVectorsScale_ShaderRead, rendererCache->m_BasicDescription.m_MotionVectorsScale);
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_AlphaRemap))
	{
		if (materialCache->m_AlphaMap == null)
			return true;
		SetMaterialSrgTexture(AlphaMap_ShaderRead, materialCache->m_AlphaMap->GetImageView());
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Distortion))
	{
		if (materialCache->m_DistortionMap == null)
			return true;
		SetMaterialSrgTexture(DistortionMap_ShaderRead, materialCache->m_DistortionMap->GetImageView());
	}

	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Lighting))
	{
		if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_NormalMap))
		{
			if (materialCache->m_NormalMap == null)
				return true;
			SetMaterialSrgTexture(NormalMap_ShaderRead, materialCache->m_NormalMap->GetImageView());
		}
		SetMaterialSrgConstantValue(Roughness_ShaderRead, rendererCache->m_BasicDescription.m_Roughness);
		SetMaterialSrgConstantValue(Metalness_ShaderRead, rendererCache->m_BasicDescription.m_Metalness);
	}
	if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Soft))
	{
		SetMaterialSrgConstantValue(InvSoftnessDistance_ShaderRead, rendererCache->m_BasicDescription.m_InvSoftnessDistance);
	}
	if (rendererCache->m_BasicDescription.m_PipelineStateKey.m_BlendMode == BlendMode::Masked)
	{
		SetMaterialSrgConstantValue(MaskThreshold_ShaderRead, rendererCache->m_BasicDescription.m_MaskThreshold);
	}

	if (!m_MaterialSrg->IsQueuedForCompile())
		m_MaterialSrg->Compile();

	// Create the object SRG
	m_ObjectSrg = _CreateShaderResourceGroup(pipelineStateCache->m_MaterialShader, "ObjectSrg");

	m_IsInitialized = true;
	return true;
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::SetBillboardingSrgBuffer(BillboardSrg::EBuffersSemantic semantic, const AZ::RHI::Ptr<AZ::RHI::BufferView> bufferView)
{
	if (semantic >= BillboardSrg::__Max_BuffersSemantic || !m_BillboardingSrgBuffBindIdx[semantic].IsValid())
	{
		PK_ASSERT_NOT_REACHED_MESSAGE("Cannot add shader read input buffer");
		return;
	}
	m_RendererSrg->SetBufferView(m_BillboardingSrgBuffBindIdx[semantic], bufferView.get());
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::SetRibbonSrgBuffer(RibbonSrg::EBuffersSemantic semantic, const AZ::RHI::Ptr<AZ::RHI::BufferView> bufferView)
{
	if (semantic >= RibbonSrg::__Max_BuffersSemantic || !m_RibbonSrgBuffBindIdx[semantic].IsValid())
	{
		PK_ASSERT_NOT_REACHED_MESSAGE("Cannot add shader read input buffer");
		return;
	}
	m_RendererSrg->SetBufferView(m_RibbonSrgBuffBindIdx[semantic], bufferView.get());
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::SetMeshSrgBuffer(MeshSrg::EBuffersSemantic semantic, const AZ::RHI::Ptr<AZ::RHI::BufferView> bufferView)
{
	if (semantic >= MeshSrg::__Max_BuffersSemantic || !m_MeshSrgBuffBindIdx[semantic].IsValid())
	{
		PK_ASSERT_NOT_REACHED_MESSAGE("Cannot add shader read input buffer");
		return;
	}
	m_RendererSrg->SetBufferView(m_MeshSrgBuffBindIdx[semantic], bufferView.get());
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::SetMaterialSrgTexture(EMaterialSrgTexturesSemantic semantic, const AZ::RHI::ImageView *imageView)
{
	if (semantic >= __Max_MaterialSrgTexturesSemantic || !m_MaterialSrgTextureBindIdx[semantic].IsValid())
	{
		PK_ASSERT_NOT_REACHED_MESSAGE("Cannot add shader read input texture");
		return;
	}
	m_MaterialSrg->SetImageView(m_MaterialSrgTextureBindIdx[semantic], imageView);
}	

//----------------------------------------------------------------------------

void	CAtomPipelineCache::SetVertexInputBuffer(EVertexInputSemantic semantic, const AZ::RHI::StreamBufferView &streamBufferView)
{
	if (semantic >= __Max_VertexInputSemantic || !m_VertexInputsIdx[semantic].Valid())
	{
		PK_ASSERT_NOT_REACHED_MESSAGE("Cannot add shader vertex input buffer");
		return;
	}
	CGuid	vtxBindIdx = m_VertexInputsIdx[semantic];

	if (m_VertexInputs.Count() <= vtxBindIdx)
		m_VertexInputs.Resize(vtxBindIdx + 1);
	m_VertexInputs[vtxBindIdx] = streamBufferView;
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::SetIndexBuffer(const AZ::RHI::IndexBufferView &idxBufferView)
{
	m_IndexBuffer = idxBufferView;
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::ConfigureDrawCall(SAtomRenderContext::SDrawCall &drawCall)
{
	if (!m_RendererSrg->IsQueuedForCompile())
		m_RendererSrg->Compile();

	// Setup the draw call data:
	drawCall.m_RendererSrg = m_RendererSrg;
	drawCall.m_MaterialSrg = m_MaterialSrg;
	drawCall.m_ObjectSrg = m_ObjectSrg;
	drawCall.m_VertexInputs = m_VertexInputs;
	drawCall.m_Indices = m_IndexBuffer;

	drawCall.m_MaterialPipelineState = m_MaterialPipelineState;
	drawCall.m_MaterialDrawList = m_MaterialDrawList;

	drawCall.m_OpaqueDepthPipelineState = m_OpaqueDepthPipelineState;
	drawCall.m_OpaqueDepthDrawList = m_OpaqueDepthDrawList;

	drawCall.m_TransparentDepthMinPipelineState = m_TransparentDepthMinPipelineState;
	drawCall.m_TransparentDepthMinDrawList = m_TransparentDepthMinDrawList;
	drawCall.m_TransparentDepthMaxPipelineState = m_TransparentDepthMaxPipelineState;
	drawCall.m_TransparentDepthMaxDrawList = m_TransparentDepthMaxDrawList;

	// Reset vertex and index buffers:
	m_VertexInputs.Clear();
	m_IndexBuffer = AZ::RHI::IndexBufferView();
}

//----------------------------------------------------------------------------

AZ::Data::Instance<AZ::RPI::ShaderResourceGroup>	CAtomPipelineCache::_CreateShaderResourceGroup(AZ::Data::Instance<AZ::RPI::Shader> shader, const char *srgName)
{
	AZ::Data::Asset<AZ::RPI::ShaderAsset> shaderAsset = shader->GetAsset();
	AZ::RPI::SupervariantIndex supervariantIndex = shader->GetSupervariantIndex();
	AZ::RHI::Ptr<AZ::RHI::ShaderResourceGroupLayout> perObjectSrgLayout = shader->FindShaderResourceGroupLayout(AZ::Name{ srgName });

	auto perInstanceSrgAsset = shader->FindShaderResourceGroupLayout(AZ::Name{ srgName });
	if (!perInstanceSrgAsset)
	{
		CLog::Log(PK_ERROR, "Could not find shader resource group asset '%s'", srgName);
		return null;
	}

	auto srg = AZ::RPI::ShaderResourceGroup::Create(shaderAsset, supervariantIndex, perObjectSrgLayout->GetName());

	if (!srg)
	{
		CLog::Log(PK_ERROR, "Failed to create shader resource group");
		return null;
	}

	return srg;
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::_FillMaterialSrgBindIndices(const CAtomRendererCache *rendererCache)
{
	AZ_UNUSED(rendererCache);

	AZ::Name	textureNames[__Max_MaterialSrgTexturesSemantic] =
	{
		AZ::Name("m_diffuseMap"),
		AZ::Name("m_diffuseRampMap"),
		AZ::Name("m_emissiveMap"),
		AZ::Name("m_emissiveRampMap"),
		AZ::Name("m_normalMap"),
		AZ::Name("m_motionVectorsMap"),
		AZ::Name("m_alphaMap"),
		AZ::Name("m_distortionMap"),
	};

	AZ::Name	constantNames[__Max_MaterialSrgConstantsSemantic] =
	{
		AZ::Name("m_roughness"),
		AZ::Name("m_metalness"),
		AZ::Name("m_motionVectorsScale"),
		AZ::Name("m_invSoftnessDistance"),
		AZ::Name("m_maskThreshold"),
	};

	for (u32 i = 0; i < __Max_MaterialSrgTexturesSemantic; ++i)
	{
		m_MaterialSrgTextureBindIdx[i] = m_MaterialSrg->FindShaderInputImageIndex(textureNames[i]);
		if (!PK_VERIFY(m_MaterialSrgTextureBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in MaterialSrg", textureNames[i].GetCStr());
	}
	for (u32 i = 0; i < __Max_MaterialSrgConstantsSemantic; ++i)
	{
		m_MaterialSrgConstBindIdx[i] = m_MaterialSrg->FindShaderInputConstantIndex(constantNames[i]);
		if (!PK_VERIFY(m_MaterialSrgConstBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in MaterialSrg", constantNames[i].GetCStr());
	}
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::_FillBillboardingSrgBindIndices(const CAtomRendererCache *rendererCache)
{
	AZ_UNUSED(rendererCache);

	AZ::Name	bufferNames[BillboardSrg::__Max_BuffersSemantic] =
	{
		AZ::Name("m_indices"),
		AZ::Name("m_positions"),
		AZ::Name("m_rotations"),
		AZ::Name("m_axis0"),
		AZ::Name("m_axis1"),
		AZ::Name("m_sizes"),
		AZ::Name("m_sizes2"),
		AZ::Name("m_diffuseColors"),
		AZ::Name("m_emissiveColors"),
		AZ::Name("m_distortionColors"),
		AZ::Name("m_alphaCursors"),
		AZ::Name("m_atlasIDs"),
		AZ::Name("m_atlasSubRects"),
		AZ::Name("m_drawRequests"),
	};

	AZ::Name	constantNames[BillboardSrg::__Max_ConstantsSemantic] =
	{
		AZ::Name("m_rendererFlags"),
		AZ::Name("m_atlasSubRectsCount"),
	};

	// Reflection on the shader constant buffers:
	for (u32 i = 0; i < BillboardSrg::__Max_BuffersSemantic; ++i)
	{
		m_BillboardingSrgBuffBindIdx[i] = m_RendererSrg->FindShaderInputBufferIndex(bufferNames[i]);
		if (!PK_VERIFY(m_BillboardingSrgBuffBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in BillboardingSrg", bufferNames[i].GetCStr());
	}
	// Reflection on the shader constant values:
	for (u32 i = 0; i < BillboardSrg::__Max_ConstantsSemantic; ++i)
	{
		m_BillboardingSrgConstBindIdx[i] = m_RendererSrg->FindShaderInputConstantIndex(constantNames[i]);
		if (!PK_VERIFY(m_BillboardingSrgConstBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in BillboardingSrg", constantNames[i].GetCStr());
	}
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::_FillRibbonSrgBindIndices(const CAtomRendererCache *rendererCache)
{
	AZ_UNUSED(rendererCache);

	AZ::Name	bufferNames[RibbonSrg::__Max_BuffersSemantic] =
	{
		AZ::Name("m_diffuseColors"),
		AZ::Name("m_emissiveColors"),
		AZ::Name("m_distortionColors"),
		AZ::Name("m_alphaCursors"),
	};

	AZ::Name	constantNames[RibbonSrg::__Max_ConstantsSemantic] =
	{
		AZ::Name("m_rendererFlags"),
		AZ::Name("m_particleCount"),
	};

	// Reflection on the shader constant buffers:
	for (u32 i = 0; i < RibbonSrg::__Max_BuffersSemantic; ++i)
	{
		m_RibbonSrgBuffBindIdx[i] = m_RendererSrg->FindShaderInputBufferIndex(bufferNames[i]);
		if (!PK_VERIFY(m_RibbonSrgBuffBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in RibbonSrg", bufferNames[i].GetCStr());
	}
	// Reflection on the shader constant values:
	for (u32 i = 0; i < RibbonSrg::__Max_ConstantsSemantic; ++i)
	{
		m_RibbonSrgConstBindIdx[i] = m_RendererSrg->FindShaderInputConstantIndex(constantNames[i]);
		if (!PK_VERIFY(m_RibbonSrgConstBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in RibbonSrg", constantNames[i].GetCStr());
	}
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::_FillMeshSrgBindIndices(const CAtomRendererCache *rendererCache)
{
	AZ_UNUSED(rendererCache);

	const AZ::Name	bufferNames[MeshSrg::__Max_BuffersSemantic] =
	{
		AZ::Name("m_matrices"),
		AZ::Name("m_diffuseColors"),
		AZ::Name("m_emissiveColors"),
		AZ::Name("m_alphaCursors")
	};

	const AZ::Name	constantNames[MeshSrg::__Max_ConstantsSemantic] =
	{
		AZ::Name("m_rendererFlags")
	};

	// Reflection on the shader constant buffers:
	for (u32 i = 0; i < MeshSrg::__Max_BuffersSemantic; ++i)
	{
		m_MeshSrgBuffBindIdx[i] = m_RendererSrg->FindShaderInputBufferIndex(bufferNames[i]);

		if (!PK_VERIFY(m_MeshSrgBuffBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in MeshSrg", bufferNames[i].GetCStr());
	}
	// Reflection on the shader constant values:	
	for (u32 i = 0; i < MeshSrg::__Max_ConstantsSemantic; ++i)
	{
		m_MeshSrgConstBindIdx[i] = m_RendererSrg->FindShaderInputConstantIndex(constantNames[i]);

		if (!PK_VERIFY(m_MeshSrgConstBindIdx[i].IsValid()))
			CLog::Log(PK_ERROR, "Could not find '%s' in MeshSrg", constantNames[i].GetCStr());
	}
}

//----------------------------------------------------------------------------

void	CAtomPipelineCache::_FillVertexInputsIndices(const CAtomRendererCache *rendererCache)
{
	for (u32 i = 0; i < __Max_VertexInputSemantic; ++i)
		m_VertexInputsIdx[i].Clear();

	// Cannot use reflection here, so we just manually set the shader vertex inputs:
	if (rendererCache->m_RendererType == Renderer_Billboard)
	{
		// For vertex draw instance:
		m_VertexInputsIdx[DrawInstanceTexCoords_Vertex] = 0; // Only one vertex input which is the texCoord...
	}
	if (rendererCache->m_RendererType == Renderer_Ribbon)
	{
		u32	vtxIdx = 0;
		// For vertex draw instance:
		m_VertexInputsIdx[Position_Vertex] = vtxIdx++;
		m_VertexInputsIdx[TexCoord0_Vertex] = vtxIdx++;

		if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_AnimBlend))
		{
			m_VertexInputsIdx[TexCoord1_Vertex] = vtxIdx++;
			m_VertexInputsIdx[TexFrameLerp_Vertex] = vtxIdx++;
		}
		else if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_CorrectDeformation))
		{
			m_VertexInputsIdx[UVRemap_Vertex] = vtxIdx++;
			m_VertexInputsIdx[UVFactors_Vertex] = vtxIdx++;
		}
		if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Lighting))
		{
			m_VertexInputsIdx[Normal_Vertex] = vtxIdx++;
			m_VertexInputsIdx[Tangent_Vertex] = vtxIdx++;
		}
	}
	if (rendererCache->m_RendererType == Renderer_Mesh)
	{
		u32	vtxIdx = 0;
		m_VertexInputsIdx[Position_Vertex] = vtxIdx++;
		m_VertexInputsIdx[TexCoord0_Vertex] = vtxIdx++;
		if (rendererCache->m_BasicDescription.HasOneRendererFlags(RendererFlags::Has_Lighting))
		{
			m_VertexInputsIdx[Normal_Vertex] = vtxIdx++;
			m_VertexInputsIdx[Tangent_Vertex] = vtxIdx++;
		}
	}
}

//----------------------------------------------------------------------------
}

#endif
