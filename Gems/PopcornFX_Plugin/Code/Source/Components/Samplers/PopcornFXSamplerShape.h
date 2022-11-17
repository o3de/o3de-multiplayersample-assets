//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXSkinnedMesh.h"

#include <AzCore/Component/TransformBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>

#if defined(PK_USE_EMOTIONFX)
#include <Integration/ActorComponentBus.h>
#endif //PK_USE_EMOTIONFX

#include "Integration/PopcornFXIntegrationBus.h"

#if defined(O3DE_USE_PK)
#include <AzCore/Component/TickBus.h>
#include <pk_particles/include/ps_samplers_shape.h>
#include <pk_geometrics/include/ge_shapes.h>
#include <pk_geometrics/include/ge_mesh_resource.h>
#endif //O3DE_USE_PK

#if defined(PK_USE_EMOTIONFX)
namespace EMotionFX
{
	class ActorInstance;
}
#endif //PK_USE_EMOTIONFX

namespace PopcornFX {

	class PopcornFXSamplerShape
		: public AZ::Render::MeshComponentNotificationBus::Handler
		, public AZ::TransformNotificationBus::Handler
#if defined(O3DE_USE_PK)
		, public AZ::TickBus::Handler
#endif //O3DE_USE_PK
#if defined(PK_USE_EMOTIONFX)
		, private EMotionFX::Integration::ActorComponentNotificationBus::Handler
#endif //PK_USE_EMOTIONFX
		, public PopcornFXSamplerComponentRequestBus::Handler
{
	public:
		enum class EShapeType
		{
			BoxShape,
			SphereShape,
			CylinderShape,
			CapsuleShape,
			MeshShape,
			SkinnedMeshShape,
			ShapeType_Max
		};

		AZ_TYPE_INFO(PopcornFXSamplerShape, "{7435936D-23A2-41CF-A967-AAEE1093989A}")

		static void		Reflect(AZ::ReflectContext *context);

		void			CopyFrom(const PopcornFXSamplerShape &other);

		void			Activate();
		void			Deactivate();

		AZ::EntityId	m_AttachedToEntityId;

	private:
		static bool	VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);

	protected:
		AZ::u32			_OnShapeTypeChanged();
		AZ::u32			_OnValueChanged();
		AZ::u32			_OnShapeDimensionsChanged();
		void			_OnUseRelativeTransformChanged();
		void			_TriggerRebuildKdTree();
		void			_TriggerRebuildSamplingInfo();

		bool			m_UseRelativeTransform = true;
		EShapeType		m_ShapeType = EShapeType::BoxShape;
		float			m_ValueX = 1.0f;
		float			m_ValueY = 1.0f;
		float			m_ValueZ = 1.0f;
		AZStd::string	m_ValueXName;
		AZStd::string	m_ValueYName;
		AZStd::string	m_ValueZName;
		AZ::u32			m_LodIndex = 0;
		AZ::u32			m_SubMeshIndex = 0;
		bool			m_SampleNormals = false;
		bool			m_SampleTangents = false;
		bool			m_SampleUVs = false;
		bool			m_SampleColors = false;
		bool			m_SampleVelocities = false;
		bool			m_NeedBuildKdTree = false;
		bool			m_NeedBuildSamplingInfo = true;
		bool			m_ShowValueZ = false;
		bool			m_IsMesh = false;
		bool			m_IsSkinnedMesh = false;
		bool			m_IsShape = true;

#if defined(O3DE_USE_PK)
public:
		//////////////////////////////////////////////////////////////////////////
		// PopcornFXSamplerComponentRequestBus interface implementation
		PopcornFX::CParticleSamplerDescriptor	*GetDescriptor() override { return m_SamplerDescriptor.Get(); }
		virtual AZ::u32							GetType() override { return CParticleSamplerDescriptor_Shape::SamplerTypeID(); };
		//////////////////////////////////////////////////////////////////////////

		//AZ::Render::MeshComponentNotificationBus
		virtual void	OnModelReady(const AZ::Data::Asset<AZ::RPI::ModelAsset> &modelAsset, const AZ::Data::Instance<AZ::RPI::Model> &model) override;

		//////////////////////////////////////////////////////////////////////////
		// TransformNotificationBus
		//! Called when the local transform of the entity has changed. Local transform update always implies world transform change too.
		virtual void	OnTransformChanged(const AZ::Transform &local, const AZ::Transform &world) override;
		//////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// AZ::TickBus::Handler interface implementation
		virtual void	OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
#if defined(PK_USE_EMOTIONFX)
		// ActorComponentNotificationBus
		virtual void	OnActorInstanceCreated(EMotionFX::ActorInstance *actorInstance) override;
		virtual void	OnActorInstanceDestroyed(EMotionFX::ActorInstance *actorInstance) override;
#endif
		//////////////////////////////////////////////////////////////////////////

protected:
		PParticleSamplerDescriptor				m_SamplerDescriptor = null;
		CFloat4x4								m_CurrentMat = CFloat4x4::IDENTITY;
		CFloat4x4								m_PreviousMat = CFloat4x4::IDENTITY;
		PShapeDescriptor						m_ShapeDescriptor;
		PMeshNew								m_MeshNew;
		PPopcornFXSkinnedMesh					m_SkinnedMesh;

		AZ::Data::Asset<AZ::RPI::ModelAsset>	m_ModelAsset;
#if defined(PK_USE_EMOTIONFX)
		EMotionFX::ActorInstance				*m_ActorInstance = null;
#endif

		bool	_BuildDescriptor(const AZ::RPI::ModelLodAsset::Mesh *mesh = null);
		void	_SetMeshIFN();
		void	_Init();
		void	_RebuildKdTree(const PMeshNew &mesh);
		void	_RebuildSamplingInfo(const PMeshNew &mesh);
		void	_SetTransform();
		void	_LoadMesh();
		void	_LoadShape();
		void	_SetValuesNames();
		void	_CleanShapeDescriptor();
		void	_CleanMeshAssets();
#endif
	};

}
