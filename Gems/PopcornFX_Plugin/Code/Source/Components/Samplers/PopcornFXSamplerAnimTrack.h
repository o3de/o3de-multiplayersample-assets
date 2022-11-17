//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <LmbrCentral/Shape/SplineComponentBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/EntityId.h>

#include "Integration/PopcornFXIntegrationBus.h"

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_animtrack.h>
#include <pk_engine_utils/include/eu_samplers_curve.h>
#endif //O3DE_USE_PK

namespace PopcornFX {

	class PopcornFXSamplerAnimTrack
		: public LmbrCentral::SplineComponentNotificationBus::Handler
		, public AZ::TransformNotificationBus::Handler
		, public PopcornFXSamplerComponentRequestBus::Handler
	{
	public:
		AZ_TYPE_INFO(PopcornFXSamplerAnimTrack, "{0709E447-862A-4771-8EB3-6376F9A782F5}")

		static void	Reflect(AZ::ReflectContext *context);

		void		CopyFrom(const PopcornFXSamplerAnimTrack &other);

		void		Activate();
		void		Deactivate();

		AZ::EntityId	m_AttachedToEntityId;

	protected:
		void		_OnUseRelativeTransformChanged();

		bool		m_UseRelativeTransform = true;

	private:
		static bool VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);

#if defined(O3DE_USE_PK)
	public:
		//////////////////////////////////////////////////////////////////////////
		// PopcornFXSamplerComponentRequestBus interface implementation
		PopcornFX::CParticleSamplerDescriptor	*GetDescriptor() override { return m_SamplerDescriptor.Get(); }
		virtual AZ::u32							GetType() override { return CParticleSamplerDescriptor_AnimTrack::SamplerTypeID(); };
		//////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// LmbrCentral::SplineComponentNotificationBus::Handler interface implementationa
		virtual void	OnSplineChanged() override;
		////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// TransformNotificationBus
		//! Called when the local transform of the entity has changed. Local transform update always implies world transform change too.
		virtual void	OnTransformChanged(const AZ::Transform &local, const AZ::Transform &world) override;
		//////////////////////////////////////////////////////////////////////////

	protected:
		void	_Clean();
		void	_SetTransform();
		bool	_BuildDescriptor(AZ::ConstSplinePtr spline);


		PParticleSamplerDescriptor	m_SamplerDescriptor = null;
		CFloat4x4					m_CurrentMat = CFloat4x4::IDENTITY;
		CFloat4x4					m_CurrentMatUnscaled = CFloat4x4::IDENTITY;
		CCurveDescriptor			*m_Positions = null;
#endif //O3DE_USE_PK
};

}
