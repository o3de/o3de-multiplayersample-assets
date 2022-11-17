//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerAnimTrack.h"

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_classes.h>
#include <AzCore/Math/Spline.h>
#include "Integration/PopcornFXUtils.h"
#endif //O3DE_USE_PK

#include <AzCore/Math/Matrix4x4.h>

namespace PopcornFX {

	void	PopcornFXSamplerAnimTrack::Reflect(AZ::ReflectContext *context)
	{
		AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
		if (serializeContext)
		{
			serializeContext->Class<PopcornFXSamplerAnimTrack>()
				->Version(1, &VersionConverter)
				->Field("UseRelativeTransform", &PopcornFXSamplerAnimTrack::m_UseRelativeTransform)
				;

			// edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXSamplerAnimTrack>("PopcornFX Sampler AnimTrack", "")
					->DataElement(0, &PopcornFXSamplerAnimTrack::m_UseRelativeTransform, "UseRelativeTransform", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerAnimTrack::_OnUseRelativeTransformChanged)
					;
			}
		}
	}

	// Private Static
	bool	PopcornFXSamplerAnimTrack::VersionConverter(AZ::SerializeContext &context,
														AZ::SerializeContext::DataElementNode &classElement)
	{
		(void)context; (void)classElement;
		return true;
	}

#if !defined(O3DE_USE_PK)

	void	PopcornFXSamplerAnimTrack::Activate()
	{
	}

	void	PopcornFXSamplerAnimTrack::Deactivate()
	{
	}

	void	PopcornFXSamplerAnimTrack::_OnUseRelativeTransformChanged()
	{
	}

