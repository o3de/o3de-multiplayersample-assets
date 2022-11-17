//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once
// script canvas
#include <ScriptCanvas/Libraries/Libraries.h>


namespace AZ
{
	class ReflectContext;
	class ComponentDescriptor;
} // namespace AZ

namespace PopcornFX {

	//////////////////////////////////////////////////////////////////////////
	/// This defines the Library for Input.
	/// Add your custom nodes like this:
	/// ScriptCanvas::Library::AddNodeToRegistry<InputLibrary, InputNode>(nodeRegistry);
	//////////////////////////////////////////////////////////////////////////
	struct PopcornFXLibrary : public ScriptCanvas::Library::LibraryDefinition
	{
		AZ_RTTI(PopcornFXLibrary, "{1ECB5080-0C20-421D-83DD-6A1C49A5A80F}", ScriptCanvas::Library::LibraryDefinition);

		static void Reflect(AZ::ReflectContext*);
		static void InitNodeRegistry(ScriptCanvas::NodeRegistry &nodeRegistry);
		static AZStd::vector<AZ::ComponentDescriptor*> GetComponentDescriptors();
	};

}
