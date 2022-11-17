//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <AzCore/Component/TickBus.h>

namespace PopcornFX {

class PopcornFXIntegration;

	class PopcornFXStartUpdate
		: protected AZ::TickBus::Handler
	{
	public:
		void	Activate(PopcornFXIntegration *integration);
		void	Deactivate();

	protected:
		virtual int					GetTickOrder() override;
		virtual void				OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		PopcornFXIntegration		*m_Integration = null;
	};

}

#endif //O3DE_USE_PK

