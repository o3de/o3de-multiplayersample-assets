//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFxPlugins.h"

#if defined(O3DE_USE_PK)

#include <pk_kernel/include/kr_plugins.h>

#define	USE_COMPILER_BACKEND

//------------------------------------------------------------------------------
//
//	toolkit to load/unload PKFX runtime plugins
//
//------------------------------------------------------------------------------

#ifdef	PK_PLUGINS_STATIC
#	if defined(USE_COMPILER_BACKEND)
PK_PLUGIN_DECLARE(CCompilerBackendCPU_VM);
#	endif

#	if defined(PK_DEBUG)
#		define	PK_PLUGIN_POSTFIX_BUILD			"_D"
#	else
#		define	PK_PLUGIN_POSTFIX_BUILD			""
#	endif
#	if	defined(PK_WINDOWS) || defined(PK_X360)
#		define	PK_PLUGIN_POSTFIX_EXT			".dll"
#	else
#		define	PK_PLUGIN_POSTFIX_EXT			""
#	endif
#endif

//------------------------------------------------------------------------------
namespace PopcornFX {
	static u32	g_LoadedPlugins = 0;

	//-----------------------------------------------------------------------------
	//
	//	Loads selected plugins
	//
	//-----------------------------------------------------------------------------

	bool	PopcornRegisterPlugins(u32 selected /*= 0*/)
	{
		PK_ASSERT(g_LoadedPlugins == 0);

		bool	success = true;
#	ifndef	PK_PLUGINS_STATIC
		// plugins are .dll
		PopcornFX::CPluginManager::RegisterDirectory("Plugins", false);
#	else
		// plugins are linked statically
		if (selected & EPlugin_CompilerBackendVM)
		{
			const char	*backendPath = "Plugins/CBCPU_VM_Win32" PK_PLUGIN_POSTFIX_BUILD PK_PLUGIN_POSTFIX_EXT;
			IPluginModule	*backend = StartupPlugin_CCompilerBackendCPU_VM();
			success &= (backend != null && PopcornFX::CPluginManager::PluginRegister(backend, true, backendPath));
		}
#	endif

		g_LoadedPlugins = selected;
		return success;
	}

	//-----------------------------------------------------------------------------
	//
	//	Unloads all plugins
	//
	//-----------------------------------------------------------------------------

	void	PopcornUnregisterPlugins()
	{
		// unregister plugins:
#	ifdef	PK_PLUGINS_STATIC
		if (g_LoadedPlugins & EPlugin_CompilerBackendVM)
		{
			IPluginModule	*backend = GetPlugin_CCompilerBackendCPU_VM();
			(backend != null && PopcornFX::CPluginManager::PluginRelease(backend));
			ShutdownPlugin_CCompilerBackendCPU_VM();
		}
#	endif

		g_LoadedPlugins = 0;
	}

}
//------------------------------------------------------------------------------

#endif //O3DE_USE_PK

