//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSkinnedMesh.h"

#if defined(O3DE_USE_PK)

#include <pk_maths/include/pk_maths_simd.h>

#if defined(PK_USE_EMOTIONFX)
#include <Source/Integration/Rendering/RenderActor.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshInputBuffers.h>
#include <EMotionFX/Source/Actor.h>
#include <EMotionFX/Source/ActorInstance.h>
#include <EMotionFX/Source/Mesh.h>
#include <EMotionFX/Source/SubMesh.h>
#include <EMotionFX/Source/SkinningInfoVertexAttributeLayer.h>
#include <EMotionFX/Source/TransformData.h>
#endif //PK_USE_EMOTIONFX

// Copied from ActorAsset.cpp
namespace
{
	const uint32_t MaxSupportedSkinInfluences = 4;
}

namespace PopcornFX {

#if defined(PK_USE_EMOTIONFX)

static bool IsVertexCountWithinSupportedRange(size_t vertexOffset, size_t vertexCount)
{
	return vertexOffset + vertexCount <= aznumeric_cast<size_t>(AZ::Render::SkinnedMeshVertexStreamPropertyInterface::Get()->GetMaxSupportedVertexCount());
}

static void ProcessSkinInfluences(	const EMotionFX::Mesh *mesh,
									const EMotionFX::SubMesh *subMesh,
									PopcornFX::TArray<u32> &blendIndexBufferData,
									PopcornFX::TArray<float> &blendWeightBufferData)
{
	EMotionFX::SkinningInfoVertexAttributeLayer	*sourceSkinningInfo = static_cast<EMotionFX::SkinningInfoVertexAttributeLayer*>(mesh->FindSharedVertexAttributeLayer(EMotionFX::SkinningInfoVertexAttributeLayer::TYPE_ID));

	const uint32_t	*sourceOriginalVertex = static_cast<uint32_t*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_ORGVTXNUMBERS));
	const uint32_t	vertexCount = subMesh->GetNumVertices();
	const uint32_t	vertexStart = subMesh->GetStartVertex();
	for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		const uint32_t	originalVertex = sourceOriginalVertex[vertexIndex + vertexStart];
		const uint32_t	influenceCount = AZStd::GetMin<uint32_t>(MaxSupportedSkinInfluences, static_cast<uint32_t>(sourceSkinningInfo->GetNumInfluences(originalVertex)));
		uint32_t		influenceIndex = 0;
		//float weightError = 1.0f;

		for (; influenceIndex < influenceCount; ++influenceIndex)
		{
			const uint32_t				i = vertexIndex * MaxSupportedSkinInfluences + influenceIndex;
			EMotionFX::SkinInfluence	*influence = sourceSkinningInfo->GetInfluence(originalVertex, influenceIndex);
			blendIndexBufferData[i] = static_cast<uint32_t>(influence->GetNodeNr());
			blendWeightBufferData[i] = influence->GetWeight();
			//weightError -= blendWeightBufferData[i];
		}

		// Zero out any unused ids/weights
		for (; influenceIndex < MaxSupportedSkinInfluences; ++influenceIndex)
		{
			const uint32_t	i = vertexIndex * MaxSupportedSkinInfluences + influenceIndex;
			blendIndexBufferData[i] = 0;
			blendWeightBufferData[i] = 0.0f;
		}
	}
}

bool	CPopcornFXSkinnedMesh::Init(const EMotionFX::Actor *actor, AZ::u32 lodIndex, AZ::u32 subMeshIndex, AZ::u32 vertexCount, const CMeshVStream &vstream, u32 samplingChannels)
{
	// Populate bone influences.
	PopcornFX::TArray<u32>		srcPackedBoneIndices;
	PopcornFX::TArray<float>	srcPackedWeights;

	if (PK_VERIFY(actor != null))
	{
		if (!PK_VERIFY(srcPackedWeights.Resize(MaxSupportedSkinInfluences * vertexCount)) ||
			!PK_VERIFY(srcPackedBoneIndices.Resize(MaxSupportedSkinInfluences * vertexCount)))
			return false;

		// GetNumNodes returns the number of 'joints' or 'bones' in the skeleton
		const size_t	numJoints = actor->GetNumNodes();

		for (size_t jointIndex = 0; jointIndex < numJoints; ++jointIndex)
		{
			const EMotionFX::Mesh	*mesh = actor->GetMesh(lodIndex, jointIndex);
			if (!mesh || mesh->GetIsCollisionMesh())
			{
				continue;
			}

			const size_t	numSubMeshes = mesh->GetNumSubMeshes();

			if (subMeshIndex >= numSubMeshes)
			{
				AZ_Error("PopcornFX", false, "Invalid sub-mesh index for sampler shape");
				return false;
			}

			const EMotionFX::SubMesh	*subMesh = mesh->GetSubMesh(subMeshIndex);
			const AZ::u32				subMeshVertexCount = subMesh->GetNumVertices();

			// Skip empty sub-meshes and sub-meshes that would put the total vertex count beyond the supported range
			if (subMeshVertexCount <= 0 && !IsVertexCountWithinSupportedRange(0, subMeshVertexCount))
			{
				AZ_Error("PopcornFX", false, "Sub-mesh is empty or beyond supported range for sampler shape");
				return false;
			}

			ProcessSkinInfluences(mesh, subMesh, srcPackedBoneIndices, srcPackedWeights);
			//We need only one mesh
			break;
		}

		m_SamplingChannels = samplingChannels;
		if (PostInit(vstream, static_cast<u32>(numJoints), vertexCount, srcPackedWeights, srcPackedBoneIndices))
			return true;
	}

	AZ_Error("PopcornFX", false, "Unable to create skinned mesh for sampler shape");
	return false;
}

