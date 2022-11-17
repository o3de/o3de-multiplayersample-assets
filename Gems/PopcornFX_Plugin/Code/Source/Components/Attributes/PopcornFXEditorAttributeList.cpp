//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXEditorAttributeList.h"

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/sort.h>
#include <PopcornFX/PopcornFXBus.h>

#include "Integration/PopcornFXUtils.h"

namespace PopcornFX {

//----------------------------------------------------------------------------
// PopcornFXEditorAttribute
//----------------------------------------------------------------------------

void PopcornFXEditorAttribute::Reflect(AZ::ReflectContext *context)
{
	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXEditorAttribute>()
			->Version(1, &VersionConverter)
			->Field("ValueFX", &PopcornFXEditorAttribute::m_ValueFX)
			->Field("ValueFY", &PopcornFXEditorAttribute::m_ValueFY)
			->Field("ValueFZ", &PopcornFXEditorAttribute::m_ValueFZ)
			->Field("ValueFW", &PopcornFXEditorAttribute::m_ValueFW)
			->Field("ValueIX", &PopcornFXEditorAttribute::m_ValueIX)
			->Field("ValueIY", &PopcornFXEditorAttribute::m_ValueIY)
			->Field("ValueIZ", &PopcornFXEditorAttribute::m_ValueIZ)
			->Field("ValueIW", &PopcornFXEditorAttribute::m_ValueIW)
			->Field("ValueBX", &PopcornFXEditorAttribute::m_ValueBX)
			->Field("ValueBY", &PopcornFXEditorAttribute::m_ValueBY)
			->Field("ValueBZ", &PopcornFXEditorAttribute::m_ValueBZ)
			->Field("ValueBW", &PopcornFXEditorAttribute::m_ValueBW)
			->Field("Quat", &PopcornFXEditorAttribute::m_ValueQuat)
			->Field("Color", &PopcornFXEditorAttribute::m_ValueColor)
			->Field("ColorAlpha", &PopcornFXEditorAttribute::m_ValueColorAlpha)
			->Field("ResetButton", &PopcornFXEditorAttribute::m_ResetButton)
			;

		AZ::EditContext	*edit = serializeContext->GetEditContext();
		if (edit)
		{
			edit->Class<PopcornFXEditorAttribute>("PopcornFXAttribute", "")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::NameLabelOverride, &PopcornFXEditorAttribute::m_Name)
					->Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &PopcornFXEditorAttribute::m_Description)
					->Attribute(AZ::Edit::Attributes::AutoExpand, false)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueFX, "X", "Attribute's X Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueFX)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueFX)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueFX)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueFY, "Y", "Attribute's Y Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueFY)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueFY)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueFY)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueFZ, "Z", "Attribute's Z Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueFZ)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueFZ)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueFZ)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueFW, "W", "Attribute's W Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueFW)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueFW)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueFW)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueIX, "X", "Attribute's X Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueIX)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueIX)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueIX)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueIY, "Y", "Attribute's Y Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueIY)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueIY)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueIY)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueIZ, "Z", "Attribute's Z Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueIZ)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueIZ)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueIZ)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueIW, "W", "Attribute's W Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueIW)
					->Attribute(AZ::Edit::Attributes::Min, &PopcornFXEditorAttribute::m_MinValueIW)
					->Attribute(AZ::Edit::Attributes::Max, &PopcornFXEditorAttribute::m_MaxValueIW)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::CheckBox, &PopcornFXEditorAttribute::m_ValueBX, "X", "Attribute's X Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueBX)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::CheckBox, &PopcornFXEditorAttribute::m_ValueBY, "Y", "Attribute's Y Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueBY)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::CheckBox, &PopcornFXEditorAttribute::m_ValueBZ, "Z", "Attribute's Z Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueBZ)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::CheckBox, &PopcornFXEditorAttribute::m_ValueBW, "W", "Attribute's W Value")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueBW)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Quaternion, &PopcornFXEditorAttribute::m_ValueQuat, "Quat", "Attribute's Quaternion")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueQuat)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Color, &PopcornFXEditorAttribute::m_ValueColor, "Color", "Attribute's Color")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueColor)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Slider, &PopcornFXEditorAttribute::m_ValueColorAlpha, "Alpha", "Attribute's Color Alpha")
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEditorAttribute::m_ShowValueColorAlpha)
					->Attribute(AZ::Edit::Attributes::Min, 0.0f)
					->Attribute(AZ::Edit::Attributes::Max, 1.0f)
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnAttributeChanged)
				->DataElement(AZ::Edit::UIHandlers::Button, &PopcornFXEditorAttribute::m_ResetButton, "", "Reset attribute to default value")
					->Attribute(AZ::Edit::Attributes::ButtonText, "Reset")
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttribute::OnResetButtonPressed);
				;
		}
	}
}

