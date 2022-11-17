//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXEmitter.h"

#if defined(O3DE_USE_PK)

#include <PopcornFX/PopcornFXBus.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/ComponentApplicationBus.h>

#include <pk_particles/include/ps_samplers_classes.h>
#include <pk_particles/include/ps_mediums.h>
#include <pk_particles/include/Storage/MainMemory/storage_ram.h>
#include <pk_particles_toolbox/include/pt_helpers.h>

#include "PopcornFXSystemComponent.h"

namespace PopcornFX {

	PopcornFXEmitter::PopcornFXEmitter()
	{
		m_AttributeList = null;
		m_AttachedToEntityId = AZ::EntityId(AZ::EntityId::InvalidEntityId);
		m_Transform = AZ::Transform::CreateIdentity();
	}

	PopcornFXEmitter::~PopcornFXEmitter()
	{
		Clear();
	}

	void	PopcornFXEmitter::Activate()
	{
		AZ::TickBus::Handler::BusConnect();
	}

	void	PopcornFXEmitter::Deactivate()
	{
		if (m_AttachedToEntityId.IsValid() && AZ::TransformNotificationBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
			AZ::TransformNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
		AZ::TickBus::Handler::BusDisconnect();
	}

	void	PopcornFXEmitter::OnTransformChanged(const AZ::Transform &/*local*/, const AZ::Transform &world)
	{
		m_AttachedEntityTransform = world;
		m_TransformChanged = true;
	}

	void	PopcornFXEmitter::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		(void)time;

		if (m_Emitter == null && m_RestartOnDeath)
		{
			m_RestartOnDeath = false;
			Start();
		}

		if (m_WaitSamplersReady && m_AttributeList != null && m_AttributeList->SamplersReady())
		{
			m_WaitSamplersReady = false;
			Start();
		}

		const CFloat3	prevPrevPos = m_NextTransforms.m_PrevTransform.StrippedTranslations(); // 2 frames of lag

		if (!m_TeleportThisFrame)
		{
			// Prev = Curr
			m_NextTransforms.m_PrevTransform = m_NextTransforms.m_CurTransform;
			m_NextTransforms.m_WorldVel_Previous = m_NextTransforms.m_WorldVel_Current;
		}

		if (m_TransformChanged)
		{
			// Update Position
			_ApplyTransformChanged();
			m_TransformChanged = false;
		}
		// Update Velocity
		if (!m_TeleportThisFrame && deltaTime > 1.0e-6f)
			m_NextTransforms.m_WorldVel_Current = PopcornFX::ParticleToolbox::PredictCurrentVelocity(m_NextTransforms.m_CurTransform.StrippedTranslations(), m_NextTransforms.m_PrevTransform.StrippedTranslations(), prevPrevPos, deltaTime);
		else
			m_NextTransforms.m_WorldVel_Current = CFloat3::ZERO;

		if (m_TeleportThisFrame)
		{
			// Make previous frame same as current
			m_NextTransforms.m_PrevTransform = m_NextTransforms.m_CurTransform;
			m_NextTransforms.m_WorldVel_Previous = m_NextTransforms.m_WorldVel_Current;
		}

		UpdateTransforms();

		// Always reset m_TeleportThisFrame
		if (m_TeleportThisFrame)
			m_TeleportThisFrame = false;
	}

	void	PopcornFXEmitter::_ApplyTransformChanged()
	{
		AZ::Transform	mat;

		if (m_AttachedToEntityId.IsValid())
			mat = m_AttachedEntityTransform * m_Transform;
		else
			mat = m_Transform;

		AZ::Matrix4x4	tr = AZ::Matrix4x4::CreateFromTransform(mat);
		Mem::Copy(&m_NextTransforms.m_CurTransform, &tr, sizeof(CFloat4x4));
		m_NextTransforms.m_CurTransform.Transpose();
		m_NextTransforms.m_CurTransform.XAxis().w() = 0.0f;
		m_NextTransforms.m_CurTransform.YAxis().w() = 0.0f;
		m_NextTransforms.m_CurTransform.ZAxis().w() = 0.0f;
		m_NextTransforms.m_CurTransform.WAxis().w() = 1.0f;
	}

