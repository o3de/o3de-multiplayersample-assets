//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/std/string/string.h>

#if defined(O3DE_USE_PK)

namespace PopcornFX {
	bool	LoadPackPathFromJson(AZStd::string &outPackPath, AZStd::string &outLibraryPath);
	bool	SavePackPathToJson(const AZStd::string &packPath, const AZStd::string &libraryPath);
}

#endif //O3DE_USE_PK