//----------------------------------------------------------------------------.

// Private Static
bool	PopcornFXEditorAttribute::VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context; (void)classElement;
	return true;
}

//----------------------------------------------------------------------------

PopcornFXEditorAttribute::PopcornFXEditorAttribute()
{
}

//----------------------------------------------------------------------------

void PopcornFXEditorAttribute::Copy(AZ::EntityId entityId, const CParticleAttributeDeclaration *attrib, const SAttributesContainer::SAttrib &attribValue, AZ::u32 id)
{
	m_EntityId = entityId;
	if (!attrib->ExportedName().Empty())
		m_Name = attrib->ExportedName().Data();

	m_Type = (AZ::u32)attrib->ExportedType();

	m_Semantic = (AZ::u32)attrib->GetEffectiveDataSemantic();
	//if it's an HDR color, remove the DataSemantic_Color
	if (m_Semantic == DataSemantic_Color &&
		(attribValue.Get<float>()[0] > 1.0f || attribValue.Get<float>()[1] > 1.0f || attribValue.Get<float>()[2] > 1.0f))
	{
		m_Semantic = DataSemantic_None;
	}

	m_Description = AZStd::string("Type ") + O3DEPopcornFXTypeToString(BaseTypeToO3DEPopcornFXType((EBaseTypeID)m_Type));
	if ((m_Type == BaseType_Float3 || m_Type == BaseType_Float4) && m_Semantic == DataSemantic_Color)
		m_Description += " Color";
	if (!attrib->Description().Empty())
		m_Description += AZStd::string(" - ") + attrib->Description().MapDefault().ToUTF8().Data();

	m_Id = id;

	if (m_Type == BaseType_Quaternion)
	{
		_FillAttributeQuat(attribValue);
	}
	else if ((m_Type == BaseType_Float3 || m_Type == BaseType_Float4) && m_Semantic == DataSemantic_Color)
	{
		_FillAttributeColor(attribValue);
	}
	else
	{
		// Regular attribute
		const CBaseTypeTraits	&baseTraits = CBaseTypeTraits::Traits(static_cast<EBaseTypeID>(m_Type));

		for (u32 componentId = 0; componentId < baseTraits.VectorDimension; componentId++)
			_FillAttribute(attrib, attribValue, componentId);
	}
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttribute::Reset()
{
	EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, ResetAttributeToDefault, m_Id);
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttribute::Refresh()
{
	EBaseTypeID	type = (EBaseTypeID)m_Type;
	switch (type)
	{
	case BaseType_Float:
		EBUS_EVENT_ID_RESULT(m_ValueFX, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsFloat, m_Id);
		break;
	case BaseType_Float2:
	{
		AZ::Vector2	value;
		EBUS_EVENT_ID_RESULT(value, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsFloat2, m_Id);
		m_ValueFX = value.GetX();
		m_ValueFY = value.GetY();
	}
	break;
	case BaseType_Float3:
	{
		if (m_Semantic == DataSemantic_Color)
		{
			AZ::Color	value;
			EBUS_EVENT_ID_RESULT(value, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsColor, m_Id);
			_SetColorValues(value);
		}
		else
		{
			AZ::Vector3	value;
			EBUS_EVENT_ID_RESULT(value, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsFloat3, m_Id);
			m_ValueFX = value.GetX();
			m_ValueFY = value.GetY();
			m_ValueFZ = value.GetZ();
		}
	}
	break;
	case BaseType_Float4:
	{
		if (m_Semantic == DataSemantic_Color)
		{
			AZ::Color	value;
			EBUS_EVENT_ID_RESULT(value, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsColor, m_Id);
			_SetColorValues(value);
		}
		else
		{
			AZ::Vector4	value;
			EBUS_EVENT_ID_RESULT(value, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsFloat4, m_Id);
			m_ValueFX = value.GetX();
			m_ValueFY = value.GetY();
			m_ValueFZ = value.GetZ();
			m_ValueFW = value.GetW();
		}
	}
	break;
	case BaseType_Int4:
		EBUS_EVENT_ID_RESULT(m_ValueIW, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeWAsInt, m_Id);
	case BaseType_Int3:
		EBUS_EVENT_ID_RESULT(m_ValueIZ, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeZAsInt, m_Id);
	case BaseType_Int2:
		EBUS_EVENT_ID_RESULT(m_ValueIY, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeYAsInt, m_Id);
	case BaseType_I32:
		EBUS_EVENT_ID_RESULT(m_ValueIX, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeXAsInt, m_Id);
		break;
	case BaseType_Bool4:
		EBUS_EVENT_ID_RESULT(m_ValueBW, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeWAsBool, m_Id);
	case BaseType_Bool3:
		EBUS_EVENT_ID_RESULT(m_ValueBZ, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeZAsBool, m_Id);
	case BaseType_Bool2:
		EBUS_EVENT_ID_RESULT(m_ValueBY, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeYAsBool, m_Id);
	case BaseType_Bool:
		EBUS_EVENT_ID_RESULT(m_ValueBX, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeXAsBool, m_Id);
		break;
	case BaseType_Quaternion:
		EBUS_EVENT_ID_RESULT(m_ValueQuat, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeAsQuaternion, m_Id);
		break;
	default:
		break;
	}
}

//----------------------------------------------------------------------------

AZ::u32	PopcornFXEditorAttribute::OnAttributeChanged()
{
	EBaseTypeID	type = (EBaseTypeID)m_Type;
	switch (type)
	{
	case BaseType_Float:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsFloat, m_Id, m_ValueFX);
		break;
	case BaseType_Float2:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsFloat2, m_Id, AZ::Vector2(m_ValueFX, m_ValueFY));
		break;
	case BaseType_Float3:
		if (m_Semantic == DataSemantic_Color)
		{
			EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsColor, m_Id, AZ::Color::CreateFromVector3(m_ValueColor.GetAsVector3()));
		}
		else
		{
			EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsFloat3, m_Id, AZ::Vector3(m_ValueFX, m_ValueFY, m_ValueFZ));
		}
		break;
	case BaseType_Float4:
		if (m_Semantic == DataSemantic_Color)
		{
			EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsColor, m_Id, AZ::Color::CreateFromVector3AndFloat(m_ValueColor.GetAsVector3(), m_ValueColorAlpha));
		}
		else
		{
			EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsFloat4, m_Id, AZ::Vector4(m_ValueFX, m_ValueFY, m_ValueFZ, m_ValueFW));
		}
		break;
	case BaseType_I32:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsInt, m_Id, m_ValueIX);
		break;
	case BaseType_Int2:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsInt2, m_Id, m_ValueIX, m_ValueIY);
		break;
	case BaseType_Int3:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsInt3, m_Id, m_ValueIX, m_ValueIY, m_ValueIZ);
		break;
	case BaseType_Int4:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsInt4, m_Id, m_ValueIX, m_ValueIY, m_ValueIZ, m_ValueIW);
		break;
	case BaseType_Bool:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsBool, m_Id, m_ValueBX);
		break;
	case BaseType_Bool2:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsBool2, m_Id, m_ValueBX, m_ValueBY);
		break;
	case BaseType_Bool3:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsBool3, m_Id, m_ValueBX, m_ValueBY, m_ValueBZ);
		break;
	case BaseType_Bool4:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsBool4, m_Id, m_ValueBX, m_ValueBY, m_ValueBZ, m_ValueBW);
		break;
	case BaseType_Quaternion:
		EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeAsQuaternion, m_Id, m_ValueQuat);
		break;
	default:
		break;
	}
	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
}

