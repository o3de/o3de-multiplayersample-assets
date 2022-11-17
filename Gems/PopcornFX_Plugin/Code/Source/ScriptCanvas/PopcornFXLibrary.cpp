//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "PopcornFXLibrary.h"

#include <AzCore/Serialization/EditContext.h>

// script canvas
#include <ScriptCanvas/Libraries/Libraries.h>

#include "PopcornFXBroadcastNodeable.h"
#include "PopcornFXExtractPayloadNodes.h"

namespace PopcornFX {

	void	PopcornFXLibrary::Reflect(AZ::ReflectContext *reflection)
	{
		AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);
		if (serializeContext)
		{
			serializeContext->Class<PopcornFXLibrary, LibraryDefinition>()
				->Version(2)
				;

			AZ::EditContext	*editContext = serializeContext->GetEditContext();
			if (editContext)
			{
				editContext->Class<PopcornFXLibrary>("PopcornFX", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Script::Attributes::Category, "PopcornFX")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					;
			}
		}

		if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			SCRIPT_CANVAS_GENERICS_TO_VM(ExtractPayloadNodes::Registrar, ExtractPayloadNodes::ExtractPayload, behaviorContext, ExtractPayloadNodes::k_categoryName);
		}
	}

	void	PopcornFXLibrary::InitNodeRegistry(ScriptCanvas::NodeRegistry &nodeRegistry)
	{
		ExtractPayloadNodes::Registrar::AddToRegistry<PopcornFXLibrary>(nodeRegistry);
		ScriptCanvas::Library::AddNodeToRegistry<PopcornFXLibrary, Nodes::PopcornFXBroadcastNodeableNode>(nodeRegistry);
	}

	AZStd::vector<AZ::ComponentDescriptor*>	PopcornFXLibrary::GetComponentDescriptors()
	{
		AZStd::vector<AZ::ComponentDescriptor*> descriptors;

		ExtractPayloadNodes::Registrar::AddDescriptors(descriptors);
		descriptors.push_back(Nodes::PopcornFXBroadcastNodeableNode::CreateDescriptor());

		return descriptors;
	}

}
