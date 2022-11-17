//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXEmitterRuntime.h"

#include <PopcornFX/PopcornFXBus.h>

#include <AzCore/Component/Component.h>

namespace PopcornFX {

	class PopcornFXEmitterGameComponent
		: public AZ::Component
#if defined(O3DE_USE_PK)
		, public PopcornFXEmitterComponentRequestBus::Handler
#endif //O3DE_USE_PK
{
	public:
		AZ_COMPONENT(PopcornFXEmitterGameComponent, EmitterComponentTypeId);

		PopcornFXEmitterGameComponent();

		//////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void	Activate() override;
		void	Deactivate() override;
		//////////////////////////////////////////////////////////////////////////


		static void	Reflect(AZ::ReflectContext *reflection);

		static void	GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
		{
			provided.push_back(AZ_CRC("PopcornFXEmitterService"));
		}

		static void	GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
		{
			incompatible.push_back(AZ_CRC("PopcornFXEmitterService"));
		}

		static void	GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
		{
			required.push_back(AZ_CRC("TransformService"));
		}

		static void	GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &/*dependent*/)
		{
		}

		PopcornFXEmitterRuntime	m_Emitter;

#if defined(O3DE_USE_PK)
		//////////////////////////////////////////////////////////////////////////
		// PopcornFXEmitterComponentRequestBus
		virtual void			Enable(bool enable) override;
		virtual bool			Start() override;
		virtual bool			Stop() override;
		virtual bool			Terminate() override;
		virtual bool			Kill() override;
		virtual bool			Restart(bool killOnRestart) override;
		virtual bool			IsLoaded() override;
		virtual bool			IsEnabled() override;
		virtual bool			IsPlaying() override;
		virtual bool			SetupEmitterById(const AZ::Data::AssetId &assetId, bool enable) override;
		virtual bool			SetupEmitterByName(const AZStd::string &path, bool enable) override;
		virtual void			SetVisible(bool visible) override;
		virtual bool			GetVisible() override;
		virtual void			SetTimeScale(float timeScale) override;
		virtual float			GetTimeScale() override;
		virtual void			SetPrewarmEnable(bool enable) override;
		virtual bool			GetPrewarmEnable() override;
		virtual void			SetPrewarmTime(float time) override;
		virtual float			GetPrewarmTime() override;
		virtual AZ::u32			GetAttributesCount() override;
		virtual AZStd::string	GetAttributeName(AZ::u32 attribId) override;
		virtual AZ::s32			GetAttributeType(AZ::u32 attribId) override;
		virtual AZ::s32			GetAttributeId(const AZStd::string &name) override;
		virtual bool			ResetAttributeToDefault(AZ::u32 attribId) override;

		virtual bool			SetAttributeAsFloat(AZ::u32 attribId, float value) override;
		virtual bool			SetAttributeAsFloat2(AZ::u32 attribId, const AZ::Vector2 &value) override;
		virtual bool			SetAttributeAsFloat3(AZ::u32 attribId, const AZ::Vector3 &value) override;
		virtual bool			SetAttributeAsFloat4(AZ::u32 attribId, const AZ::Vector4 &value) override;
		virtual bool			SetAttributeAsInt(AZ::u32 attribId, AZ::s32 value) override;
		virtual bool			SetAttributeAsInt2(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY) override;
		virtual bool			SetAttributeAsInt3(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ) override;
		virtual bool			SetAttributeAsInt4(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW) override;
		virtual bool			SetAttributeAsBool(AZ::u32 attribId, bool value) override;
		virtual bool			SetAttributeAsBool2(AZ::u32 attribId, bool valueX, bool valueY) override;
		virtual bool			SetAttributeAsBool3(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ) override;
		virtual bool			SetAttributeAsBool4(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW) override;
		virtual bool			SetAttributeAsQuaternion(AZ::u32 attribId, const AZ::Quaternion &value) override;
		virtual bool			SetAttributeAsColor(AZ::u32 attribId, const AZ::Color &value) override;

		virtual float			GetAttributeAsFloat(AZ::u32 attribId) override;
		virtual AZ::Vector2		GetAttributeAsFloat2(AZ::u32 attribId) override;
		virtual AZ::Vector3		GetAttributeAsFloat3(AZ::u32 attribId) override;
		virtual AZ::Vector4		GetAttributeAsFloat4(AZ::u32 attribId) override;
		virtual AZ::s32			GetAttributeXAsInt(AZ::u32 attribId) override;
		virtual AZ::s32			GetAttributeYAsInt(AZ::u32 attribId) override;
		virtual AZ::s32			GetAttributeZAsInt(AZ::u32 attribId) override;
		virtual AZ::s32			GetAttributeWAsInt(AZ::u32 attribId) override;
		virtual bool			GetAttributeXAsBool(AZ::u32 attribId) override;
		virtual bool			GetAttributeYAsBool(AZ::u32 attribId) override;
		virtual bool			GetAttributeZAsBool(AZ::u32 attribId) override;
		virtual bool			GetAttributeWAsBool(AZ::u32 attribId) override;
		virtual AZ::Quaternion	GetAttributeAsQuaternion(AZ::u32 attribId) override;
		virtual AZ::Color		GetAttributeAsColor(AZ::u32 attribId) override;

		virtual AZ::u32			GetAttributeSamplersCount() override;
		virtual AZ::s32			GetAttributeSamplerId(const AZStd::string &name) override;
		virtual bool			SetAttributeSampler(AZ::u32 attribSamplerId, AZ::EntityId entityId) override;
		virtual bool			ResetSamplerToDefault(AZ::u32 attribSamplerId) override;
		virtual AZ::EntityId	GetAttributeSampler(AZ::u32 attribSamplerId) override;

		virtual void			SetTeleportThisFrame() override;

		//////////////////////////////////////////////////////////////////////////
#endif //O3DE_USE_PK
};

}