//----------------------------------------------------------------------------

AZ::u32	PopcornFXEditorAttribute::OnResetButtonPressed()
{
	Reset();
	Refresh();
	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttribute::_FillAttributeQuat(const SAttributesContainer::SAttrib &attribValue)
{
	m_ShowValueQuat = true;
	m_ValueQuat = AZ::Quaternion(attribValue.Get<float>()[0], attribValue.Get<float>()[1], attribValue.Get<float>()[2], attribValue.Get<float>()[3]);
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttribute::_FillAttributeColor(const SAttributesContainer::SAttrib &attribValue)
{
	m_ShowValueColor = true;

	_SetColorValues(AZ::Color(attribValue.Get<float>()[0], attribValue.Get<float>()[1], attribValue.Get<float>()[2], attribValue.Get<float>()[3]));

	if (m_Type == BaseType_Float4)
		m_ShowValueColorAlpha = true;
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttribute::_FillAttribute(const CParticleAttributeDeclaration *decl, const SAttributesContainer::SAttrib &attribValue, AZ::u32 laneId)
{
	const CBaseTypeTraits				&typeTraits = CBaseTypeTraits::Traits(static_cast<EBaseTypeID>(decl->ExportedType()));
	const SAttributesContainer::SAttrib	minValue = decl->GetMinValue();
	const SAttributesContainer::SAttrib	maxValue = decl->GetMaxValue();
	const bool							hasMax = decl->HasMax();
	const bool							hasMin = decl->HasMin();

	if (typeTraits.ScalarType == BaseType_Bool)
	{
		bool	*valueB = &m_ValueBX;
		bool	*showValueB = &m_ShowValueBX;

		valueB[laneId] = attribValue.Get<bool>()[laneId];
		showValueB[laneId] = true;
	}
	else if (typeTraits.IsFp)
	{
		float	*valueF = &m_ValueFX;
		float	*minValueF = &m_MinValueFX;
		float	*maxValueF = &m_MaxValueFX;
		bool	*showValueF = &m_ShowValueFX;

		valueF[laneId] = attribValue.Get<float>()[laneId];
		if (hasMin)
			minValueF[laneId] = minValue.Get<float>()[laneId];
		if (hasMax)
			maxValueF[laneId] = maxValue.Get<float>()[laneId];
		showValueF[laneId] = true;
	}
	else
	{
		AZ::s32	*valueI = &m_ValueIX;
		AZ::s32	*minValueI = &m_MinValueIX;
		AZ::s32	*maxValueI = &m_MaxValueIX;
		bool	*showValueI = &m_ShowValueIX;

		valueI[laneId] = attribValue.Get<s32>()[laneId];
		if (hasMin)
			minValueI[laneId] = minValue.Get<s32>()[laneId];
		if (hasMax)
			maxValueI[laneId] = maxValue.Get<s32>()[laneId];
		showValueI[laneId] = true;
	}
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttribute::_SetColorValues(const AZ::Color &color)
{
	m_ValueColorAlpha = color.GetA();
	m_ValueColor = AZ::Color::CreateFromVector3(color.GetAsVector3());
}

//----------------------------------------------------------------------------
// PopcornFXEditorSampler
//----------------------------------------------------------------------------

PopcornFXEditorSampler::PopcornFXEditorSampler()
{
}

//----------------------------------------------------------------------------

void	PopcornFXEditorSampler::Reflect(AZ::ReflectContext *context)
{
	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXEditorSampler>()
			->Version(1, &VersionConverter)
			->Field("Entity", &PopcornFXEditorSampler::m_SamplerEntityId)
		;

		AZ::EditContext	*edit = serializeContext->GetEditContext();
		if (edit)
		{
			edit->Class<PopcornFXEditorSampler>("PopcornFXSampler", "PopcornFX Effect's Sampler")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::NameLabelOverride, &PopcornFXEditorSampler::m_Name)
					->Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &PopcornFXEditorSampler::m_Description)
					->Attribute(AZ::Edit::Attributes::AutoExpand, false)
				->DataElement(0, &PopcornFXEditorSampler::m_SamplerEntityId, "Entity", "")
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorSampler::OnSamplerChanged)
					->Attribute(AZ::Edit::Attributes::SliceFlags, AZ::Edit::SliceFlags::DontGatherReference | AZ::Edit::SliceFlags::NotPushableOnSliceRoot)
				;
		}
	}
}

