//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "PopcornFXFeatureProcessor.h"
#include "Integration/PopcornFXUtils.h"
#include <pk_kernel/include/kr_profiler.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RHI/DrawPacketBuilder.h>
#include <Atom/RPI.Public/Scene.h>

#include <Atom/Feature/RenderCommon.h>
#if PK_O3DE_MAJOR_VERSION == 2111
#include <Atom/Feature/ReflectionProbe/ReflectionProbeFeatureProcessor.h>
#endif

#if defined(O3DE_USE_PK)

namespace PopcornFX {
//----------------------------------------------------------------------------

void	CPopcornFXFeatureProcessor::Reflect(AZ::ReflectContext *context)
{
	if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
	{
		serializeContext
			->Class<CPopcornFXFeatureProcessor, AZ::RPI::FeatureProcessor>()
			->Version(0);
	}
}

CPopcornFXFeatureProcessor::CPopcornFXFeatureProcessor()
{
}

void	CPopcornFXFeatureProcessor::Activate()
{
	m_RenderManager.SetFeatureProcessor(this);
}

void	CPopcornFXFeatureProcessor::Deactivate()
{

}

void	CPopcornFXFeatureProcessor::Simulate(const SimulatePacket &packet)
{
	AZ_UNUSED(packet);
	if (!CCurrentThread::IsRegistered())
		CCurrentThread::RegisterUserThread();

	// To enable once O3DE moves the simple point light processor GPU buffer update into their Render() function instead of Simulate()
#if 0
	AppendLightParticles();
#endif
}

void	CPopcornFXFeatureProcessor::Render(const RenderPacket &packet)
{
	const SAtomRenderContext	&drawCalls = m_RenderManager.GetRenderContext();

	// Delete the DrawPackets that were used last frame
	m_drawPackets.clear();

	{
		PK_NAMEDSCOPEDPROFILE("Append draw calls");

		for (const SAtomRenderContext::SDrawCall &dc : drawCalls.m_DrawCalls)
		{
			for (const AZ::RPI::ViewPtr &view : packet.m_views)
			{
				const AZ::RHI::DrawPacket	*drawPacket = BuildDrawPacket(dc, view->GetRHIShaderResourceGroup(), 0);
				m_drawPackets.emplace_back(drawPacket);

				const CFloat3		&bboxCenter = dc.m_BoundingBox.Center();
				const AZ::Matrix4x4	&cameraMatrix = view->GetViewToWorldMatrix();
				CFloat3				cameraPosition = ToPk(cameraMatrix.GetTranslation());

				view->AddDrawPacket(drawPacket, (cameraPosition - bboxCenter).Length());
			}
		}
	}
}

void	CPopcornFXFeatureProcessor::AppendLightParticles()
{
	SAtomRenderContext	&drawCalls = const_cast<SAtomRenderContext&>(m_RenderManager.GetRenderContext());

	if (!drawCalls.m_Lights.Empty())
	{
		PK_NAMEDSCOPEDPROFILE("Append lights");
		if (PK_VERIFY(drawCalls.m_LightHandles.Reserve(drawCalls.m_Lights.Count())))
		{
			SAtomRenderContext::ParticleLightProcessor	*processor = GetParentScene()->GetFeatureProcessor<SAtomRenderContext::ParticleLightProcessor>();

			for (const SAtomRenderContext::SLight &light : drawCalls.m_Lights)
			{
				SAtomRenderContext::ParticleLightHandle	lightHandle = processor->AcquireLight();
				processor->SetAttenuationRadius(lightHandle, light.m_AttenuationRadius);
				processor->SetPosition(lightHandle, ToAZ(light.m_Position));

				// TODO: can this be simplified, and check the color space
				// Here, it's doing a CFloat3->AZ::Vector3->AZ::Color->AZ::Render::PhotometricColor convertion/copy per particle..
				AZ::Render::PhotometricColor<SAtomRenderContext::ParticleLightProcessor::PhotometricUnitType> lightColor(AZ::Color::CreateFromVector3(ToAZ(light.m_Color)));
				processor->SetRgbIntensity(lightHandle, lightColor);

				PK_VERIFY(drawCalls.m_LightHandles.PushBack(lightHandle).Valid());
			}
		}
	}
}

const AZ::RHI::DrawPacket	*CPopcornFXFeatureProcessor::BuildDrawPacket(	const SAtomRenderContext::SDrawCall &pkfxDrawCall,
																			const AZ::RHI::ShaderResourceGroup *viewSrg,
																			AZ::RHI::DrawItemSortKey sortKey)
{
	AZ_UNUSED(viewSrg);
	AZ::RHI::DrawPacketBuilder	dpBuilder;

	dpBuilder.Begin(null);
	dpBuilder.SetDrawArguments(pkfxDrawCall.m_DrawIndexed);
	dpBuilder.SetIndexBufferView(pkfxDrawCall.m_Indices);
	dpBuilder.AddShaderResourceGroup(pkfxDrawCall.m_RendererSrg->GetRHIShaderResourceGroup());
	dpBuilder.AddShaderResourceGroup(pkfxDrawCall.m_MaterialSrg->GetRHIShaderResourceGroup());

	// Actual particle rendering:
	{
		AZ::RHI::DrawPacketBuilder::DrawRequest	materialDr;
		materialDr.m_listTag = pkfxDrawCall.m_MaterialDrawList;
		materialDr.m_pipelineState = pkfxDrawCall.m_MaterialPipelineState.get();
#if PK_O3DE_MAJOR_VERSION >= 2205
		materialDr.m_streamBufferViews = AZStd::span<const AZ::RHI::StreamBufferView>(	pkfxDrawCall.m_VertexInputs.RawDataPointer(),
																						pkfxDrawCall.m_VertexInputs.Count());
#else
		materialDr.m_streamBufferViews = AZStd::array_view<AZ::RHI::StreamBufferView>(	pkfxDrawCall.m_VertexInputs.RawDataPointer(),
																						pkfxDrawCall.m_VertexInputs.Count());
#endif

		// TODO: set this depending on lit state.
		materialDr.m_stencilRef = (AZ::Render::StencilRefs::UseIBLSpecularPass | AZ::Render::StencilRefs::UseDiffuseGIPass);

		materialDr.m_sortKey = sortKey;
		dpBuilder.AddDrawItem(materialDr);
	}

#if PK_O3DE_MAJOR_VERSION >= 2205
	AZStd::span<const AZ::RHI::StreamBufferView>	depthVtxInput;
#else
	AZStd::array_view<AZ::RHI::StreamBufferView>	depthVtxInput;
#endif

	if (pkfxDrawCall.m_RendererType == Renderer_Billboard ||
		pkfxDrawCall.m_RendererType == Renderer_Mesh)
	{
#if PK_O3DE_MAJOR_VERSION >= 2205
		depthVtxInput = AZStd::span<const AZ::RHI::StreamBufferView>(	pkfxDrawCall.m_VertexInputs.RawDataPointer(),
																		pkfxDrawCall.m_VertexInputs.Count());
#else
		depthVtxInput = AZStd::array_view<AZ::RHI::StreamBufferView>(	pkfxDrawCall.m_VertexInputs.RawDataPointer(),
																		pkfxDrawCall.m_VertexInputs.Count());
#endif
	}
	else
	{
#if PK_O3DE_MAJOR_VERSION >= 2205
		depthVtxInput = AZStd::span<const AZ::RHI::StreamBufferView>(	pkfxDrawCall.m_VertexInputs.RawDataPointer(),
																		1);
#else
		depthVtxInput = AZStd::array_view<AZ::RHI::StreamBufferView>(	pkfxDrawCall.m_VertexInputs.RawDataPointer(),
																		1);
#endif
	}

	if (pkfxDrawCall.m_OpaqueDepthPipelineState != null)
	{
		AZ::RHI::DrawPacketBuilder::DrawRequest	opaqueDepthDr;
		opaqueDepthDr.m_listTag = pkfxDrawCall.m_OpaqueDepthDrawList;
		opaqueDepthDr.m_pipelineState = pkfxDrawCall.m_OpaqueDepthPipelineState.get();
		opaqueDepthDr.m_streamBufferViews = depthVtxInput;
		opaqueDepthDr.m_sortKey = sortKey;
		dpBuilder.AddDrawItem(opaqueDepthDr);
	}
	else if (	pkfxDrawCall.m_TransparentDepthMinPipelineState != null &&
				pkfxDrawCall.m_TransparentDepthMaxPipelineState != null)
	{
		AZ::RHI::DrawPacketBuilder::DrawRequest	transparentDepthMinDr;
		transparentDepthMinDr.m_listTag = pkfxDrawCall.m_TransparentDepthMinDrawList;
		transparentDepthMinDr.m_pipelineState = pkfxDrawCall.m_TransparentDepthMinPipelineState.get();
		transparentDepthMinDr.m_streamBufferViews = depthVtxInput;
		transparentDepthMinDr.m_sortKey = sortKey;
		dpBuilder.AddDrawItem(transparentDepthMinDr);

		AZ::RHI::DrawPacketBuilder::DrawRequest	transparentDepthMaxDr;
		transparentDepthMaxDr.m_listTag = pkfxDrawCall.m_TransparentDepthMaxDrawList;
		transparentDepthMaxDr.m_pipelineState = pkfxDrawCall.m_TransparentDepthMaxPipelineState.get();
		transparentDepthMaxDr.m_streamBufferViews = depthVtxInput;
		transparentDepthMaxDr.m_sortKey = sortKey;
		dpBuilder.AddDrawItem(transparentDepthMaxDr);
	}

	AZ::Data::Instance<AZ::RPI::ShaderResourceGroup>	objectSrg = pkfxDrawCall.m_ObjectSrg;

	if (objectSrg != null)
	{
		// retrieve probe constant indices
		AZ::RHI::ShaderInputConstantIndex	modelToWorldConstantIndex = objectSrg->FindShaderInputConstantIndex(AZ::Name("m_reflectionProbeData.m_modelToWorld"));
		AZ_Error("MeshDataInstance", modelToWorldConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

		AZ::RHI::ShaderInputConstantIndex	modelToWorldInverseConstantIndex = objectSrg->FindShaderInputConstantIndex(AZ::Name("m_reflectionProbeData.m_modelToWorldInverse"));
		AZ_Error("MeshDataInstance", modelToWorldInverseConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

		AZ::RHI::ShaderInputConstantIndex	outerObbHalfLengthsConstantIndex = objectSrg->FindShaderInputConstantIndex(AZ::Name("m_reflectionProbeData.m_outerObbHalfLengths"));
		AZ_Error("MeshDataInstance", outerObbHalfLengthsConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

		AZ::RHI::ShaderInputConstantIndex	innerObbHalfLengthsConstantIndex = objectSrg->FindShaderInputConstantIndex(AZ::Name("m_reflectionProbeData.m_innerObbHalfLengths"));
		AZ_Error("MeshDataInstance", innerObbHalfLengthsConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

		AZ::RHI::ShaderInputConstantIndex	useReflectionProbeConstantIndex = objectSrg->FindShaderInputConstantIndex(AZ::Name("m_reflectionProbeData.m_useReflectionProbe"));
		AZ_Error("MeshDataInstance", useReflectionProbeConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

		AZ::RHI::ShaderInputConstantIndex	useParallaxCorrectionConstantIndex = objectSrg->FindShaderInputConstantIndex(AZ::Name("m_reflectionProbeData.m_useParallaxCorrection"));
		AZ_Error("MeshDataInstance", useParallaxCorrectionConstantIndex.IsValid(), "Failed to find ReflectionProbe constant index");

		// retrieve probe cubemap index
		AZ::Name	reflectionCubeMapImageName = AZ::Name("m_reflectionProbeCubeMap");
		AZ::RHI::ShaderInputImageIndex reflectionCubeMapImageIndex = objectSrg->FindShaderInputImageIndex(reflectionCubeMapImageName);
		AZ_Error("MeshDataInstance", reflectionCubeMapImageIndex.IsValid(), "Failed to find shader image index [%s]", reflectionCubeMapImageName.GetCStr());

#if PK_O3DE_MAJOR_VERSION == 2111
//See : https://github.com/o3de/o3de/pull/7189 and https://github.com/o3de/o3de/issues/7434
		AZ::Render::ReflectionProbeFeatureProcessor	*reflectionProbeFeatureProcessor = GetParentScene()->GetFeatureProcessor<AZ::Render::ReflectionProbeFeatureProcessor>();

		AZ::Render::ReflectionProbeFeatureProcessor::ReflectionProbeVector	reflectionProbes;
		reflectionProbeFeatureProcessor->FindReflectionProbes(AZ::Vector3(pkfxDrawCall.m_BoundingBox.Center().x(), pkfxDrawCall.m_BoundingBox.Center().y(), pkfxDrawCall.m_BoundingBox.Center().z()), reflectionProbes);

		if (!reflectionProbes.empty() && reflectionProbes[0])
		{
			objectSrg->SetConstant(modelToWorldConstantIndex, reflectionProbes[0]->GetTransform());
			objectSrg->SetConstant(modelToWorldInverseConstantIndex, AZ::Matrix3x4::CreateFromTransform(reflectionProbes[0]->GetTransform()).GetInverseFull());
			objectSrg->SetConstant(outerObbHalfLengthsConstantIndex, reflectionProbes[0]->GetOuterObbWs().GetHalfLengths());
			objectSrg->SetConstant(innerObbHalfLengthsConstantIndex, reflectionProbes[0]->GetInnerObbWs().GetHalfLengths());
			objectSrg->SetConstant(useReflectionProbeConstantIndex, true);
			objectSrg->SetConstant(useParallaxCorrectionConstantIndex, reflectionProbes[0]->GetUseParallaxCorrection());

			objectSrg->SetImage(reflectionCubeMapImageIndex, reflectionProbes[0]->GetCubeMapImage());
		}
		else
#endif
		{
			objectSrg->SetConstant(useReflectionProbeConstantIndex, false);
		}
		if (!objectSrg->IsQueuedForCompile())
			objectSrg->Compile();
		dpBuilder.AddShaderResourceGroup(objectSrg->GetRHIShaderResourceGroup());
	}

	return dpBuilder.End();
}

void	CPopcornFXFeatureProcessor::Init(CParticleMediumCollection *medCol, const SSceneViews *views)
{
	m_MediumCollection = medCol;
	m_SceneViews = views;
}

//----------------------------------------------------------------------------
}

#endif
