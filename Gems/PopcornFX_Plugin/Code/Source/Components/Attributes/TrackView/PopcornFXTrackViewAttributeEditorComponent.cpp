//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXTrackViewAttributeEditorComponent.h"
#include "PopcornFXTrackViewAttributeGameComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include "Integration/PopcornFXUtils.h"

namespace PopcornFX {

	void	PopcornFXTrackViewAttributeEditorComponent::Reflect(AZ::ReflectContext *context)
	{
		auto	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
		if (serializeContext)
		{
			serializeContext->Class<PopcornFXTrackViewAttributeEditorComponent, AZ::Component>()
				->Version(1)
				->Field("AttributeName", &PopcornFXTrackViewAttributeEditorComponent::m_AttributeName)
				;

			// Edit context:
			auto	*editContext = serializeContext->GetEditContext();
			if (editContext)
			{
				editContext->Class<PopcornFXTrackViewAttributeEditorComponent>("PopcornFX TrackView Attribute", "Drive attribute from TrackView")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &PopcornFXTrackViewAttributeEditorComponent::m_AttributeName, "Attribute name", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXTrackViewAttributeEditorComponent::OnAttributeNameChanged)
						->Attribute(AZ::Edit::Attributes::StringList, &PopcornFXTrackViewAttributeEditorComponent::GetAttributeNames)
						->Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &PopcornFXTrackViewAttributeEditorComponent::GetAttributeDescription)
					;
			}
		}

		AZ::BehaviorContext	*behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
		if (behaviorContext)
		{
			behaviorContext->Class<PopcornFXTrackViewAttributeEditorComponent>()
				->RequestBus("PopcornFXTrackViewComponentRequestBus")
				;
		}
	}

	PopcornFXTrackViewAttributeEditorComponent::PopcornFXTrackViewAttributeEditorComponent()
	{
	}

	PopcornFXTrackViewAttributeEditorComponent::~PopcornFXTrackViewAttributeEditorComponent()
	{
	}

	void	PopcornFXTrackViewAttributeEditorComponent::Init()
	{
	}

	void	PopcornFXTrackViewAttributeEditorComponent::Activate()
	{
		m_TrackViewAttribute.Activate(GetEntityId(), m_AttributeName);

		AZ::EntityId	parentId;
		AZ::TransformBus::EventResult(parentId, GetEntityId(), &AZ::TransformBus::Events::GetParentId);
		PopcornFXEmitterEditorComponentEventsBus::Handler::BusConnect(parentId);
		PopcornFXEmitterComponentEventsBus::Handler::BusConnect(parentId);
	}

	void PopcornFXTrackViewAttributeEditorComponent::Deactivate()
	{
		m_TrackViewAttribute.Deactivate();

		PopcornFXEmitterEditorComponentEventsBus::Handler::BusDisconnect();
		PopcornFXEmitterComponentEventsBus::Handler::BusDisconnect();
	}

	void PopcornFXTrackViewAttributeEditorComponent::BuildGameEntity(AZ::Entity *gameEntity)
	{
		PopcornFXTrackViewAttributeGameComponent	*component = gameEntity->CreateComponent<PopcornFXTrackViewAttributeGameComponent>();

		if (component)
		{
			component->m_AttributeName = m_AttributeName;
		}
	}

	void	PopcornFXTrackViewAttributeEditorComponent::OnEmitterReady()
	{
		AZ::EntityId	parentId;
		AZ::TransformBus::EventResult(parentId, GetEntityId(), &AZ::TransformBus::Events::GetParentId);

		AZ::u32	attributesCount;
		PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(attributesCount, parentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributesCount);

		m_AttributeNames.clear();
		m_AttributeNames.resize(attributesCount);
		for (AZ::u32 i = 0; i < attributesCount; ++i)
		{
			AZStd::string	name;
			PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(m_AttributeNames[i], parentId, &PopcornFX::PopcornFXEmitterComponentRequests::GetAttributeName, i);
		}
	}

	void	PopcornFXTrackViewAttributeEditorComponent::OnAttributeChanged()
	{
		//notify modification to make the recording works
		AzToolsFramework::PropertyEditorEntityChangeNotificationBus::Event(
			GetEntityId(),
			&AzToolsFramework::PropertyEditorEntityChangeNotifications::OnEntityComponentPropertyChanged,
			GetId());
		//ask UI to refresh attributes values
		AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
			&AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay,
			AzToolsFramework::Refresh_Values);
	}

	AZ::u32	PopcornFXTrackViewAttributeEditorComponent::OnAttributeNameChanged()
	{
		m_TrackViewAttribute.SetAttributeName(m_AttributeName);
		return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
	}

	AZStd::string	PopcornFXTrackViewAttributeEditorComponent::GetAttributeDescription()
	{
		return AZStd::string("Type ") + O3DEPopcornFXTypeToString((EPopcornFXType)m_TrackViewAttribute.AttributeType());
	}

}