#else

	void	PopcornFXSamplerAnimTrack::Activate()
	{
		LmbrCentral::SplineComponentNotificationBus::Handler::BusConnect(m_AttachedToEntityId);
		AZ::TransformNotificationBus::Handler::BusConnect(m_AttachedToEntityId);
		PopcornFXSamplerComponentRequestBus::Handler::BusConnect(m_AttachedToEntityId);

		_SetTransform();
		OnSplineChanged();
	}

	void	PopcornFXSamplerAnimTrack::Deactivate()
	{
		_Clean();

		PopcornFXSamplerComponentRequestBus::Handler::BusDisconnect(m_AttachedToEntityId);
		AZ::TransformNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
		LmbrCentral::SplineComponentNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
	}

	void	PopcornFXSamplerAnimTrack::_OnUseRelativeTransformChanged()
	{
		_SetTransform();
	}

	void	PopcornFXSamplerAnimTrack::OnSplineChanged()
	{
		AZ::ConstSplinePtr	spline;
		LmbrCentral::SplineComponentRequestBus::EventResult(spline, m_AttachedToEntityId, &LmbrCentral::SplineComponentRequests::GetSpline);

		if (!PK_VERIFY(spline != null))
			return;

		if (!PK_VERIFY(_BuildDescriptor(spline)))
		{
			AZ_Error("PopcornFX", false, "Build descriptor failed for Sampler AnimTrack");
		}
	}

	void	PopcornFXSamplerAnimTrack::OnTransformChanged(const AZ::Transform &/*local*/, const AZ::Transform &/*world*/)
	{
		_SetTransform();
	}

	bool	PopcornFXSamplerAnimTrack::_BuildDescriptor(AZ::ConstSplinePtr spline)
	{
		if (m_SamplerDescriptor == null)
		{
			m_SamplerDescriptor = PK_NEW(CParticleSamplerDescriptor_AnimTrack_Default());
			if (!PK_VERIFY(m_SamplerDescriptor != null))
				return false;
		}
		PParticleSamplerDescriptor_AnimTrack_Default	desc = reinterpret_cast<CParticleSamplerDescriptor_AnimTrack_Default *>(m_SamplerDescriptor.Get());
		if (!PK_VERIFY(desc != null))
			return false;

		desc->m_TrackTransforms = &m_CurrentMat;
		desc->m_TrackTransformsUnscaled = &m_CurrentMatUnscaled;

		const bool							isLinear = azrtti_cast<const AZ::LinearSpline*>(spline.get()) != null;
		const bool							isLooping = spline->IsClosed();
		const bool							isCatmullRom = azrtti_cast<const AZ::CatmullRomSpline*>(spline.get()) != null;
		const AZStd::vector<AZ::Vector3>	&vertices = spline->GetVertices();
		const u32							keyCount = static_cast<u32>(spline->GetSegmentCount()) + 1;
		const float							splineLength = spline->GetSplineLength();
		const u32							catmullNotLoopingOffset = (!isLooping && isCatmullRom) ? 1 : 0;

		if (m_Positions == null)
			m_Positions = PK_NEW(PopcornFX::CCurveDescriptor());
		if (!PK_VERIFY(m_Positions != null))
			return false;
		m_Positions->m_Order = 3;

		if (isLinear)
			m_Positions->m_Interpolator = CInterpolableVectorArray::Interpolator_Linear;
		else
			m_Positions->m_Interpolator = PopcornFX::CInterpolableVectorArray::Interpolator_Hermite;

		if (!PK_VERIFY(m_Positions->Resize(keyCount)))
			return false;

		float	length = 0.0f;
		for (u32 i = 1; i < keyCount - 1; ++i)
		{
			length += spline->GetSegmentLength(i - 1 + catmullNotLoopingOffset);
			m_Positions->m_Times[i] = length / splineLength;
		}
		m_Positions->m_Times[0] = 0.0f;
		m_Positions->m_Times[keyCount - 1] = 1.0f;

		const u32	fillCount = isLooping ? keyCount - 1 : keyCount;
		CFloat3		*dstPos = reinterpret_cast<CFloat3*>(m_Positions->m_FloatValues.RawDataPointer());

		for (u32 i = catmullNotLoopingOffset; i < fillCount + catmullNotLoopingOffset; ++i)
			*dstPos++ = ToPk(vertices[i]);
		if (isLooping)
			*dstPos = ToPk(vertices[0]);

		if (!isLinear)
		{
			CFloat3	*dstTangents = reinterpret_cast<CFloat3*>(m_Positions->m_FloatTangents.RawDataPointer());

			if (const AZ::BezierSpline *bezierSpline = azrtti_cast<const AZ::BezierSpline*>(spline.get()))
			{
				const AZStd::vector<AZ::BezierSpline::BezierData>	&bezierData = bezierSpline->GetBezierData();
				for (u32 i = 0; i < fillCount; ++i)
				{
					const AZ::Vector3	in = 3.0f * (vertices[i] - bezierData[i].m_back);
					const AZ::Vector3	out = 3.0f * (bezierData[i].m_forward - vertices[i]);
					*dstTangents++ = ToPk(in);
					*dstTangents++ = ToPk(out);
				}
				if (isLooping)
				{
					const AZ::Vector3	in = 3.0f * (vertices[0] - bezierData[0].m_back);
					const AZ::Vector3	out = 3.0f * (bezierData[0].m_forward - vertices[0]);
					*dstTangents++ = ToPk(in);
					*dstTangents++ = ToPk(out);
				}
			}
			else if (const AZ::CatmullRomSpline *catmullRomSpline = azrtti_cast<const AZ::CatmullRomSpline*>(spline.get()))
			{
				//can't get CatmullRomSpline KnotParameterization, need to be at 0
				for (u32 i = catmullNotLoopingOffset; i < fillCount + catmullNotLoopingOffset; ++i)
				{
					const u32	prevI = i == 0 ? keyCount - 2 : i - 1;
					const u32	nextI = isLooping ? (i == keyCount - 2 ? 0 : i + 1) : (i + 1);
					const AZ::Vector3	prev = vertices[prevI];
					const AZ::Vector3	next = vertices[nextI];
					const AZ::Vector3	tan = 0.5f * (next - prev);
					*dstTangents++ = ToPk(tan);
					*dstTangents++ = ToPk(tan);
				}
				if (isLooping)
				{
					const AZ::Vector3	prev = vertices[keyCount - 2];
					const AZ::Vector3	next = vertices[1];
					const AZ::Vector3	tan = 0.5f * (next - prev);
					*dstTangents++ = ToPk(tan);
					*dstTangents++ = ToPk(tan);
				}
			}
		}

		m_Positions->RecomputeParametricDomain();

		if (!PK_VERIFY(m_Positions->IsCoherent()))
			return false;

		desc->m_Tracks.Clear();
		desc->m_Tracks.PushBack(CParticleSamplerDescriptor_AnimTrack_Default::SPathDefinition(m_Positions, null, null));
		PopcornFXSamplerComponentEventsBus::Event(m_AttachedToEntityId, &PopcornFXSamplerComponentEventsBus::Events::OnSamplerReady, m_AttachedToEntityId);

		return true;
	}

	void	PopcornFXSamplerAnimTrack::CopyFrom(const PopcornFXSamplerAnimTrack &other)
	{
		m_UseRelativeTransform = other.m_UseRelativeTransform;
	}

	void	PopcornFXSamplerAnimTrack::_SetTransform()
	{
		// Get entity's world transforms
		AZ::Transform parentTransform = AZ::Transform::CreateIdentity();
		if (m_UseRelativeTransform)
		{
			EBUS_EVENT_ID_RESULT(parentTransform, m_AttachedToEntityId, AZ::TransformBus, GetLocalTM);
		}
		else
		{
			EBUS_EVENT_ID_RESULT(parentTransform, m_AttachedToEntityId, AZ::TransformBus, GetWorldTM);
		}
		// Setup effect transforms
		AZ::Matrix4x4 tr = AZ::Matrix4x4::CreateFromTransform(parentTransform);
		Mem::Copy(&m_CurrentMat, &tr, sizeof(CFloat4x4));
		m_CurrentMat.Transpose();
		m_CurrentMat.XAxis().w() = 0.0f;
		m_CurrentMat.YAxis().w() = 0.0f;
		m_CurrentMat.ZAxis().w() = 0.0f;
		m_CurrentMat.WAxis().w() = 1.0f;
		m_CurrentMatUnscaled = m_CurrentMat;
		m_CurrentMatUnscaled.RemoveScale();
	}

	void	PopcornFXSamplerAnimTrack::_Clean()
	{
		if (m_SamplerDescriptor != null)
		{
			PParticleSamplerDescriptor_AnimTrack_Default	desc = reinterpret_cast<CParticleSamplerDescriptor_AnimTrack_Default *>(m_SamplerDescriptor.Get());
			if (PK_VERIFY(desc != null))
			{
				desc->m_Tracks.Clean();
			}
			m_SamplerDescriptor = null;
		}
	}

#endif //O3DE_USE_PK


}
