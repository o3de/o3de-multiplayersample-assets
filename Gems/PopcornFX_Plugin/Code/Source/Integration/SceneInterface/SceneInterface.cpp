//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "SceneInterface.h"

#if defined(O3DE_USE_PK)

#include <AzCore/Component/ComponentApplicationBus.h>

#if defined(PK_USE_PHYSX)
	#include <AzFramework/Physics/PhysicsScene.h>
	#include <AzFramework/Physics/ShapeConfiguration.h>
	#include <AzCore/std/smart_ptr/make_shared.h>
	#if RESOLVE_MATERIAL_PROPERTIES
	#include <AzFramework/Physics/Material.h>
	#endif
	#if RESOLVE_CONTACT_OBJECT
	#include <AzFramework/Physics/RigidBodyBus.h>
	#endif
#endif

#include "Integration/PopcornFXIntegrationBus.h"
#include "Integration/PopcornFXUtils.h"

namespace PopcornFX {

	void	CSceneInterface::RayTracePacket(const Colliders::STraceFilter	&traceFilter,
											const Colliders::SRayPacket		&packet,
											const Colliders::STracePacket	&results)
	{
#if defined(PK_USE_PHYSX)
		AzPhysics::SceneHandle	sceneHandle = AzPhysics::InvalidSceneHandle;
		auto					*sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

		if (sceneInterface != null)
		{
#if defined(POPCORNFX_EDITOR)
			if (!m_InGameMode)
				sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::EditorPhysicsSceneName);
			else
				sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
#else
			sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
#endif
		}

		if (!PK_VERIFY(sceneHandle != AzPhysics::InvalidSceneHandle))
			return;

		AzPhysics::CollisionGroup	collisionGroup(traceFilter.m_FilterFlags == 0 ? AzPhysics::CollisionGroup::All.GetMask() : traceFilter.m_FilterFlags);

		const u32	resCount = results.Count();
		const bool	emptyMasks = packet.m_RayMasks_Aligned16.Empty();
		const bool	emptySphereSweeps = packet.m_RaySweepRadii_Aligned16.Empty();

		for (u32 rayi = 0; rayi < resCount; ++rayi)
		{
			if (!emptyMasks && packet.m_RayMasks_Aligned16[rayi] == 0)
				continue;

			const CFloat4	&_rayDirAndLen = packet.m_RayDirectionsAndLengths_Aligned16[rayi];
			if (_rayDirAndLen.w() <= 0)
				continue;
			const CFloat3	&start = packet.m_RayOrigins_Aligned16[rayi].xyz();
			const CFloat3	&dir = _rayDirAndLen.xyz();

			// Perform raycast
			AzPhysics::SceneQueryHits	hitResult;
			if (emptySphereSweeps || packet.m_RaySweepRadii_Aligned16[rayi] == 0.0f)
			{
				AzPhysics::RayCastRequest	request;
				request.m_start = ToAZ(start);
				request.m_direction = ToAZ(dir);
				request.m_distance = _rayDirAndLen.w();
				request.m_collisionGroup = collisionGroup;

				hitResult = sceneInterface->QueryScene(sceneHandle, &request);
			}
			else
			{
				AzPhysics::ShapeCastRequest	request;
				request.m_distance = _rayDirAndLen.w();
				request.m_start = AZ::Transform::CreateTranslation(ToAZ(start));
				request.m_direction = ToAZ(dir);
				request.m_shapeConfiguration = AZStd::make_shared<Physics::SphereShapeConfiguration>(packet.m_RaySweepRadii_Aligned16[rayi]);
				request.m_collisionGroup = collisionGroup;

				hitResult = sceneInterface->QueryScene(sceneHandle, &request);
			}
			if (!hitResult.m_hits.empty())
			{
				//m_reportMultipleHits in AzPhysics::RayCastRequest and AzPhysics::ShapeCastRequest are set to false, only 1 hit possible
				const AzPhysics::SceneQueryHit	&hit = hitResult.m_hits[0];

				results.m_HitTimes_Aligned16[rayi] = hit.m_distance;
				if (results.m_ContactObjects_Aligned16 != null)
				{
#if RESOLVE_CONTACT_OBJECT
					Physics::RigidBody	*rigidbody = null;
					AZ::Entity			*entity = null;
					EBUS_EVENT_RESULT(entity, AZ::ComponentApplicationBus, FindEntity, hit.m_body->GetEntityId());
					if (entity)
					{
						Physics::RigidBodyRequestBus::EventResult(rigidbody, hit.m_body->GetEntityId(), &Physics::RigidBodyRequests::GetRigidBody);
					}
					if (rigidbody)
						results.m_ContactObjects_Aligned16[rayi] = rigidbody;
					else
#endif
						results.m_ContactObjects_Aligned16[rayi] = CollidableObject::DEFAULT;
				}
				if (results.m_ContactPoints_Aligned16 != null)
					results.m_ContactPoints_Aligned16[rayi].xyz() = ToPk(hit.m_position);
				if (results.m_ContactNormals_Aligned16 != null)
					results.m_ContactNormals_Aligned16[rayi].xyz() = ToPk(hit.m_normal);
#if RESOLVE_MATERIAL_PROPERTIES
				if (results.m_ContactSurfaces_Aligned16 != null)
				{
					AZ::Entity	*entity = null;
					EBUS_EVENT_RESULT(entity, AZ::ComponentApplicationBus, FindEntity, hit.m_entityId);
					if (entity)
					{
						Physics::RigidBody	*rigidbody;
						Physics::RigidBodyRequestBus::EventResult(rigidbody, hit.m_body->GetEntityId(), &Physics::RigidBodyRequests::GetRigidBody);
						if (rigidbody)
						{
							Physics::MaterialId	materialId = rigidbody->GetMaterialIdForShapeHierarchy(hit.m_hitShapeIdHierarchy);
							Physics::MaterialProperties	*matProperties = null;
							Physics::MaterialRequestBus::BroadcastResult(matProperties, &Physics::MaterialRequests::GetPhysicsMaterialProperties, materialId);
							results.m_ContactSurfaces_Aligned16[rayi] = matProperties;
						}
					}
				}
#endif //RESOLVE_MATERIAL_PROPERTIES
			}
		}
#endif
	}

#if RESOLVE_MATERIAL_PROPERTIES
void	CSceneInterface::ResolveContactMaterials(	const TMemoryView<void * const>					&contactObjects,
													const TMemoryView<void * const>					&contactSurfaces,
													const TMemoryView<Colliders::SSurfaceProperties>&outSurfaceProperties) const
{
	const u32	materialCount = contactSurfaces.Count();

	for (u32 iMaterial = 0; iMaterial < materialCount; ++iMaterial)
	{
		PopcornFX::Colliders::SSurfaceProperties	&surface = outSurfaceProperties[iMaterial];
		Physics::MaterialProperties	*matProperties = reinterpret_cast<Physics::MaterialProperties*>(contactSurfaces[iMaterial]);
		if (matProperties == null)
			continue;
		surface.m_Restitution = matProperties->m_restitution;
		surface.m_StaticFriction = matProperties->m_friction;
		surface.m_DynamicFriction = surface.m_StaticFriction;
		surface.m_SurfaceType = AZStd::hash<AZStd::string>{}(matProperties->m_name);
		surface.m_RestitutionCombineMode = Colliders::Combine_Average;
		surface.m_FrictionCombineMode = Colliders::Combine_Average;
	}
}
#endif //RESOLVE_MATERIAL_PROPERTIES

}

#endif //O3DE_USE_PK
