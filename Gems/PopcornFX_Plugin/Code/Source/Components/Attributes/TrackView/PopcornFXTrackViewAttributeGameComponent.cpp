//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXTrackViewAttributeGameComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace PopcornFX {

	void	PopcornFXTrackViewAttributeGameComponent::Reflect(AZ::ReflectContext *context)
	{
		PopcornFXTrackViewAttribute::Reflect(context);

		auto	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
		if (serializeContext)
		{
			serializeContext->Class<PopcornFXTrackViewAttributeGameComponent, AZ::Component>()
				->Version(1)
				->Field("AttributeName", &PopcornFXTrackViewAttributeGameComponent::m_AttributeName)
				;
		}

		auto	*behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
		if (behaviorContext)
		{
			behaviorContext->Class<PopcornFXTrackViewAttributeGameComponent>()->RequestBus("PopcornFXTrackViewComponentRequestBus");
		}
	}

	PopcornFXTrackViewAttributeGameComponent::PopcornFXTrackViewAttributeGameComponent()
	{
	}

	PopcornFXTrackViewAttributeGameComponent::~PopcornFXTrackViewAttributeGameComponent()
	{
	}

	void	PopcornFXTrackViewAttributeGameComponent::Init()
	{
	}

	void	PopcornFXTrackViewAttributeGameComponent::Activate()
	{
		m_TrackViewAttribute.Activate(GetEntityId(), m_AttributeName);
	}

	void PopcornFXTrackViewAttributeGameComponent::Deactivate()
	{
		m_TrackViewAttribute.Deactivate();
	}

}