//----------------------------------------------------------------------------

void	CPopcornFXSkinnedMesh::UpdateSkinning(EMotionFX::ActorInstance *actorInstance, float dt)
{
	const EMotionFX::TransformData	*transformData = actorInstance->GetTransformData();

	const AZ::u32	boneCount = m_SkeletonView->BoneCount();

	if (!PK_VERIFY(transformData != null) ||
		!PK_VERIFY(boneCount == transformData->GetNumTransforms()))
		return;

	const AZ::Matrix3x4	*globalMatrices = transformData->GetSkinningMatrices();
	for (uint32 iBone = 0; iBone < boneCount; ++iBone)
	{
		globalMatrices[iBone].StoreToColumnMajorFloat16((float*)&m_BoneInverseMatrices[iBone]);
		m_BoneInverseMatrices[iBone].XAxis().w() = 0.0f;
		m_BoneInverseMatrices[iBone].YAxis().w() = 0.0f;
		m_BoneInverseMatrices[iBone].ZAxis().w() = 0.0f;
		m_BoneInverseMatrices[iBone].WAxis().w() = 1.0f;
	}

	PostUpdateSkinnig(dt);
}
#endif

//----------------------------------------------------------------------------

bool	CPopcornFXSkinnedMesh::PostInit(const CMeshVStream &vstream, u32 boneCount, u32 vertexCount,
										const PopcornFX::TArray<float> &srcPackedWeights, const PopcornFX::TArray<u32> &srcPackedBoneIndices)
{
	m_SrcPositions = vstream.Positions();

	if (!PK_VERIFY(m_DstPositionsData.Resize(vertexCount)) ||
		!PK_VERIFY(m_OldPositionsData.Resize(vertexCount)))
		return false;

	for (u32 i = 0; i < vertexCount; ++i)
	{
		m_DstPositionsData[i] = m_SrcPositions[i].xyz0();
	}
	m_DstPositions = TStridedMemoryView<CFloat3>(reinterpret_cast<CFloat3*>(m_DstPositionsData.RawDataPointer()),
		m_DstPositionsData.Count(), 16);

	m_OldPositions = TStridedMemoryView<CFloat3>(reinterpret_cast<CFloat3*>(m_OldPositionsData.RawDataPointer()),
		m_OldPositionsData.Count(), 16);

	if (m_SamplingChannels & Channel_Velocity)
	{
		if (!PK_VERIFY(m_VelocitiesData.Resize(vertexCount)))
			return false;
		Mem::Clear(m_VelocitiesData.RawDataPointer(), m_VelocitiesData.CoveredBytes());
		m_Velocities = TStridedMemoryView<CFloat3>(reinterpret_cast<CFloat3*>(m_VelocitiesData.RawDataPointer()),
			m_VelocitiesData.Count(), 16);
	}
	else
	{
		m_VelocitiesData.Clear();
		m_Velocities = TStridedMemoryView<CFloat3>();
	}

	if (m_SamplingChannels & Channel_Normal)
	{
		if (!PK_VERIFY(m_NormalsData.Resize(vertexCount)))
			return false;
		const TStridedMemoryView<const CFloat3>	srcNormals = vstream.Normals();
		m_Normals = TStridedMemoryView<CFloat3>(reinterpret_cast<CFloat3*>(m_NormalsData.RawDataPointer()),
			m_NormalsData.Count(), 16);
		for (u32 i = 0; i < vertexCount; ++i)
		{
			m_NormalsData[i] = srcNormals[i].xyz0();
		}
	}
	else
	{
		m_NormalsData.Clear();
		m_Normals = TStridedMemoryView<CFloat3>();
	}

	if (m_SamplingChannels & Channel_Tangent)
	{
		if (!PK_VERIFY(m_TangentsData.Resize(vertexCount)))
			return false;
		const TStridedMemoryView<const CFloat4>	srcTangents = vstream.Tangents();
		m_Tangents = TStridedMemoryView<CFloat4>(m_TangentsData.RawDataPointer(), m_TangentsData.Count(), 16);
		for (u32 i = 0; i < vertexCount; ++i)
		{
			m_TangentsData[i] = srcTangents[i];
		}
	}
	else
	{
		m_TangentsData.Clear();
		m_Tangents = TStridedMemoryView<CFloat4>();
	}

	m_SkinningStreams = CBaseSkinningStreams::BuildFromUnpackedStreams(vertexCount, srcPackedWeights, srcPackedBoneIndices, TMemoryView<const u32>(), boneCount - 1);
	if (!PK_VERIFY(m_SkinningStreams != null))
		return false;

	// init skeleton
	if (!PK_VERIFY(m_BoneMatrices.Resize(boneCount)) ||
		!PK_VERIFY(m_BoneInverseMatrices.Resize(boneCount)))
		return false;

	m_SkeletonView = PK_NEW(CSkeletonView(boneCount,
		m_BoneMatrices.RawDataPointer(),
		m_BoneInverseMatrices.RawDataPointer()));
	if (!PK_VERIFY(m_SkeletonView != null))
		return false;

	//init skin context
	m_SkinContext.m_SkinningStreams = m_SkinningStreams;
	m_SkinContext.m_SrcPositions = m_SrcPositions;
	m_SkinContext.m_DstPositions = m_DstPositions;

	m_Override.m_PositionsOverride = m_DstPositions;
	m_Override.m_VelocitiesOverride = m_Velocities;
	m_Override.m_NormalsOverride = m_Normals;
	m_Override.m_TangentsOverride = m_Tangents;

	m_FirstFrameAfterLoad = true;
	return true;
}

