//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/Math/Color.h>
#include <AzCore/Math/Quaternion.h>

#include "PopcornFXAttributeList.h"

#include <pk_particles/include/ps_descriptor.h>

namespace PopcornFX {

	class PopcornFXEditorAttribute
	{
	public:

		AZ_TYPE_INFO(PopcornFXEditorAttribute, "{5899A5B7-4354-40D9-B96B-12D2F1DE0257}")

		PopcornFXEditorAttribute();

		static void	Reflect(AZ::ReflectContext *context);

		AZ::u32		Id() { return m_Id; }

		void		Copy(AZ::EntityId entityId, const CParticleAttributeDeclaration *attrib, const SAttributesContainer::SAttrib &attribValue, AZ::u32 id);
		void		Reset();
		void		Refresh();

	private:
		//Desc
		AZ::EntityId	m_EntityId;
		AZStd::string	m_Name;
		AZStd::string	m_Description;
		AZ::u32			m_Id;
		AZ::u32			m_Type;
		AZ::u32			m_Semantic;

		//Values
		float			m_ValueFX = 0.0f;
		float			m_ValueFY = 0.0f;
		float			m_ValueFZ = 0.0f;
		float			m_ValueFW = 0.0f;

		AZ::s32			m_ValueIX = 0;
		AZ::s32			m_ValueIY = 0;
		AZ::s32			m_ValueIZ = 0;
		AZ::s32			m_ValueIW = 0;

		bool			m_ValueBX = false;
		bool			m_ValueBY = false;
		bool			m_ValueBZ = false;
		bool			m_ValueBW = false;

		AZ::Quaternion	m_ValueQuat = AZ::Quaternion::CreateIdentity();

		AZ::Color		m_ValueColor = AZ::Color::CreateOne();
		float			m_ValueColorAlpha = 1.0f;

		//Attributes
		float			m_MinValueFX = -FLT_MAX;
		float			m_MinValueFY = -FLT_MAX;
		float			m_MinValueFZ = -FLT_MAX;
		float			m_MinValueFW = -FLT_MAX;
		float			m_MaxValueFX = FLT_MAX;
		float			m_MaxValueFY = FLT_MAX;
		float			m_MaxValueFZ = FLT_MAX;
		float			m_MaxValueFW = FLT_MAX;

		AZ::s32			m_MinValueIX = -INT_MAX;
		AZ::s32			m_MinValueIY = -INT_MAX;
		AZ::s32			m_MinValueIZ = -INT_MAX;
		AZ::s32			m_MinValueIW = -INT_MAX;
		AZ::s32			m_MaxValueIX = INT_MAX;
		AZ::s32			m_MaxValueIY = INT_MAX;
		AZ::s32			m_MaxValueIZ = INT_MAX;
		AZ::s32			m_MaxValueIW = INT_MAX;

		bool			m_ShowValueFX = false;
		bool			m_ShowValueFY = false;
		bool			m_ShowValueFZ = false;
		bool			m_ShowValueFW = false;
		bool			m_ShowValueIX = false;
		bool			m_ShowValueIY = false;
		bool			m_ShowValueIZ = false;
		bool			m_ShowValueIW = false;
		bool			m_ShowValueBX = false;
		bool			m_ShowValueBY = false;
		bool			m_ShowValueBZ = false;
		bool			m_ShowValueBW = false;
		bool			m_ShowValueQuat = false;
		bool			m_ShowValueColor = false;
		bool			m_ShowValueColorAlpha = false;
		bool			m_ResetButton = false;

		static bool	VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement);

		AZ::u32		OnAttributeChanged();
		AZ::u32		OnResetButtonPressed();

		void		_FillAttributeQuat(const SAttributesContainer::SAttrib &attribValue);
		void		_FillAttributeColor(const SAttributesContainer::SAttrib &attribValue);
		void		_FillAttribute(const CParticleAttributeDeclaration *decl, const SAttributesContainer::SAttrib &attribValue, AZ::u32 laneId);
		void		_SetColorValues(const AZ::Color &color);
};

	class PopcornFXEditorSampler
	{
	public:

		AZ_TYPE_INFO(PopcornFXEditorSampler, "{D3CE4B99-FC05-4C0E-BC38-FE8242440708}")

		PopcornFXEditorSampler();

		static void	Reflect(AZ::ReflectContext *context);

		AZ::u32		Id() { return m_Id; }

		void		Copy(AZ::EntityId entityId, const CParticleAttributeSamplerDeclaration *sampler, AZ::EntityId samplerValue, AZ::u32 id);
		void		Reset();
		void		Refresh();

	private:
		//Desc
		AZ::u32			m_Id;
		AZStd::string	m_Name;
		AZStd::string	m_Description;
		AZ::u32			m_Type;
		AZ::EntityId	m_EntityId;

		//Values
		AZ::EntityId	m_SamplerEntityId;

		static bool	VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement);

		AZ::u32		OnSamplerChanged();
	};

	class PopcornFXEditorAttributeCategory
	{
	public:
		AZ_TYPE_INFO(PopcornFXEditorAttributeCategory, "{DC1729A4-11BB-4220-8DDB-C88EB89C0212}")

		PopcornFXEditorAttributeCategory();

		static void	Reflect(AZ::ReflectContext *context);

		void					SetName(const AZStd::string &name) { m_Name = name; }
		const AZStd::string		&Name() const { return m_Name; }

		void					AddAttribute(AZ::EntityId entityId, const CParticleAttributeDeclaration *attrib, const SAttributesContainer::SAttrib &attribValue, AZ::u32 id);
		void					AddSampler(AZ::EntityId entityId, const CParticleAttributeSamplerDeclaration *sampler, AZ::EntityId samplerValue, AZ::u32 id);
		void					ResetAll();
		void					RefreshAttributeIFP(AZ::u32 id);
		void					RefreshSamplerIFP(AZ::u32 id);

	private:
		AZStd::string							m_Name;
		AZStd::vector<PopcornFXEditorAttribute>	m_Attributes;
		AZStd::vector<PopcornFXEditorSampler>	m_Samplers;

		static bool	VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement);
	};

	class PopcornFXEditorAttributeList
	{
	public:

		AZ_TYPE_INFO(PopcornFXEditorAttributeList, "{4A3AD65F-97EB-4835-BF07-681ADD2B2C9B}")

		PopcornFXEditorAttributeList();

		static void Reflect(AZ::ReflectContext *context);

		void					Prepare(const CParticleAttributeList *defaultList, const TMemoryView<SAttributesContainer::SAttrib> attribRawData,
										const AZStd::vector<PopcornFXSampler> &samplers, AZ::EntityId entityId);
		void					Clear();
		void					RefreshAttribute(AZ::u32 attribId);
		void					RefreshSampler(AZ::u32 samplerId);

	private:
		AZStd::vector<PopcornFXEditorAttributeCategory>	m_AttributeCategories;
		bool											m_ResetAllButton = false;

		static bool	VersionConverter(AZ::SerializeContext &context, AZ::SerializeContext::DataElementNode &classElement);

		AZ::u32		OnResetAllButtonPressed();
		CGuid		_GetOrAddCategory(const CStringLocalized &categoryName);
	};

}
