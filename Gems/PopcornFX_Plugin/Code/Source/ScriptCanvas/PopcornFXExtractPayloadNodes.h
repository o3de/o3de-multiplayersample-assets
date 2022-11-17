//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFX_precompiled.h"

#include <ScriptCanvas/Core/NodeFunctionGeneric.h>

#include "Integration/PopcornFXIntegrationBus.h"

namespace PopcornFX
{
	namespace ExtractPayloadNodes
	{
		class ExtractPayload
		{
		public:
			AZ_TYPE_INFO(ExtractPayload, "{AA62A057-DE7E-4CBB-B0C6-615A7F3A05F4}");

			static void Reflect(AZ::ReflectContext* context)
			{
				if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
				{
					serializeContext->Class<ExtractPayload>()
						->Version(0)
						;

					if (AZ::EditContext* editContext = serializeContext->GetEditContext())
					{
						editContext->Class<ExtractPayload>("ExtractPayload", "")
							->ClassElement(AZ::Edit::ClassElements::EditorData, "")
							;
					}
				}
			}
		};

		static constexpr const char* k_categoryName = "PopcornFX/ExtractPayload";

		#define SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE(UUID, TYPE_NAME, TYPE, DEFAULT_VALUE, VALUE)\
		AZ_INLINE AZStd::tuple<TYPE, bool> ExtractPayload##TYPE_NAME(const AZStd::string &payloadName)\
		{\
			const PopcornFX::SPayloadValue	*payloadValue;\
			PopcornFX::PopcornFXIntegrationBus::BroadcastResult(payloadValue, &PopcornFX::PopcornFXIntegrationBus::Handler::GetCurrentPayloadValue, payloadName);\
			if (payloadValue == null)\
				return AZStd::tuple<TYPE, bool>(DEFAULT_VALUE, false);\
			return AZStd::tuple<TYPE, bool>(VALUE, true);\
		}\
		SCRIPT_CANVAS_GENERIC_FUNCTION_NODE(ExtractPayload##TYPE_NAME,\
			k_categoryName,\
			UUID,\
			"Extract a payload from a broadcasted event.",\
			"Payload Name",\
			"Value",\
			"Found");\

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{DF475DD2-D0E4-41EF-9094-705FEB4D8724}",
			Float,
			float,
			0.0f,
			payloadValue->m_ValueFloat[0]);

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{C5BB9DEE-B357-4765-B21B-04C7EC89F642}",
			Float2,
			AZ::Vector2,
			AZ::Vector2(0.0f),
			AZ::Vector2(payloadValue->m_ValueFloat[0], payloadValue->m_ValueFloat[1]));

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{1B9453A1-4A35-472B-9B3F-19C18755144D}",
			Float3,
			AZ::Vector3,
			AZ::Vector3(0.0f),
			AZ::Vector3(payloadValue->m_ValueFloat[0], payloadValue->m_ValueFloat[1], payloadValue->m_ValueFloat[2]));

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{E114C0FD-983E-42C2-9DFF-D33A8FD4EF74}",
			Float4,
			AZ::Vector4,
			AZ::Vector4(0.0f),
			AZ::Vector4(payloadValue->m_ValueFloat[0], payloadValue->m_ValueFloat[1], payloadValue->m_ValueFloat[2], payloadValue->m_ValueFloat[3]));

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{593D33D7-6467-4930-BD3C-169669F884A5}",
			Int,
			AZ::u32,
			0,
			payloadValue->m_ValueInt[0]);

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{4CEDB2C6-B16A-46D2-9C04-1347C1F098AE}",
			Int2,
			AZ::Vector2,
			AZ::Vector2(0.0f),
			AZ::Vector2(static_cast<float>(payloadValue->m_ValueInt[0]), static_cast<float>(payloadValue->m_ValueInt[1])));

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{759E8C0F-2EB6-412D-83C4-0A167391274B}",
			Int3,
			AZ::Vector3,
			AZ::Vector3(0.0f),
			AZ::Vector3(static_cast<float>(payloadValue->m_ValueInt[0]), static_cast<float>(payloadValue->m_ValueInt[1]), static_cast<float>(payloadValue->m_ValueInt[2])));

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{2E73B20A-AAC8-4E33-8BB4-DF880C23DA34}",
			Int4,
			AZ::Vector4,
			AZ::Vector4(0.0f),
			AZ::Vector4(static_cast<float>(payloadValue->m_ValueInt[0]), static_cast<float>(payloadValue->m_ValueInt[1]), static_cast<float>(payloadValue->m_ValueInt[2]), static_cast<float>(payloadValue->m_ValueInt[3])));

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{AF5CD975-5493-43C0-895C-FCDBDB278AA6}",
			Bool,
			bool,
			false,
			payloadValue->m_ValueBool[0]);

		SCRIPT_CANVAS_EXTRACT_PAYLOAD_FUNCTION_NODE("{CAFE93B0-2FD3-40AE-A15A-23831C5EBAB6}",
			Orientation,
			AZ::Quaternion,
			AZ::Quaternion::CreateIdentity(),
			AZ::Quaternion(payloadValue->m_ValueFloat[0], payloadValue->m_ValueFloat[1], payloadValue->m_ValueFloat[2], payloadValue->m_ValueFloat[3]));
		
		///////////////////////////////////////////////////////////////
		using Registrar = ScriptCanvas::RegistrarGeneric
			<ExtractPayloadFloatNode,
			ExtractPayloadFloat2Node,
			ExtractPayloadFloat3Node,
			ExtractPayloadFloat4Node,
			ExtractPayloadIntNode,
			ExtractPayloadInt2Node,
			ExtractPayloadInt3Node,
			ExtractPayloadInt4Node,
			ExtractPayloadBoolNode,
			ExtractPayloadOrientationNode>;
	}
}