//----------------------------------------------------------------------------

void	CPopcornFXSkinnedMesh::PostUpdateSkinnig(float dt)
{
	if (m_SamplingChannels & Channel_Velocity)
	{
		m_SkinDt = dt;

		// hook our pre-skin callback where we'll copy the positions to m_OldPositions so that the skin job can correctly
		// differentiate the two and compute the instantaneous mesh surface velocities using 'asyncSkinContext.m_SrcDt'
		m_SkinContext.m_CustomProcess_PreSkin = SSkinContext::CbCustomProcess(this, &CPopcornFXSkinnedMesh::_Skin_PreProcess);
		m_SkinContext.m_CustomProcess_PostSkin = SSkinContext::CbCustomProcess(this, &CPopcornFXSkinnedMesh::_Skin_PostProcess);
	}

	// Launch skinning tasks
	CSkeletalSkinnerSimple::AsyncSkinStart(m_AsyncSkinContext, *m_SkeletonView, m_SkinContext);

	// Do something while waiting for the skinning to finish
	// ...

	// Wait for skinning tasks to end
	CSkeletalSkinnerSimple::AsyncSkinWait(m_AsyncSkinContext, null);

	if ((m_SamplingChannels & Channel_Velocity) &&
		m_FirstFrameAfterLoad &&
		m_Velocities.Count() > 0)
	{
		Mem::Clear(m_Velocities.Data(), m_Velocities.CoveredBytes());
		m_FirstFrameAfterLoad = false;
	}
}

//----------------------------------------------------------------------------

void	CPopcornFXSkinnedMesh::_Skin_PreProcess(u32 vertexStart, u32 vertexCount, const SSkinContext &ctx)
{
	AZ_UNUSED(ctx);

	TStridedMemoryView<const CFloat3>	src = m_DstPositions.Slice(vertexStart, vertexCount);
	TStridedMemoryView<CFloat3>			dst = m_OldPositions.Slice(vertexStart, vertexCount);

	PK_ASSERT(src.Stride() == 0x10 && dst.Stride() == 0x10);
	Mem::Copy(dst.Data(), src.Data(), dst.Count() * dst.Stride());
}

//----------------------------------------------------------------------------

