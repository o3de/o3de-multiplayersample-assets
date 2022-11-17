//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXHelperProfilerGameComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#if defined(PK_USE_STARTINGPOINTINPUT)
	#include <StartingPointInput/InputEventNotificationBus.h>
#endif

namespace PopcornFX {

PopcornFXHelperProfilerGameComponent::PopcornFXHelperProfilerGameComponent()
{
}

void	PopcornFXHelperProfilerGameComponent::Activate()
{
	AZ::TickBus::Handler::BusConnect();
#if defined(PK_USE_STARTINGPOINTINPUT)
	StartingPointInput::InputEventNotificationId	inputBusId = StartingPointInput::InputEventNotificationId(m_EventName.c_str());
	StartingPointInput::InputEventNotificationBus::Handler::BusConnect(inputBusId);
#endif
}

void	PopcornFXHelperProfilerGameComponent::Deactivate()
{
	if (AZ::TickBus::Handler::BusIsConnected())
		AZ::TickBus::Handler::BusDisconnect();
#if defined(PK_USE_STARTINGPOINTINPUT)
	StartingPointInput::InputEventNotificationId	inputBusId = StartingPointInput::InputEventNotificationId(m_EventName.c_str());
	StartingPointInput::InputEventNotificationBus::Handler::BusDisconnect(inputBusId);
#endif
}

void	PopcornFXHelperProfilerGameComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	(void)time; (void)deltaTime;

	if (m_Recording)
	{
		m_FrameCount++;
		if (m_FrameCount >= m_FrameCountToRecord)
		{
			AZ_Printf("PopcorFX", "Stopping the profiler. ");

			const AZStd::string	fullEntryPath = PopcornFXHelperProfilerGameComponent::GetFullPath(m_FileName);
			bool				writeOk = false;

			PopcornFXProfilerRequestBus::BroadcastResult(writeOk, &PopcornFXProfilerRequestBus::Events::WriteProfileReport, fullEntryPath);
			if (writeOk)
			{
				AZ_Printf("PopcorFX", "Profile report wrote to %s.", fullEntryPath.c_str());
			}
			else
			{
				AZ_Printf("PopcorFX", "Could not open file %s to write profile report.", fullEntryPath.c_str());
			}

			PopcornFXProfilerRequestBus::Broadcast(&PopcornFXProfilerRequestBus::Events::ProfilerSetEnable, false);
			m_Recording = false;
			m_FrameCount = 0;
		}
	}
}

#if defined(PK_USE_STARTINGPOINTINPUT)
void	PopcornFXHelperProfilerGameComponent::OnPressed(float /*value*/)
{
	if (!m_Recording)
	{
		PopcornFXProfilerRequestBus::Broadcast(&PopcornFXProfilerRequestBus::Events::ProfilerSetEnable, true);
		m_Recording = true;
		AZ_Printf("PopcorFX", "Starting the profiler.");
	}
	else
	{
		AZ_Printf("PopcorFX", "The profiler is already running.");
	}
}
#endif

void	PopcornFXHelperProfilerGameComponent::Reflect(AZ::ReflectContext *reflection)
{
	if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		// Serializer:
		serializeContext->Class<PopcornFXHelperProfilerGameComponent, AZ::Component>()
			->Version(1)
			->Field("FrameCountToRecord", &PopcornFXHelperProfilerGameComponent::m_FrameCountToRecord)
			->Field("FileName", &PopcornFXHelperProfilerGameComponent::m_FileName)
			->Field("EventName", &PopcornFXHelperProfilerGameComponent::m_EventName)
			;
	}
}

}
