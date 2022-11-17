//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/TickBus.h>

#include <pk_particles/include/ps_effect.h>

#include "Components/Attributes/PopcornFXAttributeList.h"

#if defined(_DEBUG) || defined(_PROFILE)
#define	PK_ATTRIB_ENABLE_CHECKS	1
#else
#define	PK_ATTRIB_ENABLE_CHECKS	0
#endif

namespace PopcornFX
{
	struct StandaloneEmitter;
}

namespace PopcornFX {

	template <typename _Scalar>
	struct IsFp
	{
		enum { Value = false };
	};

	template <>
	struct IsFp<float>
	{
		enum { Value = true };
	};

	class PopcornFXEmitter
		: public AZ::TransformNotificationBus::Handler
		, public AZ::TickBus::Handler
	{
	public:

		PopcornFXEmitter();
		virtual ~PopcornFXEmitter();

		void	Activate();
		void	Deactivate();

		//////////////////////////////////////////////////////////////////////////
		// TransformNotificationBus
		//! Called when the local transform of the entity has changed. Local transform update always implies world transform change too.
		virtual void	OnTransformChanged(const AZ::Transform &local, const AZ::Transform &world) override;
		//////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// AZ::TickBus::Handler interface implementation
		virtual void	OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		////////////////////////////////////////////////////////////////////////

		//! Attaches the component's entity to this emitter.
		void	AttachToEntity(AZ::EntityId id, bool needUpdate);

		//! Detaches the component's entity from this emitter.
		void	DetachFromEntity();

		//! Detaches the component's entity from this emitter.
		AZ::EntityId	GetAttachedEntity() { return m_AttachedToEntityId; }

		//! Set the transform if the emitter is not attached on an entity.
		void	SetTransform(AZ::Transform transform);

		//! Will find and load the effect associated with the provided emitter name
		void	Set(PopcornFXAsset *asset, PopcornFXAttributeList &attributeList);

		//! Removes the emitter.
		void	Clear();

		//! Start the emitter
		bool	Start();

		//! Stop the emitter
		bool	Stop();

		//! Stop the emitter
		bool	Terminate();

		//! Kill the emitter
		bool	Kill();

		//! Set Time Scale
		void	SetTimeScale(float timeScale);

		//! Set Prewarm Enable
		void	SetPrewarmEnable(bool enable);

		//! Set Prewarm Time
		void	SetPrewarmTime(float time);

		//! Set Visiblity
		void	SetVisible(bool visible);

		//! Set if the emitter will restart in the death notifier
		void	SetRestartOnDeath(bool restart);

		//! Get the number of attributes in the effect:
		AZ::u32	GetAttributesCount() const;

		//! Get the attribute name:
		AZStd::string	GetAttributeName(CGuid id) const;

		//! Get the attribute type:
		AZ::s32	GetAttributeType(CGuid id) const;

		//! Get the attribute Id.
		AZ::s32	GetAttributeId(const AZStd::string &name);

		//! Reset the attribute.
		bool	ResetAttribute(CGuid id);

		//! Get the attribute.
		bool	GetAttribute(CGuid id, SAttributesContainer::SAttrib &outAttrib);

		//! Set the attribute.
		template <typename _Scalar, u32 _Dim>
		bool	SetAttribute(CGuid id, const PopcornFX::TVector<_Scalar, _Dim> &value)
		{
			PK_SCOPEDPROFILE();
			if (m_AttributeList == null)
				return false;
			if (id < 0 || id >= m_AttributeList->AttributeCount())
			{
				AZ_Warning("PopcornFX", false, "SetAttribute failed (invalid index).");
				return false;
			}

			const CParticleAttributeDeclaration	*decl = _GetAttributeDeclaration(id);
			if (!PK_VERIFY(decl != null))
				return false;

#if PK_ATTRIB_ENABLE_CHECKS
			const PopcornFX::CBaseTypeTraits		&attrTraits = PopcornFX::CBaseTypeTraits::Traits((PopcornFX::EBaseTypeID)decl->ExportedType());

			if (_Dim > attrTraits.VectorDimension || IsFp<_Scalar>::Value != attrTraits.IsFp)
			{
				const char			*inType = (IsFp<_Scalar>::Value ? "Float" : "Int");
				const char			*attrType = (attrTraits.IsFp ? "Float" : "Int");
				AZ_Warning(	"PopcornFX", false, "SetAttribute: the Attribute [%d] \"%s\" cannot be set as %s %d: the attribute is %s %d",
							id, decl->ExportedName().Data(), inType, _Dim, attrType, attrTraits.VectorDimension);
				return false;
			}
#endif
			SAttributesContainer_SAttrib	attribValue;
			attribValue.m_Data32u[0] = 0;
			attribValue.m_Data32u[1] = 0;
			attribValue.m_Data32u[2] = 0;
			attribValue.m_Data32u[3] = 0;
			PK_STATIC_ASSERT(sizeof(attribValue) >= sizeof(value));
			reinterpret_cast<PopcornFX::TVector<_Scalar, _Dim>&>(attribValue) = value;
			decl->ClampToRangeIFN(attribValue);

			_SetAttribute(id, attribValue);
			return true;
		}

		AZ::u32				GetAttributeSamplersCount();
		//! Get the attribute sampler Id.
		AZ::s32				GetAttributeSamplerId(const AZStd::string &name);
		//! Set the attribute sampler.
		bool				SetAttributeSampler(AZ::u32 attribSamplerId, AZ::EntityId entityId);
		//! Get the attribute sampler.
		AZ::EntityId		GetAttributeSampler(AZ::u32 attribSamplerId);

		void				SetTeleportThisFrame();

		void				EnableAutoRemove(PopcornFX::StandaloneEmitter *emitter) { m_StandaloneEmitterForAutoRemove = emitter; }

		bool				IsPlaying() { return m_Emitter != null; }

		PParticleEffect		GetEffect() { return m_Asset == null ? null : m_Asset->m_Effect; }

		PParticleEffectInstance		GetEmitter() { return m_Emitter; }


		void				UpdateTransforms();

	protected:
		bool	_SpawnEmitter();
		void	_SetAttribute(CGuid id, const SAttributesContainer_SAttrib &value);
		void	_RefreshSamplers();
		bool	_SetSampler(AZ::u32 attribSamplerId, const PopcornFXSampler *sampler);
		void	_TransformLocalToWorld();
		void	_TransformWorldToLocal();
		void	_ApplyTransformChanged();

		void	_OnDeath();
		void	_OnDeathNotifier(const PParticleEffectInstance &effectInstance);

		const CParticleAttributeDeclaration	*_GetAttributeDeclaration(CGuid id);


		PopcornFXAttributeList					*m_AttributeList;
		AZ::EntityId							m_AttachedToEntityId;
		AZ::Transform							m_AttachedEntityTransform;
		AZ::Transform							m_Transform; //Used only if m_AttachedToEntityId is invalid
		bool									m_TransformChanged = false;
		bool									m_TeleportThisFrame = true;
		bool									m_RestartOnDeath = false;
		bool									m_WaitSamplersReady = false;

		bool									m_PrewarmEnable = false;
		float									m_PrewarmTime = 0.0f;

		PopcornFXAsset							*m_Asset = null;
		PParticleEffectInstance					m_Emitter;

		struct	SBufferedTransforms
		{
			CFloat4x4		m_CurTransform;			//	current world transform
			CFloat4x4		m_PrevTransform;		//	previous world transform
			CFloat3			m_WorldVel_Current;		//	current world velocity
			CFloat3			m_WorldVel_Previous;	//	previous world velocity

			SBufferedTransforms()
				:	m_CurTransform(CFloat4x4::IDENTITY)
				,	m_PrevTransform(CFloat4x4::IDENTITY)
				,	m_WorldVel_Current(0.0f)
				,	m_WorldVel_Previous(0.0f)
			{
			}
		};

		SBufferedTransforms						m_CurrentTransforms;
		SBufferedTransforms						m_NextTransforms;

		PopcornFX::StandaloneEmitter			*m_StandaloneEmitterForAutoRemove = null;
	};

}

#endif //O3DE_USE_PK
