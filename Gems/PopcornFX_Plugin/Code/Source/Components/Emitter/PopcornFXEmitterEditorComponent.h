//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "PopcornFXEmitter.h"
#include "PopcornFXEmitterGameComponent.h"
#include "Components/Attributes/PopcornFXAttributeList.h"
#include "Components/Attributes/PopcornFXEditorAttributeList.h"
#include "Asset/PopcornFXAsset.h"

#include <PopcornFX/PopcornFXBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzToolsFramework/ToolsComponents/EditorVisibilityBus.h>

namespace PopcornFX {

	class PopcornFXEmitterEditorComponent
		: public AzToolsFramework::Components::EditorComponentBase
		, private AZ::Data::AssetBus::Handler
		, public PopcornFXEmitterComponentRequestBus::Handler
		, private AzFramework::AssetCatalogEventBus::Handler
		, public AZ::TickBus::Handler
		, private AzToolsFramework::EditorVisibilityNotificationBus::Handler

	{
	public:
		AZ_EDITOR_COMPONENT(PopcornFXEmitterEditorComponent, "{B62ED02E-731B-4ACD-BCA1-78EF92528228}");

		PopcornFXEmitterEditorComponent();

		static void	GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
		{
			PopcornFXEmitterGameComponent::GetProvidedServices(provided);
		}

		static void	GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible)
		{
			PopcornFXEmitterGameComponent::GetIncompatibleServices(incompatible);
		}

		static void	GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required)
		{
			PopcornFXEmitterGameComponent::GetRequiredServices(required);
		}

		static void	GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &/*dependent*/)
		{
		}

		// AZ::Component interface implementation.
		void	Init() override;
		void	Activate() override;
		void	Deactivate() override;

		void	BuildGameEntity(AZ::Entity *gameEntity) override;

		//! Called when you want to change the game asset through code (like when creating components based on assets).
		void	SetPrimaryAsset(const AZ::Data::AssetId &assetId) override;

		//! Invoked in the editor when the user assigns a new asset.
		void	OnAssetChanged();

	protected:
		// Required Reflect function.
		static void	Reflect(AZ::ReflectContext *context);

		AZ::u32			OnEnableChanged();
		AZ::u32			OnStartStopButtonPressed();
		AZ::u32			OnRestartButtonPressed();
		AZ::u32			OnKillButtonPressed();
		AZ::u32			OnResetPrewarmButtonPressed();

		AZStd::string	StartStopButtonText() const;
		AZ::Crc32		GetButtonsVisibility() const;
		bool			IsPrewarmTimeReadOnly() const;

	public:
		////////////////////////////////////////////////////////////////////////
		// AZ::TickBus::Handler interface implementation
		virtual void	OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// EditorVisibilityNotificationBus
		void			OnEntityVisibilityChanged(bool visibility) override;
		////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// AssetEventHandler
		void			OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
		void			OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// AssetCatalogEventBus
		void			OnCatalogAssetAdded(const AZ::Data::AssetId &assetId) override;
		//////////////////////////////////////////////////////////////////////////

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

protected:
		bool							m_Enable = true;
		bool							m_IsPlaying = false;
		bool							m_UpdateUI = false;
		bool							m_StartStopButton = false;
		bool							m_RestartButton = false;
		bool							m_KillButton = false;
		AZ::Data::Asset<PopcornFXAsset>	m_Asset;
		bool							m_Visible = true;
		float							m_TimeScale = 1.0f;
		bool							m_PrewarmEnable = false;
		float							m_PrewarmTime = -1.0f;
		bool							m_ResetPrewarmButton = false;
		PopcornFXAttributeList			m_AttributeList;
		PopcornFXEditorAttributeList	m_EditorAttributeList;
		PopcornFXEmitter				m_Emitter;

		//! The full file path of the emitter to be loaded when set a emitter programmatically.
		AZStd::string					m_EmitterToLoad;

		bool							m_EditorVisibility = true;

		void	_SetupAsset(const AZ::Data::AssetId &assetId);
	};

}
