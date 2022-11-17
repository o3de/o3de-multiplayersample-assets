//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "AtomFrameCollector.h"

#if defined(O3DE_USE_PK)

namespace PopcornFX {
//----------------------------------------------------------------------------

CAtomFrameCollector::CAtomFrameCollector()
{

}

CAtomFrameCollector::~CAtomFrameCollector()
{

}

bool	CAtomFrameCollector::EarlyCull(const PopcornFX::CAABB &bbox) const
{
	// Can happen if bounds are not active
	if (!bbox.IsFinite() ||
		!bbox.Valid())
		return false;
	return false;
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
