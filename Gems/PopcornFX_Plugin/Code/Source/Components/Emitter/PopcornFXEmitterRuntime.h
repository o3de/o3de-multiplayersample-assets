//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/Asset/AssetTypeInfoBus.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Transform.h>
#include <PopcornFX/PopcornFXBus.h>

#include "Asset/PopcornFXAsset.h"
#include "Components/Attributes/PopcornFXAttributeList.h"
#include "PopcornFXEmitter.h"

namespace PopcornFX {

	class PopcornFXEmitterRuntime
		: public AZ::Data::AssetBus::Handler
	{
	public:
		/**
		* Specifies that this class should use AZ::SystemAllocator for memory
		* management by default.
		*/
		AZ_CLASS_ALLOCATOR(PopcornFXEmitterRuntime, AZ::SystemAllocator, 0);

		AZ_TYPE_INFO(PopcornFXEmitterRuntime, "{2546CABA-4B3E-444C-8C9A-D490E79CEE7E}")

		PopcornFXEmitterRuntime();

		static void	Reflect(AZ::ReflectContext *reflection);

		//Init from PopcornFXEmitterGameComponent
		void	Init(bool enable, const PopcornFXAttributeList &attributeList);
		//Init from PopcornFXSystemComponent
		void	Init(bool enable, StandaloneEmitter *standaloneEmitter);

		void	Activate();
		void	Deactivate();

		// AZ::Data::AssetBus::Handler
		void	OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
		void	OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

		void	SetAsset(const AZ::Data::Asset<PopcornFXAsset> &asset) { m_Asset = asset; }

		//Only for StandaloneEmitter
		void	SetAssetId(AZ::Data::AssetId assetId) { m_AssetId = assetId; }
		void	SetSpawnAttributes(	const AZStd::vector<PopcornFX::SpawnParams::Attribute> &attributes,
									const AZStd::vector<PopcornFX::SpawnParams::AttributeSampler> &samplers)
		{
			m_StandaloneEmitterAttributes = attributes;
			m_StandaloneEmitterAttributeSamplers = samplers;
		}

#if !defined(O3DE_USE_PK)
		void	SetTransform(const AZ::Transform &transform) { }
		void	AttachToEntity(AZ::EntityId entityId) { }
		void	EnableAutoRemove(bool enable) { }
#else
		void	SetTransform(const AZ::Transform &transform) { m_Emitter.SetTransform(transform); }
		void	AttachToEntity(AZ::EntityId entityId) { m_EntityId = entityId; m_Emitter.AttachToEntity(entityId, m_StandaloneEmitter != null); }
		void	EnableAutoRemove(bool enable) { m_Emitter.EnableAutoRemove(enable ? m_StandaloneEmitter : null); }

		PopcornFXEmitter	&Emitter() { return m_Emitter; }
#endif //O3DE_USE_PK

		void		Enable(bool enable);
		bool		IsEnabled() { return m_Enable; }

		void			SetVisible(bool visible);
		bool			GetVisible();
		void			SetTimeScale(float timeScale);
		float			GetTimeScale();
		void			SetPrewarmEnable(bool enable);
		bool			GetPrewarmEnable();
		void			SetPrewarmTime(float time);
		float			GetPrewarmTime();
		bool			SetAttributeAsFloat(AZ::u32 attribId, float value);
		bool			SetAttributeAsFloat2(AZ::u32 attribId, const AZ::Vector2 &value);
		bool			SetAttributeAsFloat3(AZ::u32 attribId, const AZ::Vector3 &value);
		bool			SetAttributeAsFloat4(AZ::u32 attribId, const AZ::Vector4 &value);
		bool			SetAttributeAsInt(AZ::u32 attribId, AZ::s32 value);
		bool			SetAttributeAsInt2(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY);
		bool			SetAttributeAsInt3(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ);
		bool			SetAttributeAsInt4(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW);
		bool			SetAttributeAsBool(AZ::u32 attribId, bool value);
		bool			SetAttributeAsBool2(AZ::u32 attribId, bool valueX, bool valueY);
		bool			SetAttributeAsBool3(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ);
		bool			SetAttributeAsBool4(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW);
		bool			SetAttributeAsQuaternion(AZ::u32 attribId, const AZ::Quaternion &value);
		bool			SetAttributeAsColor(AZ::u32 attribId, const AZ::Color &value);
		float			GetAttributeAsFloat(AZ::u32 attribId);
		AZ::Vector2		GetAttributeAsFloat2(AZ::u32 attribId);
		AZ::Vector3		GetAttributeAsFloat3(AZ::u32 attribId);
		AZ::Vector4		GetAttributeAsFloat4(AZ::u32 attribId);
		AZ::s32			GetAttributeXAsInt(AZ::u32 attribId);
		AZ::s32			GetAttributeYAsInt(AZ::u32 attribId);
		AZ::s32			GetAttributeZAsInt(AZ::u32 attribId);
		AZ::s32			GetAttributeWAsInt(AZ::u32 attribId);
		bool			GetAttributeXAsBool(AZ::u32 attribId);
		bool			GetAttributeYAsBool(AZ::u32 attribId);
		bool			GetAttributeZAsBool(AZ::u32 attribId);
		bool			GetAttributeWAsBool(AZ::u32 attribId);
		AZ::Quaternion	GetAttributeAsQuaternion(AZ::u32 attribId);
		AZ::Color		GetAttributeAsColor(AZ::u32 attribId);

	protected:
		bool							m_Enable = false;
		bool							m_Visible = true;
		float							m_TimeScale = 1.0f;
		bool							m_PrewarmEnable = false;
		float							m_PrewarmTime = 0.0f;
#if defined(O3DE_USE_PK)
		PopcornFXEmitter				m_Emitter;
#endif //O3DE_USE_PK
		PopcornFXAttributeList			m_AttributeList;

		//Only for StandaloneEmitter
		StandaloneEmitter										*m_StandaloneEmitter = null;
		AZStd::vector<PopcornFX::SpawnParams::Attribute>		m_StandaloneEmitterAttributes;
		AZStd::vector<PopcornFX::SpawnParams::AttributeSampler>	m_StandaloneEmitterAttributeSamplers;

		AZ::EntityId					m_EntityId;
		AZ::Data::Asset<PopcornFXAsset>	m_Asset;
		AZ::Data::AssetId				m_AssetId;

		bool	_LoadAndSet();
		void	_SetupStandaloneEmitterAttributes();
		bool	_SetupAsset();

	private:
		static bool VersionConverter(	AZ::SerializeContext &context,
										AZ::SerializeContext::DataElementNode &classElement);
	};

}
