//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXHelperProfilerGameComponent.h"

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Component/TickBus.h>

namespace PopcornFX {

class PopcornFXHelperProfilerEditorComponent
	: public AzToolsFramework::Components::EditorComponentBase
	, public AZ::TickBus::Handler
{
public:
	AZ_EDITOR_COMPONENT(PopcornFXHelperProfilerEditorComponent, "{48BA43E6-5543-430D-AB6F-48A8764DE068}");

	PopcornFXHelperProfilerEditorComponent();

	// AZ::Component interface implementation.
	void			Activate() override;
	void			Deactivate() override;

	static void		Reflect(AZ::ReflectContext *context);

	void			BuildGameEntity(AZ::Entity *gameEntity) override;

	////////////////////////////////////////////////////////////////////////
	// AZ::TickBus::Handler interface implementation
	virtual void	OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
	////////////////////////////////////////////////////////////////////////

	static void		GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
	{
		PopcornFXHelperProfilerGameComponent::GetProvidedServices(provided);
	}

	static void		GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
	{
		PopcornFXHelperProfilerGameComponent::GetIncompatibleServices(incompatible);
	}

	static void		GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &/*required*/)
	{
	}

	static void		GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &/*dependent*/)
	{
	}

protected:
	AZ::u32		OnRecordChanged();
	AZ::u32		OnOpenChanged();

	bool			m_RecordButton = false;
	bool			m_OpenButton = false;
	AZ::u32			m_FrameCountToRecord = 5;
	AZStd::string	m_FileName = "profilereport.pkpr";
	AZStd::string	m_EventName = "PKFXProfile";


	bool			m_Recording = false;
	AZ::u32			m_FrameCount = 0;
};

}
