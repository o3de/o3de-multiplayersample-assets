//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <AzCore/base.h>
#include <pk_particles_toolbox/include/pt_mesh_deformers_skin.h>
#include <pk_geometrics/include/ge_mesh_resource.h>

#if defined(PK_USE_EMOTIONFX)
namespace EMotionFX
{
	class Actor;
	class ActorInstance;
}
#endif //PK_USE_EMOTIONFX

namespace PopcornFX {

	class CPopcornFXSkinnedMesh : public CRefCountedObject
	{
	public:
		enum	ESamplingChannels : int
		{
			Channel_Normal = 0x1,
			Channel_Tangent = 0x2,
			Channel_UV = 0x4,
			Channel_VertexColor = 0x8,
			Channel_Velocity = 0x10,
		};

		CPopcornFXSkinnedMesh()
		{
		}
		~CPopcornFXSkinnedMesh()
		{
			PK_SAFE_DELETE(m_SkinningStreams);
			PK_SAFE_DELETE(m_SkeletonView);
		}

#if defined(PK_USE_EMOTIONFX)
		bool	Init(const EMotionFX::Actor *actor, AZ::u32 lodIndex, AZ::u32 subMeshIndex, AZ::u32 vertexCount, const CMeshVStream &vstream, u32 samplingChannels);
		void	UpdateSkinning(EMotionFX::ActorInstance *actorInstance, float dt);
#endif
		bool	PostInit(	const CMeshVStream &vstream, u32 boneCount, u32 vertexCount,
							const PopcornFX::TArray<float> &srcPackedWeights, const PopcornFX::TArray<u32> &srcPackedBoneIndices);
		void	PostUpdateSkinnig(float dt);


		TStridedMemoryView<CFloat3>	&Velocities() { return m_Velocities; }
		SSamplerSourceOverride		*Override() { return &m_Override; }

	private:

		void	_Skin_PreProcess(u32 vertexStart, u32 vertexCount, const SSkinContext &ctx);
		void	_Skin_PostProcess(u32 vertexStart, u32 vertexCount, const SSkinContext &ctx);

		SSamplerSourceOverride							m_Override;
		TStridedMemoryView<const CFloat3>				m_SrcPositions;
		PopcornFX::TArray<CFloat4, TArrayAligned16>		m_DstPositionsData;
		TStridedMemoryView<CFloat3>						m_DstPositions;
		PopcornFX::TArray<CFloat4, TArrayAligned16>		m_OldPositionsData;
		TStridedMemoryView<CFloat3>						m_OldPositions;
		PopcornFX::TArray<CFloat4, TArrayAligned16>		m_VelocitiesData;
		TStridedMemoryView<CFloat3>						m_Velocities;
		PopcornFX::TArray<CFloat4, TArrayAligned16>		m_NormalsData;
		TStridedMemoryView<CFloat3>						m_Normals;
		PopcornFX::TArray<CFloat4, TArrayAligned16>		m_TangentsData;
		TStridedMemoryView<CFloat4>						m_Tangents;
		CBaseSkinningStreams							*m_SkinningStreams = null;
		PopcornFX::TArray<CFloat4x4, TArrayAligned16>	m_BindPoseMatrices;
		PopcornFX::TArray<CFloat4x4, TArrayAligned16>	m_BoneMatrices;
		PopcornFX::TArray<CFloat4x4, TArrayAligned16>	m_BoneInverseMatrices;
		float											m_SkinDt;
		CSkeletonView									*m_SkeletonView = null;
		CSkinAsyncContext								m_AsyncSkinContext;
		SSkinContext									m_SkinContext;
		bool											m_FirstFrameAfterLoad = false;
		u32												m_SamplingChannels = 0;
	};
	PK_DECLARE_REFPTRCLASS(PopcornFXSkinnedMesh);

}

#endif //O3DE_USE_PK
