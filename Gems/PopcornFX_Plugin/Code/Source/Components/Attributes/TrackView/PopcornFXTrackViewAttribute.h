//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <PopcornFX/PopcornFXBus.h>

namespace PopcornFX {

	class PopcornFXTrackViewAttribute
		: public PopcornFXEmitterComponentEventsBus::Handler
		, public PopcornFXTrackViewComponentRequestBus::Handler
	{
	public:
		AZ_TYPE_INFO(PopcornFXTrackViewAttribute, "{78928AA8-8999-4778-A5B7-93E44DD4B94E}")

		static void	Reflect(AZ::ReflectContext *context);

		PopcornFXTrackViewAttribute();
		~PopcornFXTrackViewAttribute();

		void	Activate(AZ::EntityId entityId, const AZStd::string &attributeName);
		void	Deactivate();

		void	SetAttributeName(const AZStd::string &name);

		AZ::s32	AttributeType() { return m_AttributeType; }

		// PopcornFXEmitterComponentEventsBus::Handler
		void	OnEmitterReady() override;

		// PopcornFXTrackViewComponentRequestBus::Handler
		void	SetValueFX(float value) override;
		float	GetValueFX() const override;
		void	SetValueFY(float value) override;
		float	GetValueFY() const override;
		void	SetValueFZ(float value) override;
		float	GetValueFZ() const override;
		void	SetValueFW(float value) override;
		float	GetValueFW() const override;

		void	SetValueIX(AZ::u32 value) override;
		AZ::u32	GetValueIX() const override;
		void	SetValueIY(AZ::u32 value) override;
		AZ::u32	GetValueIY() const override;
		void	SetValueIZ(AZ::u32 value) override;
		AZ::u32	GetValueIZ() const override;
		void	SetValueIW(AZ::u32 value) override;
		AZ::u32	GetValueIW() const override;

		void	SetValueBX(bool value) override;
		bool	GetValueBX() const override;
		void	SetValueBY(bool value) override;
		bool	GetValueBY() const override;
		void	SetValueBZ(bool value) override;
		bool	GetValueBZ() const override;
		void	SetValueBW(bool value) override;
		bool	GetValueBW() const override;

		void			SetValueQuat(const AZ::Quaternion &value) override;
		AZ::Quaternion	GetValueQuat() const override;
		void			SetValueColor(const AZ::Color &value) override;
		AZ::Color		GetValueColor() const override;
		void			SetValueColorAlpha(float value) override;
		float			GetValueColorAlpha() const override;

	private:
		bool	AttributeValid() const;
		void	GetAttributeIdAndType();

		void	SetValue(float value, AZ::u32 id);
		void	SetValue(AZ::u32 value, AZ::u32 id);
		void	SetValue(bool value, AZ::u32 id);

		AZ::EntityId	m_ParentId;
		AZStd::string	m_AttributeName;
		bool			m_FxCreated = false;
		AZ::s32			m_AttributeId = (AZ::s32)-1;
		AZ::s32			m_AttributeType = (AZ::s32)-1;
	};

}
