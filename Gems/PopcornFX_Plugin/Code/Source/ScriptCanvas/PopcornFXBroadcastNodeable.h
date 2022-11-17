//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

// script canvas
#include <ScriptCanvas/Core/Nodeable.h>
#include <ScriptCanvas/Core/NodeableNode.h>
#include <ScriptCanvas/CodeGen/NodeableCodegen.h>
#include <PopcornFX/PopcornFXBus.h>

#include <Source/ScriptCanvas/PopcornFXBroadcastNodeable.generated.h>

//namespace PopcornFX {
namespace PopcornFX
{

	//////////////////////////////////////////////////////////////////////////
	class PopcornFXBroadcastNodeable
		: public ScriptCanvas::Nodeable
		, protected PopcornFX::PopcornFXEmitterComponentEventsBus::Handler
	{
		SCRIPTCANVAS_NODE(PopcornFXBroadcastNodeable)

	public:
		PopcornFXBroadcastNodeable() = default;
		virtual ~PopcornFXBroadcastNodeable();
		PopcornFXBroadcastNodeable(const PopcornFXBroadcastNodeable&) = default;
		PopcornFXBroadcastNodeable &operator=(const PopcornFXBroadcastNodeable&) = default;

	protected:
		void OnDeactivate() override;

		//////////////////////////////////////////////////////////////////////////
		/// PopcornFX::PopcornFXEventsBus::Handler
		void OnEmitterReady() override;
		void OnEmitterBroadcastEvent(PopcornFX::SBroadcastParams *params) override;

		AZStd::string	m_EventName;
		AZ::EntityId	m_EntityId;
	};

}
//}
