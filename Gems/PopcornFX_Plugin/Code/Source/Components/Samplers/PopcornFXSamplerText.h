//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "Integration/PopcornFXIntegrationBus.h"

#include <AzCore/Serialization/SerializeContext.h>

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_text.h>
#endif //O3DE_USE_PK


namespace PopcornFX
{
	class CParticleAttributeSamplerDeclaration;
}

namespace PopcornFX {

	class PopcornFXSamplerText
		: public PopcornFXSamplerComponentRequestBus::Handler
	{
	public:
		AZ_TYPE_INFO(PopcornFXSamplerText, "{BA964576-2207-4A47-B077-63496F8D0368}")

		static void		Reflect(AZ::ReflectContext *context);

		void			CopyFrom(const PopcornFXSamplerText &other);

		void			Activate();
		void			Deactivate();

		AZ::EntityId	m_AttachedToEntityId;

	protected:
		void			OnDataChanged();
		AZStd::string	m_Text;

	private:
		static bool	VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);

#if defined(O3DE_USE_PK)
	public:
		//////////////////////////////////////////////////////////////////////////
		// PopcornFXSamplerComponentRequestBus interface implementation
		PopcornFX::CParticleSamplerDescriptor	*GetDescriptor() override { return m_SamplerDescriptor.Get(); }
		virtual AZ::u32							GetType() override { return CParticleSamplerDescriptor_Text::SamplerTypeID(); };
		//////////////////////////////////////////////////////////////////////////


	protected:
		void	_Clean();
		bool	_BuildDescriptor();

		PParticleSamplerDescriptor	m_SamplerDescriptor = null;
#endif //O3DE_USE_PK
};

}