	void	PopcornFXEmitter::AttachToEntity(AZ::EntityId id, bool needUpdate)
	{
		if (m_AttachedToEntityId.IsValid())
		{
			if (AZ::TransformNotificationBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
				AZ::TransformNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
			if (needUpdate)
				_TransformLocalToWorld();
		}
		m_AttachedToEntityId = id;
		if (m_AttachedToEntityId.IsValid())
		{
			if (needUpdate)
				_TransformWorldToLocal();
			if (!AZ::TransformNotificationBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
				AZ::TransformNotificationBus::Handler::BusConnect(m_AttachedToEntityId);
		}
	}

	void	PopcornFXEmitter::DetachFromEntity()
	{
		if (m_AttachedToEntityId.IsValid())
		{
			if (AZ::TransformNotificationBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
				AZ::TransformNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
			_TransformLocalToWorld();
		}
		m_AttachedToEntityId = AZ::EntityId();
		m_AttachedEntityTransform = AZ::Transform::Identity();
	}

	void	PopcornFXEmitter::SetTransform(AZ::Transform transform)
	{
		m_Transform = transform;
		m_TransformChanged = true;
	}

	void	PopcornFXEmitter::_TransformLocalToWorld()
	{
		AZ::Transform parentTransform;
		AZ::TransformBus::EventResult(parentTransform, m_AttachedToEntityId, &AZ::TransformBus::Events::GetWorldTM);
		m_Transform = parentTransform * m_Transform;
		m_TransformChanged = true;
	}

	void	PopcornFXEmitter::_TransformWorldToLocal()
	{
		AZ::Transform parentTransform;
		AZ::TransformBus::EventResult(parentTransform, m_AttachedToEntityId, &AZ::TransformBus::Events::GetWorldTM);
		m_Transform = parentTransform.GetInverse() * m_Transform;
		m_TransformChanged = true;
	}

	void	PopcornFXEmitter::Set(PopcornFXAsset *asset, PopcornFXAttributeList &attributeList)
	{
		if (asset != null)
		{
			m_Asset = asset;
			m_AttributeList = &attributeList;
		}
		else
		{
			Clear();
			AZ_Warning("PopcornFX", false, "Effect is null");
		}
	}

	bool	PopcornFXEmitter::_SpawnEmitter()
	{
		// If we already have an emitter, remove it to spawn a new one
		Terminate();

		// Get entity's world transforms
		if (m_AttachedToEntityId.IsValid())
		{
			EBUS_EVENT_ID_RESULT(m_AttachedEntityTransform, m_AttachedToEntityId, AZ::TransformBus, GetWorldTM);
		}

		_ApplyTransformChanged();
		m_NextTransforms.m_PrevTransform = m_NextTransforms.m_CurTransform;
		m_NextTransforms.m_WorldVel_Current = CFloat3(0.0f);
		m_NextTransforms.m_WorldVel_Previous = CFloat3(0.0f);
		UpdateTransforms();

		// Get the medium collection
		CParticleMediumCollection	*mediumCollection;
		PopcornFXIntegrationBus::BroadcastResult(mediumCollection, &PopcornFX::PopcornFXIntegrationBus::Handler::GetMediumCollection);

		// Spawn the particle emitter
		m_Emitter = m_Asset->m_Effect->Instantiate(mediumCollection);
		if (!PK_VERIFY(m_Emitter != null))
			return false;

		m_Emitter->m_DeathNotifier += FastDelegate<void(const PParticleEffectInstance &)>(this, &PopcornFXEmitter::_OnDeathNotifier);
		return true;
	}

	bool	PopcornFXEmitter::Start()
	{
		if (m_Asset == null)
			return false;

		if (m_AttributeList == null || !m_AttributeList->SamplersReady())
		{
			m_WaitSamplersReady = true;
			return false;
		}

		if (m_Emitter == null && !_SpawnEmitter())
			return false;

		// Setup the effect attributes
		SAttributesContainer	*container = m_AttributeList->AttributesContainer();
		m_Emitter->SetAllAttributes(container);
		container->Destroy();
		// Setup the effect attribute samplers
		_RefreshSamplers();

		PopcornFX::SEffectStartCtl	effectStartCtl;
		effectStartCtl.m_TimeFromStartOfFrame = 0.0f;	// TODO: Proper values !
		effectStartCtl.m_TimeToEndOfFrame = 0.0f;		// TODO: Proper values !
		effectStartCtl.m_SpawnTransformsPack.m_WorldTr_Current = &m_CurrentTransforms.m_CurTransform;
		effectStartCtl.m_SpawnTransformsPack.m_WorldTr_Previous = &m_CurrentTransforms.m_PrevTransform;
		effectStartCtl.m_SpawnTransformsPack.m_WorldVel_Current = &m_CurrentTransforms.m_WorldVel_Current;
		effectStartCtl.m_SpawnTransformsPack.m_WorldVel_Previous = &m_CurrentTransforms.m_WorldVel_Previous;
		if (m_PrewarmEnable)
			effectStartCtl.m_PrewarmTime = m_PrewarmTime;

		return m_Emitter->Start(effectStartCtl);
	}

	bool	PopcornFXEmitter::Stop()
	{
		if (m_Emitter != null)
		{
			m_Emitter->Stop();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitter::Terminate()
	{
		if (m_Emitter != null)
		{
			m_Emitter->Terminate();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitter::Kill()
	{
		if (m_Emitter != null)
		{
			m_Emitter->KillDeferred();
			return true;
		}
		return false;
	}

	void	PopcornFXEmitter::SetTimeScale(float timeScale)
	{
		if (m_Emitter != null)
			m_Emitter->SetTimeScale(timeScale);
	}

	void	PopcornFXEmitter::SetPrewarmEnable(bool enable)
	{
		m_PrewarmEnable = enable;
	}

	void	PopcornFXEmitter::SetPrewarmTime(float time)
	{
		m_PrewarmTime = time;
	}

	void	PopcornFXEmitter::SetVisible(bool visible)
	{
		if (m_Emitter != null)
			m_Emitter->SetVisible(visible);
	}

	void	PopcornFXEmitter::SetRestartOnDeath(bool restart)
	{
		m_RestartOnDeath = restart;
	}

	void	PopcornFXEmitter::_OnDeath()
	{
		if (m_Emitter != null)
			m_Emitter->m_DeathNotifier -= FastDelegate<void(const PParticleEffectInstance&)>(this, &PopcornFXEmitter::_OnDeathNotifier);
		if (m_StandaloneEmitterForAutoRemove != null)
		{
			EBUS_QUEUE_EVENT(PopcornFX::PopcornFXDestructionRequestBus, DestroyEffect, m_StandaloneEmitterForAutoRemove);
			m_StandaloneEmitterForAutoRemove = null;
		}
	}

	void	PopcornFXEmitter::_OnDeathNotifier(const PParticleEffectInstance &effectInstance)
	{
		if (PK_VERIFY(effectInstance == m_Emitter))
		{
			_OnDeath();
			m_Emitter = null;
		}
	}

	void	PopcornFXEmitter::Clear()
	{
		m_RestartOnDeath = false;
		m_WaitSamplersReady = false;
		if (m_Emitter != null)
		{
			_OnDeath();
			m_Emitter->KillDeferred();
			m_Emitter = null;
		}
		m_Asset = null;
	}

	AZ::u32	PopcornFXEmitter::GetAttributesCount() const
	{
		if (m_Asset != null && m_Asset->m_Effect->AttributeFlatList() != null)
			return m_Asset->m_Effect->AttributeFlatList()->AttributeList().Count();
		else
			return 0U;
	}

	AZStd::string	PopcornFXEmitter::GetAttributeName(CGuid id) const
	{
		if (m_Asset != null && m_Asset->m_Effect->AttributeFlatList() != null && id < m_Asset->m_Effect->AttributeFlatList()->AttributeList().Count())
			return m_Asset->m_Effect->AttributeFlatList()->AttributeList()[id]->ExportedName().Data();
		return "";
	}

	AZ::s32	PopcornFXEmitter::GetAttributeType(CGuid id) const
	{
		if (m_Asset->m_Effect != null && m_Asset->m_Effect->AttributeFlatList() != null && id < m_Asset->m_Effect->AttributeFlatList()->AttributeList().Count())
		{
			EBaseTypeID		typeId = static_cast<EBaseTypeID>(m_Asset->m_Effect->AttributeFlatList()->AttributeList()[id]->ExportedType());
			return static_cast<int32>(BaseTypeToO3DEPopcornFXType(typeId));
		}
		return static_cast<int32>(Type_Unknown);
	}

	AZ::s32	PopcornFXEmitter::GetAttributeId(const AZStd::string &name)
	{
		if (m_AttributeList == null)
			return -1;
		return m_AttributeList->GetAttributeId(name);
	}

	bool	PopcornFXEmitter::ResetAttribute(CGuid id)
	{
		if (m_Asset->m_Effect == null)
			return false;

		const CParticleAttributeDeclaration *decl = _GetAttributeDeclaration(id);
		if (decl == null)
		{
			AZ_Warning("PopcornFX", false, "ResetAttribute failed.");
			return false;
		}
		SAttributesContainer::SAttrib	attrib = decl->GetDefaultValue();
		_SetAttribute(id, attrib);
		return true;
	}

	bool	PopcornFXEmitter::GetAttribute(CGuid id, SAttributesContainer::SAttrib &outAttrib)
	{
		if (m_AttributeList == null)
			return false;

		if (id >= m_AttributeList->AttributeCount())
		{
			AZ_Warning("PopcornFX", false, "GetAttribute failed (invalid id).");
			return false;
		}
		if (m_Emitter != null)
		{
			const EBaseTypeID typeID = m_AttributeList->GetAttributeBaseType(id);
			if (!m_Emitter->GetRawAttribute(id, typeID, &outAttrib, true))
			{
				AZ_Warning("PopcornFX", false, "GetAttribute on instance failed.");
				return false;
			}
		}
		outAttrib = m_AttributeList->AttributeRawDataAttributes()[id];
		return true;
	}

	AZ::u32	PopcornFXEmitter::GetAttributeSamplersCount()
	{
		if (m_AttributeList == null)
			return 0;
		return m_AttributeList->SamplerCount();
	}

	AZ::s32	PopcornFXEmitter::GetAttributeSamplerId(const AZStd::string &name)
	{
		if (m_AttributeList == null)
			return -1;
		return m_AttributeList->GetAttributeSamplerId(name);
	}

	bool	PopcornFXEmitter::SetAttributeSampler(AZ::u32 attribSamplerId, AZ::EntityId entityId)
	{
		if (m_AttributeList == null)
			return false;

		if (IsPlaying())
		{
			AZ_Warning("PopcornFX", false, "SetAttributeSampler cannot be called on started emitters.");
			return false;
		}
		return m_AttributeList->SetSampler(attribSamplerId, entityId);
	}

	AZ::EntityId	PopcornFXEmitter::GetAttributeSampler(AZ::u32 attribSamplerId)
	{
		if (m_AttributeList != null)
		{
			PopcornFXSampler	*sampler = m_AttributeList->GetSampler(attribSamplerId);
			if (sampler != null)
			{
				return sampler->m_EntityId;
			}
		}
		return (AZ::EntityId)AZ::EntityId::InvalidEntityId;
	}

	void	PopcornFXEmitter::SetTeleportThisFrame()
	{
		m_TeleportThisFrame = true;
	}

	//----------------------------------------------------------------------------

	void	PopcornFXEmitter::UpdateTransforms()
	{
		m_CurrentTransforms = m_NextTransforms;
	}

	//----------------------------------------------------------------------------

	bool	PopcornFXEmitter::_SetSampler(AZ::u32 attribSamplerId, const PopcornFXSampler *sampler)
	{
		PParticleSamplerDescriptor	desc = null;
		AZ::u32						type = (AZ::u32)-1;
		EBUS_EVENT_ID_RESULT(type, sampler->m_EntityId, PopcornFX::PopcornFXSamplerComponentRequestBus, GetType);

		if (sampler->m_Type == type)
		{
			EBUS_EVENT_ID_RESULT(desc, sampler->m_EntityId, PopcornFX::PopcornFXSamplerComponentRequestBus, GetDescriptor);
		}
		else
		{
			AZ_Warning("PopcornFX", false, "SetSampler failed for sampler %s: invalid type.", sampler->m_Name.c_str());
			return false;
		}
		if (desc != null)
		{
			if (!m_Emitter->SetAttributeSampler(attribSamplerId, desc))
			{
				AZ_Warning("PopcornFX", false, "SetAttributeSampler failed.");
				return false;
			}
		}
		else
			return false;
		return true;
	}

	void	PopcornFXEmitter::_SetAttribute(CGuid id, const SAttributesContainer_SAttrib &value)
	{
		if (m_Emitter != null)
		{
			const EBaseTypeID typeID = m_AttributeList->GetAttributeBaseType(id);
			if (!m_Emitter->SetRawAttribute(id, typeID, &value, true))
			{
				AZ_Warning("PopcornFX", false, "SetAttribute failed on effect instance.");
			}
		}
		if (m_AttributeList != null)
			m_AttributeList->AttributeRawDataAttributes()[id] = value;
	}

	void	PopcornFXEmitter::_RefreshSamplers()
	{
		const AZStd::vector<PopcornFXSampler>	&samplers = m_AttributeList->Samplers();

		for (u32 i = 0; i < samplers.size(); ++i)
		{
			if (samplers[i].m_EntityId != (AZ::EntityId)AZ::EntityId::InvalidEntityId)
			{
				_SetSampler(i, &samplers[i]);
			}
			else
			{
				if (!m_Emitter->SetAttributeSampler(samplers[i].m_Name.c_str(), null))
				{
					AZ_Warning("PopcornFX", false, "SetAttributeSampler default failed.");
				}
			}
		}
	}

	const CParticleAttributeDeclaration	*PopcornFXEmitter::_GetAttributeDeclaration(CGuid id)
	{
		if (m_Asset == null)
			return null;

		const CParticleAttributeList	*defaultList = m_Asset->m_Effect->AttributeFlatList();
		if (defaultList == null)
			return null;
		if (id >= defaultList->UniqueAttributeList().Count())
		{
			AZ_Assert(false, "PopcornFX", "_GetAttributeDeclaration failed (invalid id).");
			return null;
		}
		return defaultList->UniqueAttributeList()[id];
	}


}

#endif //O3DE_USE_PK

