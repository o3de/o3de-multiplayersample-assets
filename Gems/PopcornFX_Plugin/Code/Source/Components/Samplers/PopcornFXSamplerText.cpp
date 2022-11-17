//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerText.h"

#include <AzCore/Serialization/EditContext.h>

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_classes.h>
#include <pk_particles/include/ps_attributes.h>
#endif //O3DE_USE_PK


namespace PopcornFX {

	void PopcornFXSamplerText::Reflect(AZ::ReflectContext *context)
	{
		AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
		if (serializeContext)
		{
			serializeContext->Class<PopcornFXSamplerText>()
				->Version(1, &VersionConverter)
				->Field("Text", &PopcornFXSamplerText::m_Text)
				;

			// edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXSamplerText>("PopcornFX Sampler Text", "")
					->DataElement(0, &PopcornFXSamplerText::m_Text, "", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerText::OnDataChanged)
					;
			}
		}
	}

	// Private Static
	bool	PopcornFXSamplerText::VersionConverter(	AZ::SerializeContext &context,
													AZ::SerializeContext::DataElementNode &classElement)
	{
		(void)context; (void)classElement;
		return true;
	}

	void	PopcornFXSamplerText::CopyFrom(const PopcornFXSamplerText &other)
	{
		m_Text = other.m_Text;
	}

#if !defined(O3DE_USE_PK)

	void	PopcornFXSamplerText::Activate()
	{
	}

	void	PopcornFXSamplerText::Deactivate()
	{
	}

	void	PopcornFXSamplerText::OnDataChanged()
	{
	}

#else

	void	PopcornFXSamplerText::Activate()
	{
		PopcornFXSamplerComponentRequestBus::Handler::BusConnect(m_AttachedToEntityId);
		OnDataChanged();
	}

	void	PopcornFXSamplerText::Deactivate()
	{
		PopcornFXSamplerComponentRequestBus::Handler::BusDisconnect(m_AttachedToEntityId);
		_Clean();
	}

	void	PopcornFXSamplerText::OnDataChanged()
	{
		if (!PK_VERIFY(_BuildDescriptor()))
		{
			AZ_Error("PopcornFX", false, "Build descriptor failed for Sampler Text");
		}
	}

	bool	PopcornFXSamplerText::_BuildDescriptor()
	{
		if (m_SamplerDescriptor == null)
		{
			m_SamplerDescriptor = PK_NEW(CParticleSamplerDescriptor_Text_Default());
			if (!PK_VERIFY(m_SamplerDescriptor != null))
				return false;
		}
		PParticleSamplerDescriptor_Text_Default	desc = reinterpret_cast<CParticleSamplerDescriptor_Text_Default*>(m_SamplerDescriptor.Get());
		if (!PK_VERIFY(desc != null))
			return false;

		CString text(m_Text.c_str());
		if (!PK_VERIFY(desc->_Setup(text, null, false)))
			return false;

		PopcornFXSamplerComponentEventsBus::Event(m_AttachedToEntityId, &PopcornFXSamplerComponentEventsBus::Events::OnSamplerReady, m_AttachedToEntityId);

		return true;
	}

	void	PopcornFXSamplerText::_Clean()
	{
		if (m_SamplerDescriptor != null)
		{
			PParticleSamplerDescriptor_Text_Default	desc = reinterpret_cast<CParticleSamplerDescriptor_Text_Default*>(m_SamplerDescriptor.Get());
			if (PK_VERIFY(desc != null))
			{
				desc->_Setup("", null, false);
			}
			m_SamplerDescriptor = null;
		}
	}

#endif //O3DE_USE_PK

}