//----------------------------------------------------------------------------

// Private Static
bool	PopcornFXEditorSampler::VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context; (void)classElement;
	return true;
}

//----------------------------------------------------------------------------

void PopcornFXEditorSampler::Copy(AZ::EntityId entityId, const CParticleAttributeSamplerDeclaration *sampler, AZ::EntityId samplerValue, AZ::u32 id)
{
	m_Id = id;
	if (!sampler->ExportedName().Empty())
		m_Name = sampler->ExportedName().Data();

	m_Description = AZStd::string("Type Sampler");
	if (!sampler->Description().Empty())
		m_Description += AZStd::string(" - ") + sampler->Description().MapDefault().ToUTF8().Data();

	const PParticleSamplerDescriptor	&desc = sampler->GetSamplerDefaultDescriptor();
	if (desc != null)
		m_Type = desc->SamplerTypeID();
	else
		m_Type = (AZ::u32) - 1;

	m_EntityId = entityId;
	m_SamplerEntityId = samplerValue;
}

//----------------------------------------------------------------------------

void	PopcornFXEditorSampler::Reset()
{
	EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, ResetSamplerToDefault, m_Id);
}

//----------------------------------------------------------------------------

void	PopcornFXEditorSampler::Refresh()
{
	AZ::EntityId	newId;
	EBUS_EVENT_ID_RESULT(newId, m_EntityId, PopcornFXEmitterComponentRequestBus, GetAttributeSampler, m_Id);

	m_SamplerEntityId = newId;
}

