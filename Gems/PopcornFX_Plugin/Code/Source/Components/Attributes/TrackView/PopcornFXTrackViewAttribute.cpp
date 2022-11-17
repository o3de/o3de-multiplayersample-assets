//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXTrackViewAttribute.h"

#include <AzCore/Component/TransformBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace PopcornFX {

	void	PopcornFXTrackViewAttribute::Reflect(AZ::ReflectContext *context)
	{
		auto	*behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
		if (behaviorContext)
		{
			behaviorContext->EBus<PopcornFXTrackViewComponentRequestBus>("PopcornFXTrackViewComponentRequestBus")
				->Event("SetValueFX", &PopcornFXTrackViewComponentRequestBus::Events::SetValueFX)
				->Event("GetValueFX", &PopcornFXTrackViewComponentRequestBus::Events::GetValueFX)
				->Event("SetValueFY", &PopcornFXTrackViewComponentRequestBus::Events::SetValueFY)
				->Event("GetValueFY", &PopcornFXTrackViewComponentRequestBus::Events::GetValueFY)
				->Event("SetValueFZ", &PopcornFXTrackViewComponentRequestBus::Events::SetValueFZ)
				->Event("GetValueFZ", &PopcornFXTrackViewComponentRequestBus::Events::GetValueFZ)
				->Event("SetValueFW", &PopcornFXTrackViewComponentRequestBus::Events::SetValueFW)
				->Event("GetValueFW", &PopcornFXTrackViewComponentRequestBus::Events::GetValueFW)
				->VirtualProperty("ValueFloatX", "GetValueFX", "SetValueFX")
				->VirtualProperty("ValueFloatY", "GetValueFY", "SetValueFY")
				->VirtualProperty("ValueFloatZ", "GetValueFZ", "SetValueFZ")
				->VirtualProperty("ValueFloatW", "GetValueFW", "SetValueFW")
				->Event("SetValueIX", &PopcornFXTrackViewComponentRequestBus::Events::SetValueIX)
				->Event("GetValueIX", &PopcornFXTrackViewComponentRequestBus::Events::GetValueIX)
				->Event("SetValueIY", &PopcornFXTrackViewComponentRequestBus::Events::SetValueIY)
				->Event("GetValueIY", &PopcornFXTrackViewComponentRequestBus::Events::GetValueIY)
				->Event("SetValueIZ", &PopcornFXTrackViewComponentRequestBus::Events::SetValueIZ)
				->Event("GetValueIZ", &PopcornFXTrackViewComponentRequestBus::Events::GetValueIZ)
				->Event("SetValueIW", &PopcornFXTrackViewComponentRequestBus::Events::SetValueIW)
				->Event("GetValueIW", &PopcornFXTrackViewComponentRequestBus::Events::GetValueIW)
				->VirtualProperty("ValueIntX", "GetValueIX", "SetValueIX")
				->VirtualProperty("ValueIntY", "GetValueIY", "SetValueIY")
				->VirtualProperty("ValueIntZ", "GetValueIZ", "SetValueIZ")
				->VirtualProperty("ValueIntW", "GetValueIW", "SetValueIW")
				->Event("SetValueBX", &PopcornFXTrackViewComponentRequestBus::Events::SetValueBX)
				->Event("GetValueBX", &PopcornFXTrackViewComponentRequestBus::Events::GetValueBX)
				->Event("SetValueBY", &PopcornFXTrackViewComponentRequestBus::Events::SetValueBY)
				->Event("GetValueBY", &PopcornFXTrackViewComponentRequestBus::Events::GetValueBY)
				->Event("SetValueBZ", &PopcornFXTrackViewComponentRequestBus::Events::SetValueBZ)
				->Event("GetValueBZ", &PopcornFXTrackViewComponentRequestBus::Events::GetValueBZ)
				->Event("SetValueBW", &PopcornFXTrackViewComponentRequestBus::Events::SetValueBW)
				->Event("GetValueBW", &PopcornFXTrackViewComponentRequestBus::Events::GetValueBW)
				->VirtualProperty("ValueBoolX", "GetValueBX", "SetValueBX")
				->VirtualProperty("ValueBoolY", "GetValueBY", "SetValueBY")
				->VirtualProperty("ValueBoolZ", "GetValueBZ", "SetValueBZ")
				->VirtualProperty("ValueBoolW", "GetValueBW", "SetValueBW")
				->Event("SetValueQuat", &PopcornFXTrackViewComponentRequestBus::Events::SetValueQuat)
				->Event("GetValueQuat", &PopcornFXTrackViewComponentRequestBus::Events::GetValueQuat)
				->VirtualProperty("ValueQuaternion", "GetValueQuat", "SetValueQuat")
				->Event("SetValueColor", &PopcornFXTrackViewComponentRequestBus::Events::SetValueColor)
				->Event("GetValueColor", &PopcornFXTrackViewComponentRequestBus::Events::GetValueColor)
				->VirtualProperty("ValueColor", "GetValueColor", "SetValueColor")
				->Event("SetValueColorAlpha", &PopcornFXTrackViewComponentRequestBus::Events::SetValueColorAlpha)
				->Event("GetValueColorAlpha", &PopcornFXTrackViewComponentRequestBus::Events::GetValueColorAlpha)
				->VirtualProperty("ValueColorAlpha", "GetValueColorAlpha", "SetValueColorAlpha")
				;
		}
	}

	PopcornFXTrackViewAttribute::PopcornFXTrackViewAttribute()
	{
	}

	PopcornFXTrackViewAttribute::~PopcornFXTrackViewAttribute()
	{
	}

	void	PopcornFXTrackViewAttribute::Activate(AZ::EntityId entityId, const AZStd::string &attributeName)
	{
		m_AttributeName = attributeName;
		AZ::TransformBus::EventResult(m_ParentId, entityId, &AZ::TransformBus::Events::GetParentId);
		PopcornFXTrackViewComponentRequestBus::Handler::BusConnect(entityId);
		PopcornFXEmitterComponentEventsBus::Handler::BusConnect(m_ParentId);
	}

	void	PopcornFXTrackViewAttribute::Deactivate()
	{
		PopcornFXTrackViewComponentRequestBus::Handler::BusDisconnect();
		PopcornFXEmitterComponentEventsBus::Handler::BusDisconnect();
	}

	void	PopcornFXTrackViewAttribute::SetAttributeName(const AZStd::string &name)
	{
		m_AttributeName = name;
		GetAttributeIdAndType();
	}

	void	PopcornFXTrackViewAttribute::OnEmitterReady()
	{
		m_FxCreated = true;
		GetAttributeIdAndType();
	}

	void	PopcornFXTrackViewAttribute::SetValueFX(float value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 0);
	}

	float	PopcornFXTrackViewAttribute::GetValueFX() const
	{
		if (!AttributeValid())
			return 0.0f;
		AZ::Vector4	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsFloat4, m_AttributeId);
		return value.GetX();
	}

	void	PopcornFXTrackViewAttribute::SetValueFY(float value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 1);
	}

	float	PopcornFXTrackViewAttribute::GetValueFY() const
	{
		if (!AttributeValid())
			return 0.0f;
		AZ::Vector4	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsFloat4, m_AttributeId);
		return value.GetY();
	}

	void	PopcornFXTrackViewAttribute::SetValueFZ(float value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 2);
	}

	float	PopcornFXTrackViewAttribute::GetValueFZ() const
	{
		if (!AttributeValid())
			return 0.0f;
		AZ::Vector4	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsFloat4, m_AttributeId);
		return value.GetZ();
	}

	void	PopcornFXTrackViewAttribute::SetValueFW(float value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 3);
	}

	float	PopcornFXTrackViewAttribute::GetValueFW() const
	{
		if (!AttributeValid())
			return 0.0f;
		AZ::Vector4	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsFloat4, m_AttributeId);
		return value.GetW();
	}

	void	PopcornFXTrackViewAttribute::SetValueIX(AZ::u32 value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 0);
	}

	AZ::u32	PopcornFXTrackViewAttribute::GetValueIX() const
	{
		if (!AttributeValid())
			return 0;
		AZ::u32	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeXAsInt, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueIY(AZ::u32 value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 1);
	}

	AZ::u32	PopcornFXTrackViewAttribute::GetValueIY() const
	{
		if (!AttributeValid())
			return 0;
		AZ::u32	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeYAsInt, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueIZ(AZ::u32 value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 2);
	}

	AZ::u32	PopcornFXTrackViewAttribute::GetValueIZ() const
	{
		if (!AttributeValid())
			return 0;
		AZ::u32	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeZAsInt, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueIW(AZ::u32 value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 3);
	}

	AZ::u32	PopcornFXTrackViewAttribute::GetValueIW() const
	{
		if (!AttributeValid())
			return 0;
		AZ::u32	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeWAsInt, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueBX(bool value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 0);
	}

	bool	PopcornFXTrackViewAttribute::GetValueBX() const
	{
		if (!AttributeValid())
			return false;
		bool	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeXAsBool, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueBY(bool value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 1);
	}

	bool	PopcornFXTrackViewAttribute::GetValueBY() const
	{
		if (!AttributeValid())
			return false;
		bool	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeYAsBool, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueBZ(bool value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 2);
	}

	bool	PopcornFXTrackViewAttribute::GetValueBZ() const
	{
		if (!AttributeValid())
			return false;
		bool	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeZAsBool, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueBW(bool value)
	{
		if (!AttributeValid())
			return;
		SetValue(value, 3);
	}

	bool	PopcornFXTrackViewAttribute::GetValueBW() const
	{
		if (!AttributeValid())
			return false;
		bool	value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeWAsBool, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueQuat(const AZ::Quaternion &value)
	{
		if (!AttributeValid())
			return;
		PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsQuaternion, m_AttributeId, value);
	}

	AZ::Quaternion	PopcornFXTrackViewAttribute::GetValueQuat() const
	{
		AZ::Quaternion	value;
		if (!AttributeValid())
			return value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsQuaternion, m_AttributeId);
		return value;
	}

	void	PopcornFXTrackViewAttribute::SetValueColor(const AZ::Color &value)
	{
		if (!AttributeValid())
			return;

		AZ::Color	curColor;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(curColor, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsColor, m_AttributeId);
		PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsColor, m_AttributeId, AZ::Color::CreateFromVector3AndFloat(value.GetAsVector3(), curColor.GetA()));

	}

	AZ::Color	PopcornFXTrackViewAttribute::GetValueColor() const
	{
		AZ::Color	value;
		if (!AttributeValid())
			return value;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(value, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsColor, m_AttributeId);
		return AZ::Color::CreateFromVector3(value.GetAsVector3());
	}

	void	PopcornFXTrackViewAttribute::SetValueColorAlpha(float value)
	{
		if (!AttributeValid())
			return;
		AZ::Color	curColor;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(curColor, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsColor, m_AttributeId);
		curColor.SetA(value);
		PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsColor, m_AttributeId, curColor);
	}

	float	PopcornFXTrackViewAttribute::GetValueColorAlpha() const
	{
		if (!AttributeValid())
			return 0.0f;
		AZ::Color	curColor;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(curColor, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsColor, m_AttributeId);
		return curColor.GetA();
	}

	bool	PopcornFXTrackViewAttribute::AttributeValid() const
	{
		if (!m_FxCreated)
			return false;
		if (m_AttributeId == (AZ::s32)-1)
		{
			AZ_Error("PopcornFX", false, "TrackView Attribute %s not found", m_AttributeName.c_str());
			return false;
		}
		return true;
	}

	void	PopcornFXTrackViewAttribute::GetAttributeIdAndType()
	{
		if (!m_AttributeName.empty())
		{
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(m_AttributeId, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeId, m_AttributeName);
			if (m_AttributeId != -1)
			{
				PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(m_AttributeType, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeType, m_AttributeId);
			}
		}
	}

	void	PopcornFXTrackViewAttribute::SetValue(float value, AZ::u32 id)
	{
		AZ::Vector4	vecValue;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(vecValue, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeAsFloat4, m_AttributeId);

		if (id == 0)
			vecValue.SetX(value);
		else if (id == 1)
			vecValue.SetY(value);
		else if (id == 2)
			vecValue.SetZ(value);
		else if (id == 3)
			vecValue.SetW(value);

		switch (m_AttributeType)
		{
		case Type_Float:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsFloat, m_AttributeId, vecValue.GetX());
			break;
		case Type_Float2:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsFloat2, m_AttributeId, AZ::Vector2(vecValue.GetX(), vecValue.GetY()));
			break;
		case Type_Float3:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsFloat3, m_AttributeId, vecValue.GetAsVector3());
			break;
		case Type_Float4:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsFloat4, m_AttributeId, vecValue);
			break;
		default:
			break;
		}
	}

	void	PopcornFXTrackViewAttribute::SetValue(AZ::u32 value, AZ::u32 id)
	{
		AZ::u32	valueX, valueY, valueZ, valueW;

		if (id == 0)
			valueX = value;
		else
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueX, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeXAsInt, m_AttributeId);

		if (id == 1)
			valueY = value;
		else if (m_AttributeType >= Type_Int2)
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueY, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeYAsInt, m_AttributeId);

		if (id == 2)
			valueZ = value;
		else if (m_AttributeType >= Type_Int3)
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueZ, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeZAsInt, m_AttributeId);

		if (id == 3)
			valueW = value;
		else if (m_AttributeType == Type_Int4)
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueW, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeWAsInt, m_AttributeId);

		switch (m_AttributeType)
		{
		case Type_Int:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsInt, m_AttributeId, valueX);
			break;
		case Type_Int2:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsInt2, m_AttributeId, valueX, valueY);
			break;
		case Type_Int3:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsInt3, m_AttributeId, valueX, valueY, valueZ);
			break;
		case Type_Int4:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsInt4, m_AttributeId, valueX, valueY, valueZ, valueW);
			break;
		default:
			break;
		}
	}

	void	PopcornFXTrackViewAttribute::SetValue(bool value, AZ::u32 id)
	{
		bool	valueX, valueY, valueZ, valueW;

		if (id == 0)
			valueX = value;
		else
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueX, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeXAsBool, m_AttributeId);

		if (id == 1)
			valueY = value;
		else if (m_AttributeType >= Type_Bool2)
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueY, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeYAsBool, m_AttributeId);

		if (id == 2)
			valueZ = value;
		else if (m_AttributeType >= Type_Bool3)
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueZ, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeZAsBool, m_AttributeId);

		if (id == 3)
			valueW = value;
		else if (m_AttributeType == Type_Bool4)
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(valueW, m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeWAsBool, m_AttributeId);

		switch (m_AttributeType)
		{
		case Type_Bool:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsBool, m_AttributeId, valueX);
			break;
		case Type_Bool2:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsBool2, m_AttributeId, valueX, valueY);
			break;
		case Type_Bool3:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsBool3, m_AttributeId, valueX, valueY, valueZ);
			break;
		case Type_Bool4:
			PopcornFX::PopcornFXEmitterComponentRequestBus::Event(m_ParentId, &PopcornFX::PopcornFXEmitterComponentRequests::SetAttributeAsBool4, m_AttributeId, valueX, valueY, valueZ, valueW);
			break;
		default:
			break;
		}
	}

}