void	CPopcornFXSkinnedMesh::_Skin_PostProcess(u32 vertexStart, u32 vertexCount, const SSkinContext &ctx)
{
	// compute instantaneous surface velocities IFN:
	TStridedMemoryView<const CFloat3>	posCur = ctx.m_DstPositions.Slice(vertexStart, vertexCount);
	TStridedMemoryView<const CFloat3>	posOld = m_OldPositions.Slice(vertexStart, vertexCount);
	TStridedMemoryView<CFloat3>			vel = m_Velocities.Slice(vertexStart, vertexCount);
	const float							dt = m_SkinDt;

	//const bool	continuousAnim = true;//(m_Skeleton != null) && (m_Skeleton->LastUpdateFrameID() == m_LastSkinnedSkeletonFrameID + 1);
	//if (!continuousAnim && !vel.Empty())
	//{
	//	Mem::Clear(vel.Data(), vel.CoveredBytes());
	//}
	/*else*/ if (!posCur.Empty() && !posOld.Empty() && !vel.Empty())
	{
		PK_ASSERT(posCur.Stride() == 0x10);
		PK_ASSERT(posOld.Stride() == 0x10);
		PK_ASSERT(vel.Stride() == 0x10);

		CFloat3			* __restrict dstVel = vel.Data();
		const CFloat3	*dstVelEnd = Mem::AdvanceRawPointer(dstVel, vertexCount * 0x10);
		const CFloat3	*srcPosCur = posCur.Data();
		const CFloat3	*srcPosOld = posOld.Data();

		PK_ASSERT(Mem::IsAligned<0x10>(dstVel));
		PK_ASSERT(Mem::IsAligned<0x10>(dstVelEnd));
		PK_ASSERT(Mem::IsAligned<0x10>(srcPosCur));
		PK_ASSERT(Mem::IsAligned<0x10>(srcPosOld));

		const SIMD::Float4	invDt = SIMD::Float4(1.0f / dt);
		dstVelEnd = Mem::AdvanceRawPointer(dstVelEnd, -0x40);
		while (dstVel < dstVelEnd)
		{
			const SIMD::Float4	pA0 = SIMD::Float4::LoadAligned16(srcPosOld, 0x00);
			const SIMD::Float4	pA1 = SIMD::Float4::LoadAligned16(srcPosOld, 0x10);
			const SIMD::Float4	pA2 = SIMD::Float4::LoadAligned16(srcPosOld, 0x20);
			const SIMD::Float4	pA3 = SIMD::Float4::LoadAligned16(srcPosOld, 0x30);
			const SIMD::Float4	pB0 = SIMD::Float4::LoadAligned16(srcPosCur, 0x00);
			const SIMD::Float4	pB1 = SIMD::Float4::LoadAligned16(srcPosCur, 0x10);
			const SIMD::Float4	pB2 = SIMD::Float4::LoadAligned16(srcPosCur, 0x20);
			const SIMD::Float4	pB3 = SIMD::Float4::LoadAligned16(srcPosCur, 0x30);
			const SIMD::Float4	v0 = (pB0 - pA0) * invDt;
			const SIMD::Float4	v1 = (pB1 - pA1) * invDt;
			const SIMD::Float4	v2 = (pB2 - pA2) * invDt;
			const SIMD::Float4	v3 = (pB3 - pA3) * invDt;
			v0.StoreAligned16(dstVel, 0x00);
			v1.StoreAligned16(dstVel, 0x10);
			v2.StoreAligned16(dstVel, 0x20);
			v3.StoreAligned16(dstVel, 0x30);

			dstVel = Mem::AdvanceRawPointer(dstVel, 0x40);
			srcPosCur = Mem::AdvanceRawPointer(srcPosCur, 0x40);
			srcPosOld = Mem::AdvanceRawPointer(srcPosOld, 0x40);
		}
		dstVelEnd = Mem::AdvanceRawPointer(dstVelEnd, +0x40);

		while (dstVel < dstVelEnd)
		{
			const SIMD::Float4	pA = SIMD::Float4::LoadAligned16(srcPosOld);
			const SIMD::Float4	pB = SIMD::Float4::LoadAligned16(srcPosCur);
			const SIMD::Float4	v = (pB - pA) * invDt;
			v.StoreAligned16(dstVel);

			dstVel = Mem::AdvanceRawPointer(dstVel, 0x10);
			srcPosCur = Mem::AdvanceRawPointer(srcPosCur, 0x10);
			srcPosOld = Mem::AdvanceRawPointer(srcPosOld, 0x10);
		}
	}
}

}

#endif //O3DE_USE_PK