//----------------------------------------------------------------------------

AZ::u32	PopcornFXEditorSampler::OnSamplerChanged()
{
	AZ::Entity	*samplerEntity = null;
	EBUS_EVENT_RESULT(samplerEntity, AZ::ComponentApplicationBus, FindEntity, m_SamplerEntityId);

	if (samplerEntity == null)
		m_SamplerEntityId = (AZ::EntityId)AZ::EntityId::InvalidEntityId;

	EBUS_EVENT_ID(m_EntityId, PopcornFXEmitterComponentRequestBus, SetAttributeSampler, m_Id, m_SamplerEntityId);
	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
}

//----------------------------------------------------------------------------
// PopcornFXEditorAttributeCategory
//----------------------------------------------------------------------------

PopcornFXEditorAttributeCategory::PopcornFXEditorAttributeCategory()
{
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeCategory::Reflect(AZ::ReflectContext *context)
{
	PopcornFXEditorAttribute::Reflect(context);
	PopcornFXEditorSampler::Reflect(context);

	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXEditorAttributeCategory>()
			->Version(1, &VersionConverter)
			->Field("Attributes", &PopcornFXEditorAttributeCategory::m_Attributes)
			->Field("Samplers", &PopcornFXEditorAttributeCategory::m_Samplers)
			;

		AZ::EditContext	*edit = serializeContext->GetEditContext();
		if (edit)
		{
			edit->Class<PopcornFXEditorAttributeCategory>("PopcornFXEditorAttributeCategory", "")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::NameLabelOverride, &PopcornFXEditorAttributeCategory::m_Name)
				->DataElement(AZ::Edit::UIHandlers::Default, &PopcornFXEditorAttributeCategory::m_Attributes, "", "")
					->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
				->DataElement(AZ::Edit::UIHandlers::Default, &PopcornFXEditorAttributeCategory::m_Samplers, "", "")
					->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
				;
		}
	}
}

