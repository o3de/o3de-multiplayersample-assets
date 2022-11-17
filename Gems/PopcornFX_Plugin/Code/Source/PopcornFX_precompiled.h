//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#undef	PV_MODULE_NAME
#undef	PV_MODULE_SYM
#define	PV_MODULE_NAME		"O3DE"
#define	PV_MODULE_SYM		O3DE

#include <pkapi/include/pk_precompiled_default.h>
#include <pk_version.h>

PK_LOG_MODULE_DEFINE();

#else

#define	PK_VERSION_CURRENT_STRING	"0.0.0.0"

#endif //O3DE_USE_PK

#if defined(POPCORNFX_EDITOR)

#if (defined(PK_MACOSX) || defined(PK_LINUX) || defined(PK_WINDOWS)) && defined(PK_CPU_64BITS)
#	define	USE_FBXIMPORTER
#endif

#endif //POPCORNFX_EDITOR

#if defined(PK_WINDOWS)
#	pragma warning( disable : 4266 )
#endif

#include <AzCore/PlatformIncl.h>
