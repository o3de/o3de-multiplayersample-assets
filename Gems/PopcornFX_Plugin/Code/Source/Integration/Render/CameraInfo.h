//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

namespace PopcornFX {

struct SSceneViews
{
	struct SView
	{
		CFloat4x4		m_ViewMatrix;
		CFloat4x4		m_InvViewMatrix;
		CFloat4x4		m_ViewProjMatrix;
	};

	PopcornFX::TArray<SView>	m_Views;
};

}

#endif //O3DE_USE_PK