//----------------------------------------------------------------------------

// Private Static
bool	PopcornFXEditorAttributeCategory::VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context; (void)classElement;
	return true;
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeCategory::AddAttribute(AZ::EntityId entityId, const CParticleAttributeDeclaration *attrib, const SAttributesContainer::SAttrib &attribValue, AZ::u32 id)
{
	m_Attributes.push_back({});

	size_t	attribId = m_Attributes.size() - 1;

	m_Attributes[attribId].Copy(entityId, attrib, attribValue, id);
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeCategory::AddSampler(AZ::EntityId entityId, const CParticleAttributeSamplerDeclaration *sampler, AZ::EntityId samplerValue, AZ::u32 id)
{
	m_Samplers.push_back({});

	size_t	samplerId = m_Samplers.size() - 1;

	m_Samplers[samplerId].Copy(entityId, sampler, samplerValue, id);
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeCategory::ResetAll()
{
	for (size_t i = 0; i < m_Attributes.size(); ++i)
	{
		m_Attributes[i].Reset();
		m_Attributes[i].Refresh();
	}
	for (size_t i = 0; i < m_Samplers.size(); ++i)
	{
		m_Samplers[i].Reset();
		m_Samplers[i].Refresh();
	}
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeCategory::RefreshAttributeIFP(AZ::u32 id)
{
	for (size_t i = 0; i < m_Attributes.size(); ++i)
	{
		if (m_Attributes[i].Id() == id)
			m_Attributes[i].Refresh();
	}
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeCategory::RefreshSamplerIFP(AZ::u32 id)
{
	for (size_t i = 0; i < m_Samplers.size(); ++i)
	{
		if (m_Samplers[i].Id() == id)
			m_Samplers[i].Refresh();
	}
}

//----------------------------------------------------------------------------
// PopcornFXEditorAttributeList
//----------------------------------------------------------------------------

PopcornFXEditorAttributeList::PopcornFXEditorAttributeList()
{
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeList::Reflect(AZ::ReflectContext *context)
{
	PopcornFXEditorAttributeCategory::Reflect(context);

	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXEditorAttributeList>()
			->Version(2, &VersionConverter)
			->Field("AttributeCategories", &PopcornFXEditorAttributeList::m_AttributeCategories)
			->Field("ResetAllButton", &PopcornFXEditorAttributeList::m_ResetAllButton)
			;

		AZ::EditContext	*edit = serializeContext->GetEditContext();
		if (edit)
		{
			edit->Class<PopcornFXEditorAttributeList>("AttributeList", "PopcornFX Effect's AttributeList")
				->DataElement(AZ::Edit::UIHandlers::Default, &PopcornFXEditorAttributeList::m_AttributeCategories, "", "")
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					->Attribute(AZ::Edit::Attributes::AutoExpand, false)
				->DataElement(AZ::Edit::UIHandlers::Button, &PopcornFXEditorAttributeList::m_ResetAllButton, "", "Reset all attributes to default value")
					->Attribute(AZ::Edit::Attributes::ButtonText, "Reset all")
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEditorAttributeList::OnResetAllButtonPressed);
				;
		}
	}
}

//----------------------------------------------------------------------------

// Private Static
bool	PopcornFXEditorAttributeList::VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context;
	if (classElement.GetVersion() == 1)
	{
		// "Remove old Attributes and Samplers, it will be reconstructed
		classElement.RemoveElementByName(AZ::Crc32("Attributes"));
		classElement.RemoveElementByName(AZ::Crc32("Samplers"));
	}
	return true;
}

//----------------------------------------------------------------------------

void PopcornFXEditorAttributeList::Prepare(	const CParticleAttributeList *defaultList, const TMemoryView<SAttributesContainer::SAttrib> attribRawData,
											const AZStd::vector<PopcornFXSampler> &samplers, AZ::EntityId entityId)
{
	Clear();

	if (defaultList == null)
		return;

	TMemoryView<const CParticleAttributeDeclaration * const>	srcAttribs = defaultList->UniqueAttributeList();
	const u32													attrCount = srcAttribs.Count();

	for (u32 iAttrib = 0; iAttrib < attrCount; ++iAttrib)
	{
		if (srcAttribs[iAttrib] != null)
		{
			CGuid	catId = _GetOrAddCategory(srcAttribs[iAttrib]->CategoryName());

			if (PK_VERIFY(catId.Valid()))
				m_AttributeCategories[catId].AddAttribute(entityId, srcAttribs[iAttrib], attribRawData[iAttrib], iAttrib);
		}
	}

	TMemoryView<const CParticleAttributeSamplerDeclaration *const>	srcSamplers = defaultList->UniqueSamplerList();
	const u32														samplerCount = srcSamplers.Count();

	PK_ASSERT(samplers.size() == samplerCount);

	for (u32 iSampler = 0; iSampler < samplerCount; ++iSampler)
	{
		if (srcSamplers[iSampler] != null)
		{
			CGuid	catId = _GetOrAddCategory(srcSamplers[iSampler]->CategoryName());

			if (PK_VERIFY(catId.Valid()))
				m_AttributeCategories[catId].AddSampler(entityId, srcSamplers[iSampler], samplers[iSampler].m_EntityId, iSampler);
		}
	}

	//Sort alphabetically
	AZStd::sort(m_AttributeCategories.begin(), m_AttributeCategories.end(), [](const PopcornFXEditorAttributeCategory &lhs, const PopcornFXEditorAttributeCategory &rhs)
	{
		if (lhs.Name() == "General")
			return true;
		else if (rhs.Name() == "General")
			return false;
		return lhs.Name().compare(rhs.Name()) < 0;
	});
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeList::Clear()
{
	m_AttributeCategories.clear();
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeList::RefreshAttribute(AZ::u32 attribId)
{
	for (size_t i = 0; i < m_AttributeCategories.size(); ++i)
		m_AttributeCategories[i].RefreshAttributeIFP(attribId);
}

//----------------------------------------------------------------------------

void	PopcornFXEditorAttributeList::RefreshSampler(AZ::u32 samplerId)
{
	for (size_t i = 0; i < m_AttributeCategories.size(); ++i)
		m_AttributeCategories[i].RefreshSamplerIFP(samplerId);
}

//----------------------------------------------------------------------------

AZ::u32	PopcornFXEditorAttributeList::OnResetAllButtonPressed()
{
	for (size_t i = 0; i < m_AttributeCategories.size(); ++i)
		m_AttributeCategories[i].ResetAll();
	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
}

//----------------------------------------------------------------------------

CGuid	PopcornFXEditorAttributeList::_GetOrAddCategory(const CStringLocalized &categoryName)
{
	CString	catName = categoryName.MapDefault().ToUTF8();
	if (catName.Empty())
		catName = "General";

	CGuid	catId = CGuid::INVALID;
	for (u32 iCat = 0; iCat < m_AttributeCategories.size(); ++iCat)
	{
		if (m_AttributeCategories[iCat].Name() == catName.Data())
			catId = iCat;
	}

	if (!catId.Valid())
	{
		m_AttributeCategories.push_back({});
		catId = m_AttributeCategories.size() - 1;
		m_AttributeCategories[catId].SetName(catName.Data());
	}
	return catId;
}

}
