//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXRendererLoader.h"

#if defined(O3DE_USE_PK)

#include "PopcornFXSystemComponent.h"

#include <AzFramework/Asset/AssetSystemBus.h>
#include <pk_kernel/include/kr_threads_basics.h>
#include <Atom/RHI.Reflect/InputStreamLayoutBuilder.h>
#include <Atom/RPI.Public/RPISystemInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Model/Model.h>

namespace PopcornFX {
//----------------------------------------------------------------------------

PopcornFXRendererLoader::PopcornFXRendererLoader()
{
}

//----------------------------------------------------------------------------

PopcornFXRendererLoader::~PopcornFXRendererLoader()
{
	AZ::Data::AssetBus::MultiHandler::BusDisconnect();
	AZ::RPI::ShaderReloadNotificationBus::MultiHandler::BusDisconnect();
}

//----------------------------------------------------------------------------

bool	PopcornFXRendererLoader::AddMaterialToCreate(const SMaterialCacheKey &materialCacheKey, const SPipelineStateCacheKey &pipelineCacheKey)
{
	PK_SCOPEDLOCK(m_Lock);
	// Find or create the caches:
	bool					materialCacheExist = false;
	PMaterialCache			materialCache = _GetOrCreateMaterialCache(materialCacheKey, materialCacheExist);
	bool					pipelineStateCacheExist = false;
	PPipelineStateCache		pipelineStateCache = _GetOrCreatePipelineCache(pipelineCacheKey, pipelineStateCacheExist);

	if (!PK_VERIFY(materialCache != null && pipelineStateCache != null))
		return false;

	SAssetDependencies::SCaches		currentCaches;
	currentCaches.m_MaterialKey = materialCacheKey;
	currentCaches.m_PipelineStateKey = pipelineCacheKey;

	// Load the material resources:
	if (!materialCacheExist)
	{
		// Load the textures:
		if (materialCacheKey.m_DiffuseMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_DiffuseMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_DiffuseMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_DiffuseRampMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_DiffuseRampMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_DiffuseRampMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_EmissiveMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_EmissiveMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_EmissiveMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_EmissiveRampMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_EmissiveRampMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_EmissiveRampMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_NormalMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_NormalMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_NormalMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_MotionVectorsMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_MotionVectorsMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_MotionVectorsMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_AlphaMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_AlphaMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_AlphaMapPath.ToStringData(), currentCaches)))
				return false;
		}
		if (materialCacheKey.m_DistortionMapPath.Valid())
		{
			currentCaches.m_Type = AssetType_DistortionMap;
			if (!PK_VERIFY(_AddTextureToLoad(materialCacheKey.m_DistortionMapPath.ToStringData(), currentCaches)))
				return false;
		}
	}

	// Load the shader variation:
	if (!pipelineStateCacheExist)
	{
		// Actual shader:
		const char	*shaderPathPtr = GetPopornFXUsedShaderPath(pipelineCacheKey.m_UsedShader);
		currentCaches.m_Type = AssetType_MaterialShader;
		if (!PK_VERIFY(_AddShaderToLoad(shaderPathPtr, currentCaches)))
			return false;

		// For opaque particles:
		const char	*opaqueDepthPass = "shaders/Depth/Basic/DepthPass.azshader";
		// For transparent particles:
		const char	*minDepthPass = "shaders/Depth/Basic/DepthPassTransparentMin.azshader";
		const char	*maxDepthPass = "shaders/Depth/Basic/DepthPassTransparentMax.azshader";

		// Billboards have billboarding also in depth shader:
		if (IsBillboardShader(pipelineCacheKey.m_UsedShader))
		{
			// For opaque particles:
			opaqueDepthPass = "shaders/Depth/Billboard/DepthPassBillboard.azshader";
			// For transparent particles:
			minDepthPass = "shaders/Depth/Billboard/DepthPassTransparentMinBillboard.azshader";
			maxDepthPass = "shaders/Depth/Billboard/DepthPassTransparentMaxBillboard.azshader";
		}
		else if (IsMeshShader(pipelineCacheKey.m_UsedShader))
		{
			// For opaque particles:
			opaqueDepthPass = "shaders/Depth/Mesh/DepthPassMesh.azshader";
			// For transparent particles:
			minDepthPass = "shaders/Depth/Mesh/DepthPassTransparentMinMesh.azshader";
			maxDepthPass = "shaders/Depth/Mesh/DepthPassTransparentMaxMesh.azshader";
		}

		// Depth shaders:
		if (IsLitShader(pipelineCacheKey.m_UsedShader))
		{
			if (pipelineCacheKey.m_BlendMode == BlendMode::Solid || pipelineCacheKey.m_BlendMode == BlendMode::Masked)
			{
				currentCaches.m_Type = AssetType_OpaqueDepthShader;
				if (!PK_VERIFY(_AddShaderToLoad(opaqueDepthPass, currentCaches)))
					return false;
			}
			else
			{
				currentCaches.m_Type = AssetType_TransparentDepthMinShader;
				if (!PK_VERIFY(_AddShaderToLoad(minDepthPass, currentCaches)))
					return false;
				currentCaches.m_Type = AssetType_TransparentDepthMaxShader;
				if (!PK_VERIFY(_AddShaderToLoad(maxDepthPass, currentCaches)))
					return false;
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
{
	_OnAssetReady(asset);
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
{
	_OnAssetReady(asset);
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::OnAssetUnloaded(const AZ::Data::AssetId id, const AZ::Data::AssetType type)
{
	(void)id;
	(void)type;
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::OnShaderAssetReinitialized(const AZ::Data::Asset<AZ::RPI::ShaderAsset> &shaderAsset)
{
	_OnAssetReady(shaderAsset);
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::OnShaderReinitialized(const AZ::RPI::Shader &shader)
{
	_OnAssetReady(shader.GetAsset());
}

//----------------------------------------------------------------------------
void	PopcornFXRendererLoader::OnShaderVariantReinitialized(const AZ::RPI::ShaderVariant &shaderVariant)
{
	_OnShaderVariantsReloaded(shaderVariant.GetShaderAsset().GetId(), shaderVariant.GetShaderVariantId());
}

//----------------------------------------------------------------------------

PMaterialCache		PopcornFXRendererLoader::FindMaterial(const SMaterialCacheKey &key) const
{
	const PMaterialCache	*materialCache = m_Materials.Find(key);
	if (materialCache == null)
		return null;
	return *materialCache;
}

//----------------------------------------------------------------------------

PPipelineStateCache	PopcornFXRendererLoader::FindPipelineState(const SPipelineStateCacheKey &key) const
{
	const PPipelineStateCache	*pipelineStateCache = m_PipelineStates.Find(key);
	if (pipelineStateCache == null)
		return null;
	return *pipelineStateCache;
}

//----------------------------------------------------------------------------

PGeometryCache	PopcornFXRendererLoader::FindGeometryCache(const CString &path) const
{
	const PGeometryCache	*geometryCache = m_GeometryCaches.Find(path);
	if (geometryCache == null)
		return null;
	return *geometryCache;
}

//----------------------------------------------------------------------------

void	_SetMeshInfos(PAtomRendererCache rendererCache, const PGeometryCache geometryCache)
{
	rendererCache->m_PerLODMeshCount.Resize(1);
	rendererCache->m_PerLODMeshCount[0] = geometryCache->m_PerGeometryViews.Count();
	rendererCache->m_GlobalMeshBounds = geometryCache->m_GlobalBounds;

	rendererCache->m_SubMeshBounds.Resize(rendererCache->m_PerLODMeshCount[0]);
	for (u32 i = 0; i < rendererCache->m_PerLODMeshCount[0] ; i++)
		rendererCache->m_SubMeshBounds[i] = geometryCache->m_PerGeometryViews[i].m_Bounds;
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::_OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
{
	PK_SCOPEDLOCK(m_Lock);
	SAssetDependencies	*dependencies = m_Assets.Find(asset.GetId());
	if (dependencies == null)
		return;

	if (asset.GetType() == azrtti_typeid<AZ::RPI::ShaderAsset>())
	{
		// Shader loaded:
		AZ::Data::Asset<AZ::RPI::ShaderAsset>	shaderAsset = AZ::Data::Asset<AZ::RPI::ShaderAsset>(dependencies->m_AssetRef);
		AZ::Data::Instance<AZ::RPI::Shader>		shader = AZ::RPI::Shader::FindOrCreate(shaderAsset);

		if (!PK_VERIFY(shader != null))
			return;
		for (const SAssetDependencies::SCaches &cache : dependencies->m_Caches)
		{
			const PPipelineStateCache	*pipelineStateCache = m_PipelineStates.Find(cache.m_PipelineStateKey);

			if (pipelineStateCache != null && *pipelineStateCache != null)
			{
				AZ::RPI::ShaderReloadNotificationBus::MultiHandler::BusConnect(shaderAsset.GetId());

				PK_SCOPEDLOCK_WRITE((*pipelineStateCache)->m_Lock);

				AZ::Data::Instance<AZ::RPI::Shader>		&shaderSlot = _GetShaderSlot(cache.m_Type, *pipelineStateCache);

				shaderSlot = shader;

				AZ::RPI::ShaderVariantStableId			variantStableId = AZ::RPI::RootShaderVariantStableId;

				const bool	isDepthShader = cache.m_Type == AssetType_OpaqueDepthShader || cache.m_Type == AssetType_TransparentDepthMinShader || cache.m_Type == AssetType_TransparentDepthMaxShader;

				if (cache.m_Type == AssetType_MaterialShader || (isDepthShader && IsBillboardShader(cache.m_PipelineStateKey.m_UsedShader)))
				{
					AZ::RPI::ShaderVariantId			variantId = cache.m_PipelineStateKey.GetShaderVariantId(*shader, true, isDepthShader);
					AZ::RPI::ShaderVariantSearchResult	searchResult = shader->FindVariantStableId(variantId);
					variantStableId = searchResult.GetStableId();
				}

				AZ::RPI::ShaderVariant	shaderVariant = shader->GetVariant(variantStableId);

				if (shaderVariant.GetStableId() != variantStableId)
				{
					// Not the variant we are looking for: the variant is not ready, skip.
					// GetVariant triggers a load, onVariantReinitialized will be call and will handle
					// pipelineStateCache creation.
					continue;
				}

				// It seems that we must keep a reference to the variant for the variant callbacks to work.
				AZ::Data::AssetId	variantID = shaderVariant.GetShaderVariantAsset().GetId();
				if (m_LoadedVariants.Find(variantID) != null)
					m_LoadedVariants.Remove(variantID);
				m_LoadedVariants.Insert(variantID, shaderVariant.GetShaderVariantAsset());

				AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	&pipelineStateSlot = _GetPipelineStateSlot(cache.m_Type, *pipelineStateCache);
				pipelineStateSlot = _CreatePipelineStateCache(*shader, shaderVariant, cache.m_Type, cache.m_PipelineStateKey);
				(*pipelineStateCache)->m_Modified = true;
			}
		}
	}
	else if (asset.GetType() == azrtti_typeid<AZ::RPI::StreamingImageAsset>())
	{
		// Material texture loaded:
		AZ::Data::Asset<AZ::RPI::StreamingImageAsset>	streamingImgAsset = AZ::Data::Asset<AZ::RPI::StreamingImageAsset>(dependencies->m_AssetRef);
		for (const SAssetDependencies::SCaches &cache : dependencies->m_Caches)
		{
			const PMaterialCache	*materialCache = m_Materials.Find(cache.m_MaterialKey);

			if (materialCache != null && *materialCache != null)
			{
				PK_SCOPEDLOCK_WRITE((*materialCache)->m_Lock);

				AZ::Data::Instance<AZ::RPI::StreamingImage> &streamingImageSlot = _GetStreamingImageSlot(cache.m_Type, *materialCache);
				streamingImageSlot = AZ::RPI::StreamingImage::FindOrCreate(streamingImgAsset);
				(*materialCache)->m_Modified = true;
			}
		}
	}
	else if (asset.GetType() == azrtti_typeid<AZ::RPI::ModelAsset>())
	{
		// Dependency: geometry cache. Will hold actual mesh data
		// We should have only one geometryCache dependency per model asset
		PGeometryCache	geometryCache = null;
		for (const SAssetDependencies::SCaches &cache : dependencies->m_Caches)
		{
			PGeometryCache *found = m_GeometryCaches.Find(cache.m_GeometryPath);

			if (found != null && *found != null)
			{
				geometryCache = *found;
				break;
			}
		}
		if (!PK_VERIFY(geometryCache != null))
			return;

		// Cache found: setup with model data
		{
			PK_SCOPEDLOCK_WRITE(geometryCache->m_Lock);

			AZ::Data::Asset<AZ::RPI::ModelAsset>	modelAsset = AZ::Data::Asset<AZ::RPI::ModelAsset>(asset);
			AZ::Data::Instance<AZ::RPI::Model>		model = AZ::RPI::Model::FindOrCreate(modelAsset);
			AZ::Data::Instance<AZ::RPI::ModelLod>	modelLod = model->GetLods()[0];
			geometryCache->m_ModelLod = modelLod;

			const u32	meshCount = static_cast<u32>(modelLod->GetMeshes().size());
			if (!PK_VERIFY(geometryCache->m_PerGeometryViews.Resize(meshCount)))
				return;

			geometryCache->m_GlobalBounds = ToPk(modelAsset->GetAabb());

			// setup a stream layout and shader input contract for the vertex streams
			static const char	*PositionSemantic = "POSITION";
			static const char	*NormalSemantic = "NORMAL";
			static const char	*TangentSemantic = "TANGENT";
			static const char	*BitangentSemantic = "BITANGENT";
			static const char	*UVSemantic = "UV";
			static const AZ::RHI::Format PositionStreamFormat = AZ::RHI::Format::R32G32B32_FLOAT;
			static const AZ::RHI::Format NormalStreamFormat = AZ::RHI::Format::R32G32B32_FLOAT;
			static const AZ::RHI::Format TangentStreamFormat = AZ::RHI::Format::R32G32B32A32_FLOAT;
			static const AZ::RHI::Format BitangentStreamFormat = AZ::RHI::Format::R32G32B32_FLOAT;
			static const AZ::RHI::Format UVStreamFormat = AZ::RHI::Format::R32G32_FLOAT;

			AZ::RHI::InputStreamLayoutBuilder layoutBuilder;
			layoutBuilder.AddBuffer()->Channel(PositionSemantic, PositionStreamFormat);
			layoutBuilder.AddBuffer()->Channel(NormalSemantic, NormalStreamFormat);
			layoutBuilder.AddBuffer()->Channel(UVSemantic, UVStreamFormat);
			layoutBuilder.AddBuffer()->Channel(TangentSemantic, TangentStreamFormat);
			layoutBuilder.AddBuffer()->Channel(BitangentSemantic, BitangentStreamFormat);
			AZ::RHI::InputStreamLayout inputStreamLayout = layoutBuilder.End();

			AZ::RPI::ShaderInputContract::StreamChannelInfo positionStreamChannelInfo;
			positionStreamChannelInfo.m_semantic = AZ::RHI::ShaderSemantic(AZ::Name(PositionSemantic));
			positionStreamChannelInfo.m_componentCount = AZ::RHI::GetFormatComponentCount(PositionStreamFormat);

			AZ::RPI::ShaderInputContract::StreamChannelInfo normalStreamChannelInfo;
			normalStreamChannelInfo.m_semantic = AZ::RHI::ShaderSemantic(AZ::Name(NormalSemantic));
			normalStreamChannelInfo.m_componentCount = AZ::RHI::GetFormatComponentCount(NormalStreamFormat);

			AZ::RPI::ShaderInputContract::StreamChannelInfo tangentStreamChannelInfo;
			tangentStreamChannelInfo.m_semantic = AZ::RHI::ShaderSemantic(AZ::Name(TangentSemantic));
			tangentStreamChannelInfo.m_componentCount = AZ::RHI::GetFormatComponentCount(TangentStreamFormat);
			tangentStreamChannelInfo.m_isOptional = true;

			AZ::RPI::ShaderInputContract::StreamChannelInfo bitangentStreamChannelInfo;
			bitangentStreamChannelInfo.m_semantic = AZ::RHI::ShaderSemantic(AZ::Name(BitangentSemantic));
			bitangentStreamChannelInfo.m_componentCount = AZ::RHI::GetFormatComponentCount(BitangentStreamFormat);
			bitangentStreamChannelInfo.m_isOptional = true;

			AZ::RPI::ShaderInputContract::StreamChannelInfo uvStreamChannelInfo;
			uvStreamChannelInfo.m_semantic = AZ::RHI::ShaderSemantic(AZ::Name(UVSemantic));
			uvStreamChannelInfo.m_componentCount = AZ::RHI::GetFormatComponentCount(UVStreamFormat);
			uvStreamChannelInfo.m_isOptional = true;

			AZ::RPI::ShaderInputContract shaderInputContract;
			shaderInputContract.m_streamChannels.emplace_back(positionStreamChannelInfo);
			shaderInputContract.m_streamChannels.emplace_back(normalStreamChannelInfo);
			shaderInputContract.m_streamChannels.emplace_back(tangentStreamChannelInfo);
			shaderInputContract.m_streamChannels.emplace_back(bitangentStreamChannelInfo);
			shaderInputContract.m_streamChannels.emplace_back(uvStreamChannelInfo);

			for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
			{
				const AZ::RPI::ModelLod::Mesh &mesh = modelLod->GetMeshes()[meshIndex];
				geometryCache->m_PerGeometryViews[meshIndex].m_Bounds = ToPk(modelAsset->GetLodAssets()[0]->GetMeshes()[meshIndex].GetAabb());

				AZ::RPI::ModelLod::StreamBufferViewList streamBufferViews;
				[[maybe_unused]] bool result = modelLod->GetStreamsForMesh(	inputStreamLayout,
																			streamBufferViews,
																			null,
																			shaderInputContract,
																			meshIndex);
				AZ_Assert(result, "Failed to retrieve mesh stream buffer views");

				//uint32_t positionBufferByteCount = streamBufferViews[0].GetBuffer()->GetDescriptor().m_byteCount;

				//uint32_t normalBufferByteCount = streamBufferViews[1].GetBuffer()->GetDescriptor().m_byteCount;

				AZ::u64	tangentBufferByteCount = streamBufferViews[2].GetBuffer()->GetDescriptor().m_byteCount;

				AZ::u64	bitangentBufferByteCount = streamBufferViews[3].GetBuffer()->GetDescriptor().m_byteCount;

				AZ::u64	uvBufferByteCount = streamBufferViews[4].GetBuffer()->GetDescriptor().m_byteCount;

				geometryCache->m_PerGeometryViews[meshIndex].m_IndexCount = mesh.m_drawArguments.m_indexed.m_indexCount;

				geometryCache->m_PerGeometryViews[meshIndex].m_PositionBuffer = streamBufferViews[0];

				geometryCache->m_PerGeometryViews[meshIndex].m_NormalBuffer = streamBufferViews[1];

				if (tangentBufferByteCount > 0)
					geometryCache->m_PerGeometryViews[meshIndex].m_TangentBuffer = streamBufferViews[2];

				if (bitangentBufferByteCount > 0)
					geometryCache->m_PerGeometryViews[meshIndex].m_BitangentBuffer = streamBufferViews[3];

				if (uvBufferByteCount > 0)
					geometryCache->m_PerGeometryViews[meshIndex].m_UVBuffer = streamBufferViews[4];

				geometryCache->m_PerGeometryViews[meshIndex].m_IndexBuffer = mesh.m_indexBufferView;
			}
			geometryCache->m_Modified = true;

			// Dependency: rendererCache(s) using this model.
			// Will update the rendercaches with mesh bbox and mesh count
			TArray<PAtomRendererCache> *rendererCaches = m_RendererCacheDependencies.Find(asset.GetId());
			if (rendererCaches != null && geometryCache != null)
			{
				for (PAtomRendererCache rendererCache : *rendererCaches)
				{
					_SetMeshInfos(rendererCache, geometryCache);
				}
				rendererCaches->Clean();
			}
		}
	}
}

//----------------------------------------------------------------------------

void	PopcornFXRendererLoader::_OnShaderVariantsReloaded(const AZ::Data::AssetId &shaderId, const AZ::RPI::ShaderVariantId &shaderVariantId)
{
	AZ_UNUSED(shaderVariantId);
	PK_SCOPEDLOCK(m_Lock);
	SAssetDependencies	*dependencies = m_Assets.Find(shaderId);

	if (!PK_VERIFY(dependencies != null))
	{
		CLog::Log(PK_ERROR, "_OnShaderVariantsReloaded: Asset dependencies not registered");
		return;
	}

	AZ::Data::Asset<AZ::RPI::ShaderAsset>	shaderAsset = AZ::Data::Asset<AZ::RPI::ShaderAsset>(dependencies->m_AssetRef);

	if (!PK_VERIFY(shaderAsset.IsReady()))
	{
		CLog::Log(PK_ERROR, "_OnShaderVariantsReloaded: Shader asset '%s' is not ready", shaderAsset.GetHint().c_str());
		return;
	}

	AZ::Data::Instance<AZ::RPI::Shader>		shader = AZ::RPI::Shader::FindOrCreate(shaderAsset);

	if (!PK_VERIFY(shader != null))
	{
		CLog::Log(PK_ERROR, "_OnShaderVariantsReloaded: Shader '%s' not found", shaderAsset.GetHint().c_str());
		return;
	}

	bool	variantFound = false;

	for (const SAssetDependencies::SCaches &cache : dependencies->m_Caches)
	{
		const PPipelineStateCache	*pipelineStateCache = m_PipelineStates.Find(cache.m_PipelineStateKey);

		if (pipelineStateCache != null && *pipelineStateCache != null)
		{
			AZ::RPI::ShaderVariantStableId			variantStableId = AZ::RPI::RootShaderVariantStableId;

			const bool	isDepthShader = cache.m_Type == AssetType_OpaqueDepthShader || cache.m_Type == AssetType_TransparentDepthMinShader || cache.m_Type == AssetType_TransparentDepthMaxShader;

			if (cache.m_Type == AssetType_MaterialShader || (isDepthShader && IsBillboardShader(cache.m_PipelineStateKey.m_UsedShader)))
			{
				AZ::RPI::ShaderVariantId			variantId = cache.m_PipelineStateKey.GetShaderVariantId(*shader, true, isDepthShader);
				AZ::RPI::ShaderVariantSearchResult	searchResult = shader->FindVariantStableId(variantId);
				variantStableId = searchResult.GetStableId();
			}

			AZ::RPI::ShaderVariant	shaderVariant = shader->GetVariant(variantStableId);

			if (shaderVariant.GetStableId() != variantStableId)
			{
				// Not the variant we are looking for: the variant is not ready.
				// this should not happen since the function should be triggered by
				// variant reinitialized callback.
				PK_ASSERT_NOT_REACHED_MESSAGE("Variant not ready");
				continue;
			}

			// It seems that we must keep a reference to the variant for the variant callbacks to work.
			AZ::Data::AssetId	variantID = shaderVariant.GetShaderVariantAsset().GetId();
			if (m_LoadedVariants.Find(variantID) != null)
				m_LoadedVariants.Remove(variantID);
			m_LoadedVariants.Insert(variantID, shaderVariant.GetShaderVariantAsset());

			PK_SCOPEDLOCK_WRITE((*pipelineStateCache)->m_Lock);

			AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	&pipelineStateSlot = _GetPipelineStateSlot(cache.m_Type, *pipelineStateCache);
			pipelineStateSlot = _CreatePipelineStateCache(*shader, shaderVariant, cache.m_Type, cache.m_PipelineStateKey);
			(*pipelineStateCache)->m_Modified = true;
			variantFound = true;
		}
	}
	/*
	We store sharedAsset - pipelineCache dependencies, but this callback is called for each variant, so it's not mandatory that
	the variant has an actual dependency to any pipelineCache dependent of the shader: don't log an error.
	if (!PK_VERIFY(variantFound))
		CLog::Log(PK_ERROR, "_OnShaderVariantsReloaded: No shader variant found");
	*/
}

//----------------------------------------------------------------------------

AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	PopcornFXRendererLoader::_CreatePipelineStateCache(	AZ::RPI::Shader &shader,
																								AZ::RPI::ShaderVariant &shaderVariant,
																								EAssetType assetType,
																								const SPipelineStateCacheKey &key)
{
	AZ::RHI::PipelineStateDescriptorForDraw	pipelineStateDesc = AZ::RHI::PipelineStateDescriptorForDraw();
	shaderVariant.ConfigurePipelineState(pipelineStateDesc);

	EPopcornFXShader	usedShader = key.m_UsedShader;

	if (IsBillboardShader(usedShader))
	{
		// GPU billboarding: only has a float2 input per vertex
		AZ::RHI::InputStreamLayoutBuilder layoutBuilder;
		layoutBuilder.AddBuffer()->Channel("POSITION", AZ::RHI::Format::R32G32_FLOAT);
		pipelineStateDesc.m_inputStreamLayout = layoutBuilder.End();
	}
	else if (IsRibbonShader(usedShader))
	{
		AZ::RHI::InputStreamLayoutBuilder layoutBuilder;
		if (usedShader == EPopcornFXShader::RibbonLegacy_Shader ||
			usedShader == EPopcornFXShader::RibbonLitLegacy_Shader ||
			usedShader == EPopcornFXShader::RibbonDistortion_Shader)
		{
			layoutBuilder.AddBuffer()->Channel("POSITION", AZ::RHI::Format::R32G32B32_FLOAT);
			if (assetType == AssetType_MaterialShader)
			{
				layoutBuilder.AddBuffer()->Channel("UV0", AZ::RHI::Format::R32G32_FLOAT); // texCoord 0
			}
		}
		else if (usedShader == EPopcornFXShader::RibbonAtlasBlendLegacy_Shader ||
			usedShader == EPopcornFXShader::RibbonLitAtlasBlendLegacy_Shader)
		{
			layoutBuilder.AddBuffer()->Channel("POSITION", AZ::RHI::Format::R32G32B32_FLOAT);
			if (assetType == AssetType_MaterialShader)
			{
				layoutBuilder.AddBuffer()->Channel("UV0", AZ::RHI::Format::R32G32_FLOAT); // texCoord 0
				layoutBuilder.AddBuffer()->Channel("UV1", AZ::RHI::Format::R32G32_FLOAT); // texCoord 1
				layoutBuilder.AddBuffer()->Channel("UV2", AZ::RHI::Format::R32_FLOAT); // texFrameLerp
			}
		}
		else if (usedShader == EPopcornFXShader::RibbonCorrectDeformationLegacy_Shader ||
			usedShader == EPopcornFXShader::RibbonLitCorrectDeformationLegacy_Shader)
		{
			layoutBuilder.AddBuffer()->Channel("POSITION", AZ::RHI::Format::R32G32B32_FLOAT);
			if (assetType == AssetType_MaterialShader)
			{
				layoutBuilder.AddBuffer()->Channel("UV0", AZ::RHI::Format::R32G32_FLOAT); // texCoord 0
				layoutBuilder.AddBuffer()->Channel("UV1", AZ::RHI::Format::R32G32B32A32_FLOAT); // UVRemap
				layoutBuilder.AddBuffer()->Channel("UV2", AZ::RHI::Format::R32G32_FLOAT); // UVFactors
			}
		}
		else
		{
			PK_ASSERT_NOT_REACHED_MESSAGE("Ribbon shader is not handled");
		}
		if (assetType == AssetType_MaterialShader && IsLitShader(usedShader))
		{
			layoutBuilder.AddBuffer()->Channel("NORMAL", AZ::RHI::Format::R32G32B32_FLOAT); // normal
			layoutBuilder.AddBuffer()->Channel("TANGENT", AZ::RHI::Format::R32G32B32A32_FLOAT); // tangent
		}
		pipelineStateDesc.m_inputStreamLayout = layoutBuilder.End();
	}
	else if (IsMeshShader(usedShader))
	{
		AZ::RHI::InputStreamLayoutBuilder layoutBuilder;
		layoutBuilder.AddBuffer()->Channel("POSITION", AZ::RHI::Format::R32G32B32_FLOAT);
		layoutBuilder.AddBuffer()->Channel("UV0", AZ::RHI::Format::R32G32_FLOAT); // texCoord 0
		if (assetType == AssetType_MaterialShader && IsLitShader(usedShader))
		{
			layoutBuilder.AddBuffer()->Channel("NORMAL", AZ::RHI::Format::R32G32B32_FLOAT); // normal
			layoutBuilder.AddBuffer()->Channel("TANGENT", AZ::RHI::Format::R32G32B32A32_FLOAT); // tangent
		}
		pipelineStateDesc.m_inputStreamLayout = layoutBuilder.End();
	}

	if (assetType == AssetType_MaterialShader)
	{
		pipelineStateDesc.m_renderStates.m_depthStencilState.m_depth.m_writeMask = AZ::RHI::DepthWriteMask::Zero;
		//		if (	key.m_BlendMode == BlendMode::Solid || key.m_BlendMode == BlendMode::Masked)
		//				pipelineStateDesc.m_renderStates.m_depthStencilState.m_depth.m_writeMask = AZ::RHI::DepthWriteMask::All;

		if (IsLegacyShader(usedShader))
		{
			pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_enable = 1;
			pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendOp = AZ::RHI::BlendOp::Add;

			if (key.m_BlendMode == BlendMode::Additive)
			{
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendSource = AZ::RHI::BlendFactor::AlphaSource;
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendDest = AZ::RHI::BlendFactor::One;
			}
			else if (key.m_BlendMode == BlendMode::AdditiveNoAlpha)
			{
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendSource = AZ::RHI::BlendFactor::One;
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendDest = AZ::RHI::BlendFactor::One;
			}
			else if (key.m_BlendMode == BlendMode::AlphaBlend)
			{
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendSource = AZ::RHI::BlendFactor::AlphaSource;
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendDest = AZ::RHI::BlendFactor::AlphaSourceInverse;
			}
			else if (key.m_BlendMode == BlendMode::PremultipliedAlpha)
			{
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendSource = AZ::RHI::BlendFactor::One;
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendDest = AZ::RHI::BlendFactor::AlphaSourceInverse;
			}
			else if (key.m_BlendMode == BlendMode::Solid || key.m_BlendMode == BlendMode::Masked)
			{
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendSource = AZ::RHI::BlendFactor::One;
				pipelineStateDesc.m_renderStates.m_blendState.m_targets[0].m_blendDest = AZ::RHI::BlendFactor::Zero;
			}
		}
	}

	const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
	const char	*shaderPath = GetPopornFXUsedShaderPath(key.m_UsedShader);

	if (!PK_VERIFY(scene != null))
	{
		CLog::Log(PK_ERROR, "Failed to get current default scene to initialize '%s'", shaderPath);
		return null;
	}

	//		if (key.m_BlendMode == BlendMode::Solid || key.m_BlendMode == BlendMode::Masked)

	scene->ConfigurePipelineState(shader.GetDrawListTag(), pipelineStateDesc);
	AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	pipelineState = shader.AcquirePipelineState(pipelineStateDesc);
	if (!PK_VERIFY(pipelineState != null))
	{
		CLog::Log(PK_ERROR, "Failed to acquire default pipeline state for shader '%s'", shaderPath);
		return null;
	}

	if (!PK_VERIFY(pipelineState->IsInitialized()))
	{
		CLog::Log(PK_ERROR, "Uninitialized pipeline state for shader '%s'", shaderPath);
	}
	return pipelineState;
}

//----------------------------------------------------------------------------

bool	PopcornFXRendererLoader::SAssetDependencies::SCaches::operator == (const SCaches &oth) const
{
	return	m_MaterialKey == oth.m_MaterialKey &&
			m_PipelineStateKey == oth.m_PipelineStateKey &&
			m_GeometryPath == oth.m_GeometryPath &&
			m_Type == oth.m_Type;
}

//----------------------------------------------------------------------------

AZ::Data::Instance<AZ::RPI::Shader>	&PopcornFXRendererLoader::_GetShaderSlot(EAssetType type, const PPipelineStateCache &pipelineState) const
{
	if (type == AssetType_MaterialShader)
		return pipelineState->m_MaterialShader;
	else if (type == AssetType_OpaqueDepthShader)
		return pipelineState->m_OpaqueDepthShader;
	else if (type == AssetType_TransparentDepthMinShader)
		return pipelineState->m_TransparentDepthMinShader;
	else if (type == AssetType_TransparentDepthMaxShader)
		return pipelineState->m_TransparentDepthMaxShader;
	else
	{
		PK_ASSERT_NOT_REACHED();
		return pipelineState->m_MaterialShader;
	}
}

//----------------------------------------------------------------------------

AZ::RHI::ConstPtr<AZ::RHI::PipelineState>	&PopcornFXRendererLoader::_GetPipelineStateSlot(EAssetType type, const PPipelineStateCache &pipelineState) const
{
	if (type == AssetType_MaterialShader)
		return pipelineState->m_MaterialPipelineState;
	else if (type == AssetType_OpaqueDepthShader)
		return pipelineState->m_OpaqueDepthPipelineState;
	else if (type == AssetType_TransparentDepthMinShader)
		return pipelineState->m_TransparentDepthMinPipelineState;
	else if (type == AssetType_TransparentDepthMaxShader)
		return pipelineState->m_TransparentDepthMaxPipelineState;
	else
	{
		PK_ASSERT_NOT_REACHED();
		return pipelineState->m_MaterialPipelineState;
	}
}

//----------------------------------------------------------------------------

AZ::Data::Instance<AZ::RPI::StreamingImage> &PopcornFXRendererLoader::_GetStreamingImageSlot(EAssetType type, const PMaterialCache &material) const
{
	if (type == AssetType_DiffuseMap)
		return material->m_DiffuseMap;
	else if (type == AssetType_DiffuseRampMap)
		return material->m_DiffuseRampMap;
	else if (type == AssetType_EmissiveMap)
		return material->m_EmissiveMap;
	else if (type == AssetType_EmissiveRampMap)
		return material->m_EmissiveRampMap;
	else if (type == AssetType_NormalMap)
		return material->m_NormalMap;
	else if (type == AssetType_MotionVectorsMap)
		return material->m_MotionVectorsMap;
	else if (type == AssetType_AlphaMap)
		return material->m_AlphaMap;
	else if (type == AssetType_DistortionMap)
		return material->m_DistortionMap;
	else
	{
		PK_ASSERT_NOT_REACHED();
		return material->m_DiffuseMap;
	}
}

//----------------------------------------------------------------------------

bool	PopcornFXRendererLoader::_AddTextureToLoad(const char *texturePath, const SAssetDependencies::SCaches &currentCaches)
{
	if (!PK_VERIFY(texturePath != null))
		return false;
	const AZ::Data::AssetId	textureId = _LoadTexture(texturePath);
	if (textureId.IsValid())
	{
		SAssetDependencies	*dependencies = _InsertAssetIFN(textureId, currentCaches);
		if (!PK_VERIFY(dependencies != null))
			return false;
		dependencies->m_AssetRef = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::StreamingImageAsset>(textureId, AZ::Data::AssetLoadBehavior::QueueLoad);
		if (dependencies->m_AssetRef.IsReady())
			PopcornFXRendererLoader::OnAssetReady(dependencies->m_AssetRef);
		AZ::Data::AssetBus::MultiHandler::BusConnect(textureId);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------

bool	PopcornFXRendererLoader::_AddShaderToLoad(const char *shaderPathPtr, const SAssetDependencies::SCaches &currentCaches)
{
	if (!PK_VERIFY(shaderPathPtr != null))
		return false;
	AZStd::string		shaderPath = shaderPathPtr;
	AZ::Data::AssetId	shaderId;
	EBUS_EVENT_RESULT(shaderId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, shaderPath.data(), azrtti_typeid<AZ::RPI::ShaderAsset>(), false);
	if (!PK_VERIFY(shaderId.IsValid()))
	{
		CLog::Log(PK_ERROR, "Could not load shader '%s'", shaderPathPtr);
		return false;
	}
	SAssetDependencies	*dependencies = _InsertAssetIFN(shaderId, currentCaches);
	if (!PK_VERIFY(dependencies != null))
		return false;
	dependencies->m_AssetRef = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::ShaderAsset>(shaderId, AZ::Data::AssetLoadBehavior::QueueLoad);
	if (dependencies->m_AssetRef.IsReady())
		OnAssetReady(dependencies->m_AssetRef);
	AZ::Data::AssetBus::MultiHandler::BusConnect(shaderId);
	return true;
}

//----------------------------------------------------------------------------

AZ::Data::AssetId	PopcornFXRendererLoader::_LoadTexture(const CString &path)
{
	AzFramework::AssetSystem::AssetStatus	status = AzFramework::AssetSystem::AssetStatus_Unknown;
	EBUS_EVENT_RESULT(status, AzFramework::AssetSystemRequestBus, CompileAssetSync, path.Data());
	if (status != AzFramework::AssetSystem::AssetStatus_Compiled)
		CLog::Log(PK_ERROR, "Could not compile image at '%s'", path.Data());

	AZ::Data::AssetId	streamingImageAssetId;
	EBUS_EVENT_RESULT(streamingImageAssetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, path.Data(), azrtti_typeid<AZ::RPI::StreamingImageAsset>(), false);
	if (!streamingImageAssetId.IsValid())
		CLog::Log(PK_ERROR, "Failed to get streaming image asset id with path '%s'", path.Data());
	return streamingImageAssetId;
}

//----------------------------------------------------------------------------

PopcornFXRendererLoader::SAssetDependencies	*PopcornFXRendererLoader::_InsertAssetIFN(const AZ::Data::AssetId &assetId, const SAssetDependencies::SCaches currentCaches)
{
	if (!assetId.IsValid())
		return null;

	SAssetDependencies	*dependencies = m_Assets.Find(assetId);
	if (dependencies == null)
	{
		dependencies = m_Assets.Insert(assetId, SAssetDependencies());
		if (!PK_VERIFY(dependencies != null))
			return null;
		if (!PK_VERIFY(dependencies->m_Caches.PushBack(currentCaches).Valid()))
			return null;
	}
	else
	{
		if (!dependencies->m_Caches.Contains(currentCaches) &&
			!PK_VERIFY(dependencies->m_Caches.PushBack(currentCaches).Valid()))
			return null;
	}
	return dependencies;
}

//----------------------------------------------------------------------------

// TODO: Merge with _InsertAsset
bool PopcornFXRendererLoader::_InsertRendererCacheDependency(const AZ::Data::AssetId &assetId, const PAtomRendererCache &rendererCache)
{
	if (!assetId.IsValid())
		return false;

	TArray<PAtomRendererCache>	*dependencies = m_RendererCacheDependencies.Find(assetId);
	if (dependencies == null)
	{
		dependencies = m_RendererCacheDependencies.Insert(assetId, TArray<PAtomRendererCache>());
		if (!PK_VERIFY(dependencies != null))
			return false;
		if (!PK_VERIFY(dependencies->PushBack(rendererCache).Valid()))
			return false;
	}
	else
	{
		if (!dependencies->Contains(rendererCache) &&
			!PK_VERIFY(dependencies->PushBack(rendererCache).Valid()))
			return false;
	}
	return true;
}

//----------------------------------------------------------------------------

PPipelineStateCache	PopcornFXRendererLoader::_GetOrCreatePipelineCache(const SPipelineStateCacheKey &key, bool &exist)
{
	PPipelineStateCache	pipelineCache = null;
	PPipelineStateCache	*found = m_PipelineStates.Find(key);

	if (found == null)
	{
		exist = false;
		pipelineCache = PK_NEW(CPipelineStateCache);
		if (pipelineCache == null || m_PipelineStates.Insert(key, pipelineCache) == null)
			return null;
	}
	else
	{
		exist = true;
		pipelineCache = *found;
	}
	return pipelineCache;
}

//----------------------------------------------------------------------------

PMaterialCache		PopcornFXRendererLoader::_GetOrCreateMaterialCache(const SMaterialCacheKey &key, bool &exist)
{
	PMaterialCache	materialCache = null;
	PMaterialCache	*found = m_Materials.Find(key);

	if (found == null)
	{
		exist = false;
		materialCache = PK_NEW(CMaterialCache);
		if (materialCache == null || m_Materials.Insert(key, materialCache) == null)
			return null;
	}
	else
	{
		exist = true;
		materialCache = *found;
	}
	return materialCache;
}

//----------------------------------------------------------------------------

AZ::Data::AssetId	_GetMeshAssetId(const CString &path)
{
	AZ::Data::AssetId	assetId;

	AzFramework::AssetSystem::AssetStatus	status = AzFramework::AssetSystem::AssetStatus_Unknown;
	EBUS_EVENT_RESULT(status, AzFramework::AssetSystemRequestBus, CompileAssetSync, path.Data());
	if (status != AzFramework::AssetSystem::AssetStatus_Compiled)
	{
		CLog::Log(PK_ERROR, "Could not compile model at '%s'", path.Data());
		return assetId;
	}

	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, path.Data(), azrtti_typeid<AZ::RPI::Model>(), false);
	if (!assetId.IsValid())
	{
		CLog::Log(PK_ERROR, "Failed to get model asset id with path '%s'", path.Data());
		return assetId;
	}
	return assetId;
}

//----------------------------------------------------------------------------

bool		PopcornFXRendererLoader::AddGeometryToLoad(const CString &path, const PAtomRendererCache &rendererCache)
{
	PK_SCOPEDLOCK(m_Lock);
	if (!PK_VERIFY(path != null))
		return false;

	PGeometryCache	*found = m_GeometryCaches.Find(path);

	if (found != null) // Geom. cache already created
	{
		if (found->Get()->m_Modified) 
		{
			// Geom. cache already loaded, data is ready
			_SetMeshInfos(rendererCache, *found);
		}
		else
		{
			// Geom. cache not loaded, add a dependency
			// to update renderercache
			AZ::Data::AssetId	assetID = _GetMeshAssetId(path); 
			if (assetID.IsValid())
				_InsertRendererCacheDependency(assetID, rendererCache);
		}
		return true;
	}

	// Create geom. cache
	PGeometryCache	geometryChache = PK_NEW(CGeometryCache);
	if (geometryChache == null || m_GeometryCaches.Insert(path, geometryChache) == null)
	{
		CLog::Log(PK_ERROR, "Failed to create geometry cache");
		return false;
	}

	// Create a dependency between model and geometry cache
	// and queue load of the model and add
	AZ::Data::AssetId	assetID = _GetMeshAssetId(path);

	SAssetDependencies::SCaches	currentCache;
	currentCache.m_Type = AssetType_Geometry;
	currentCache.m_GeometryPath = path;
	_InsertAssetIFN(assetID, currentCache);

	_InsertRendererCacheDependency(assetID, rendererCache);

	AZ::Data::AssetBus::MultiHandler::BusConnect(assetID);

	AZ::Data::Asset<AZ::RPI::ModelAsset>	modelAsset = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::ModelAsset>(assetID, AZ::Data::AssetLoadBehavior::QueueLoad);
	modelAsset.QueueLoad();
	if (modelAsset.IsReady())
		OnAssetReady(modelAsset);

	return true;
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
