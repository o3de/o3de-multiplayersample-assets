//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "Integration/PopcornFXIntegrationBus.h"

#include <AzCore/Serialization/SerializeContext.h>

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_attributes.h>
#include <pk_particles/include/ps_descriptor.h>
#include <pk_particles/include/ps_samplers.h>
#endif //O3DE_USE_PK

namespace PopcornFX {

	class PopcornFXAttribute
	{
	public:

		AZ_TYPE_INFO(PopcornFXAttribute, "{8CA706C8-3462-4128-B2B5-27C1026C6E3B}")

		static void	Reflect(AZ::ReflectContext *context);

		AZStd::string	m_Name;
		AZ::u32			m_Type = 0;

	private:
		static bool	VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);
	};

	class PopcornFXSampler
	{
	public:
		AZ_TYPE_INFO(PopcornFXSampler, "{D56CCF14-8FA3-42F0-AB9B-97BA39CCA2BF}")

		static void		Reflect(AZ::ReflectContext *context);

		AZStd::string				m_Name;
		AZ::u32						m_Type = (AZ::u32)-1;
		AZ::EntityId				m_EntityId = (AZ::EntityId)AZ::EntityId::InvalidEntityId;

		bool						m_Ready = false;

	private:
		static bool	VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);
	};

	class PopcornFXAttributeList
		: public PopcornFXSamplerComponentEventsBus::MultiHandler
	{
	public:

		AZ_TYPE_INFO(PopcornFXAttributeList, "{1FF8740B-0F55-42A2-B588-69835BEBAE64}")

		PopcornFXAttributeList();

		static void Reflect(AZ::ReflectContext *context);

		//////////////////////////////////////////////////////////////////////////
		// PopcornFXSamplerComponentEventsBus
		virtual void	OnSamplerReady(const AZ::EntityId &entityId) override;
		//////////////////////////////////////////////////////////////////////////

	protected:
		AZStd::vector<PopcornFXAttribute>	m_Attributes;
		AZStd::vector<PopcornFXSampler>		m_Samplers;
		AZStd::vector<AZ::u8>				m_AttributesRawData;

	private:
		static bool	VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);

#if defined(O3DE_USE_PK)
	public:
		bool						SamplersReady();
		bool						Prepare(const CParticleAttributeList *defaultList);
		SAttributesContainer		*AttributesContainer();
		void						Clear();
		bool						IsEmpty() { return m_Attributes.size() == 0 && m_Samplers.size() == 0; }

		EBaseTypeID									GetAttributeBaseType(CGuid id);
		s32											GetAttributeId(const AZStd::string &name);
		s32											GetAttributeSamplerId(const AZStd::string &name);
		bool										SetSampler(u32 samplerId, AZ::EntityId entityId);
		PopcornFXSampler							*GetSampler(u32 samplerId);
		u32											AttributeCount() const { return static_cast<u32>(m_Attributes.size()); }
		u32											SamplerCount() const { return static_cast<u32>(m_Samplers.size()); }
		const AZStd::vector<PopcornFXSampler>		&Samplers() const { return m_Samplers; }
		TMemoryView<SAttributesContainer::SAttrib>	AttributeRawDataAttributes();

	protected:
		void	_CopyAttributes(const CParticleAttributeList *attributeList);
		void	_CopySamplers(const CParticleAttributeList *attributeList);
		void	_CopyAttributeToRaw(AZ::u32 idxInRaw, const SAttributesContainer::SAttrib &attrib);
		void	_SwapAttributes(AZ::u32 idxA, AZ::u32 idxB);
		void	_SwapRawData(AZ::u32 idxInRawA, AZ::u32 idxInRawB);
		void	_SwapSamplers(AZ::u32 idxA, AZ::u32 idxB);
		void	_SetSamplersStatus();
#endif //O3DE_USE_PK
	};

#if defined(O3DE_USE_PK)
	inline TMemoryView<SAttributesContainer::SAttrib>		PopcornFXAttributeList::AttributeRawDataAttributes()
	{
		return TMemoryView<SAttributesContainer::SAttrib>(reinterpret_cast<SAttributesContainer::SAttrib*>(m_AttributesRawData.data()), AttributeCount());
	}
#endif //O3DE_USE_PK

}
