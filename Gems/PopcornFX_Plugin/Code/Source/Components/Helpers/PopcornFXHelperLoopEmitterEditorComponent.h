//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once


#include "PopcornFXHelperLoopEmitterGameComponent.h"

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Component/TickBus.h>
#include <PopcornFX/PopcornFXBus.h>

namespace PopcornFX {

class PopcornFXHelperLoopEmitterEditorComponent
	: public AzToolsFramework::Components::EditorComponentBase
	, public AZ::TickBus::Handler
{
public:
	AZ_EDITOR_COMPONENT(PopcornFXHelperLoopEmitterEditorComponent, "{E6103342-D393-4D0D-8256-D032F97A7C51}");

	PopcornFXHelperLoopEmitterEditorComponent();

	// AZ::Component interface implementation.
	void	Init() override;
	void	Activate() override;
	void	Deactivate() override;

	static void	Reflect(AZ::ReflectContext *context);

	void	BuildGameEntity(AZ::Entity *gameEntity) override;

	////////////////////////////////////////////////////////////////////////
	// AZ::TickBus::Handler interface implementation
	virtual void	OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
	////////////////////////////////////////////////////////////////////////

	static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		PopcornFXHelperLoopEmitterGameComponent::GetProvidedServices(provided);
	}

	static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		PopcornFXHelperLoopEmitterGameComponent::GetIncompatibleServices(incompatible);
	}

	static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
	{
		PopcornFXHelperLoopEmitterGameComponent::GetRequiredServices(required);
	}

	static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &/*dependent*/)
	{
	}

protected:
	AZ::u32	OnLoopInEditorChanged();

	bool	m_LoopInEditor = false;
	bool	m_KillOnRestart = false;
	float	m_Delay = 2.0f;

	float	m_CurrentTime = 0.0f;

};

}
