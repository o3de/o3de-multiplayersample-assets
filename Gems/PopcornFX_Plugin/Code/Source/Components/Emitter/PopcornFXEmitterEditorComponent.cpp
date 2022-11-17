//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXEmitterEditorComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/Entity/EditorEntityContextComponent.h>

#include "Integration/PopcornFXIntegrationBus.h"
#include "Integration/PopcornFXUtils.h"

namespace PopcornFX {

	PopcornFXEmitterEditorComponent::PopcornFXEmitterEditorComponent()
		: m_Enable(true)
		, m_Asset(AZ::Data::AssetLoadBehavior::QueueLoad)
	{
	}

	void	PopcornFXEmitterEditorComponent::Init()
	{
		EditorComponentBase::Init();
	}

	void	PopcornFXEmitterEditorComponent::Activate()
	{
		EditorComponentBase::Activate();

		AZ::TickBus::Handler::BusConnect();
		PopcornFXEmitterComponentRequestBus::Handler::BusConnect(m_entity->GetId());
		AzToolsFramework::EditorVisibilityNotificationBus::Handler::BusConnect(GetEntityId());

		m_IsPlaying = false;
		EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);

		m_Emitter.Activate();
		m_Emitter.AttachToEntity(m_entity->GetId(), false);
		OnAssetChanged();
	}

	void	PopcornFXEmitterEditorComponent::Deactivate()
	{
		m_Emitter.Deactivate();
		m_Emitter.Clear();
		m_Emitter.AttachToEntity(AZ::EntityId(), false);

		if (!m_EmitterToLoad.empty())
		{
			m_EmitterToLoad.clear();
			AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
		}

		if (m_Asset.GetId().IsValid() && AZ::Data::AssetBus::Handler::BusIsConnectedId(m_Asset.GetId()))
		{
			AZ::Data::AssetBus::Handler::BusDisconnect(m_Asset.GetId());
		}
		m_Asset.Release();

		m_IsPlaying = false;
		EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);

		AzToolsFramework::EditorVisibilityNotificationBus::Handler::BusConnect(GetEntityId());
		PopcornFXEmitterComponentRequestBus::Handler::BusDisconnect();
		AZ::TickBus::Handler::BusDisconnect();

		EditorComponentBase::Deactivate();
	}

	void	PopcornFXEmitterEditorComponent::Reflect(AZ::ReflectContext *context)
	{
		PopcornFXEditorAttributeList::Reflect(context);

		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
		{
			// Serializer:
			serializeContext->Class<PopcornFXEmitterEditorComponent, AzToolsFramework::Components::EditorComponentBase>()
				->Version(1)
				->Field("Enable", &PopcornFXEmitterEditorComponent::m_Enable)
				->Field("StartStopButton", &PopcornFXEmitterEditorComponent::m_StartStopButton)
				->Field("RestartButton", &PopcornFXEmitterEditorComponent::m_RestartButton)
				->Field("KillButton", &PopcornFXEmitterEditorComponent::m_KillButton)
				->Field("ParticleSystem", &PopcornFXEmitterEditorComponent::m_Asset)
				->Field("Visible", &PopcornFXEmitterEditorComponent::m_Visible)
				->Field("TimeScale", &PopcornFXEmitterEditorComponent::m_TimeScale)
				->Field("PrewarmEnable", &PopcornFXEmitterEditorComponent::m_PrewarmEnable)
				->Field("PrewarmTime", &PopcornFXEmitterEditorComponent::m_PrewarmTime)
				->Field("ResetPrewanButton", &PopcornFXEmitterEditorComponent::m_ResetPrewarmButton)
				->Field("AttributeList", &PopcornFXEmitterEditorComponent::m_AttributeList)
				->Field("EditorAttributeList", &PopcornFXEmitterEditorComponent::m_EditorAttributeList)
				;

			// Edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXEmitterEditorComponent>("PopcornFX Emitter", "Particle Emitter")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
						->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
						->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
						->Attribute(AZ::Edit::Attributes::PrimaryAssetType, AZ::AzTypeInfo<PopcornFXAsset>::Uuid())
						->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/PopcornFX_Icon.svg")
						->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Viewport_PKFX.png")
						->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(0, &PopcornFXEmitterEditorComponent::m_Enable, "Enable", "Whether or not the particle component is active.")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEmitterEditorComponent::OnEnableChanged)

					->DataElement(AZ::Edit::UIHandlers::Button, &PopcornFXEmitterEditorComponent::m_StartStopButton, "", "Start/Stop the particles emission.")
						->Attribute(AZ::Edit::Attributes::ButtonText, &PopcornFXEmitterEditorComponent::StartStopButtonText)
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEmitterEditorComponent::OnStartStopButtonPressed)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEmitterEditorComponent::GetButtonsVisibility)
					->DataElement(AZ::Edit::UIHandlers::Button, &PopcornFXEmitterEditorComponent::m_RestartButton, "", "Restart the particles emission.")
						->Attribute(AZ::Edit::Attributes::ButtonText, "Restart")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEmitterEditorComponent::OnRestartButtonPressed)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEmitterEditorComponent::GetButtonsVisibility)
					->DataElement(AZ::Edit::UIHandlers::Button, &PopcornFXEmitterEditorComponent::m_KillButton, "", "Kill all the particles and stop the emission.")
						->Attribute(AZ::Edit::Attributes::ButtonText, "Kill")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEmitterEditorComponent::OnKillButtonPressed)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXEmitterEditorComponent::GetButtonsVisibility)

					->DataElement(0, &PopcornFXEmitterEditorComponent::m_Asset, "Particle System", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEmitterEditorComponent::OnAssetChanged)

					->DataElement(0, &PopcornFXEmitterEditorComponent::m_EditorAttributeList, "Attributes", "List of effect's attributes.")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
						->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)

					->ClassElement(AZ::Edit::ClassElements::Group, "Options")
						->DataElement(0, &PopcornFXEmitterEditorComponent::m_Visible, "Visible", "Visibility of the emitter instance.")
						->DataElement(0, &PopcornFXEmitterEditorComponent::m_TimeScale, "Time Scale", "Time Scale of the emitter instance.")
							->Attribute(AZ::Edit::Attributes::Min, 0.0f)
						->DataElement(0, &PopcornFXEmitterEditorComponent::m_PrewarmEnable, "Prewarm Enable", "Enable the prewarm effect of the emitter instance.")
							->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
						->DataElement(0, &PopcornFXEmitterEditorComponent::m_PrewarmTime, "Prewarm Time", "Prewarm time of the emitter instance.")
							->Attribute(AZ::Edit::Attributes::Min, 0.0f)
							->Attribute(AZ::Edit::Attributes::ReadOnly, &PopcornFXEmitterEditorComponent::IsPrewarmTimeReadOnly)
						->DataElement(AZ::Edit::UIHandlers::Button, &PopcornFXEmitterEditorComponent::m_ResetPrewarmButton, "", "Reset Prewarm values to default defined in the effect.")
							->Attribute(AZ::Edit::Attributes::ButtonText, "Reset Prewarm")
							->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXEmitterEditorComponent::OnResetPrewarmButtonPressed)
					;
			}
		}

		AZ::BehaviorContext	*behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
		if (behaviorContext)
		{
			behaviorContext->Class<PopcornFXEmitterEditorComponent>()
				->RequestBus("PopcornFXEmitterComponentRequestBus");
		}
	}

	void	PopcornFXEmitterEditorComponent::BuildGameEntity(AZ::Entity *gameEntity)
	{
		PopcornFXEmitterGameComponent	*component = gameEntity->CreateComponent<PopcornFXEmitterGameComponent>();

		if (component)
		{
			// Copy the editor-side settings to the game entity to be exported.
			component->m_Emitter.Init(m_Enable, m_AttributeList);
			component->m_Emitter.SetAsset(m_Asset);
			component->m_Emitter.SetVisible(m_Visible);
			component->m_Emitter.SetTimeScale(m_TimeScale);
			component->m_Emitter.SetPrewarmEnable(m_PrewarmEnable);
			component->m_Emitter.SetPrewarmTime(m_PrewarmTime);
		}
	}

	void	PopcornFXEmitterEditorComponent::SetPrimaryAsset(const AZ::Data::AssetId &assetId)
	{
		AZ::Data::Asset<PopcornFXAsset> asset = AZ::Data::AssetManager::Instance().GetAsset<PopcornFXAsset>(assetId, AZ::Data::AssetLoadBehavior::QueueLoad);
		if (asset)
		{
			m_Asset = asset;
			OnAssetChanged();
		}
	}

	void	PopcornFXEmitterEditorComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		(void)deltaTime; (void)time;

		m_Emitter.SetVisible(m_EditorVisibility && m_Visible);
		m_Emitter.SetTimeScale(m_TimeScale);
		m_Emitter.SetPrewarmEnable(m_PrewarmEnable);
		m_Emitter.SetPrewarmTime(m_PrewarmTime);

		if (m_UpdateUI)
		{
			bool	wasPlaying = m_IsPlaying;
			m_IsPlaying = m_Emitter.IsPlaying();

			if (wasPlaying != m_IsPlaying && IsSelected())
			{
				EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);
			}
		}
	}

	void	PopcornFXEmitterEditorComponent::OnEntityVisibilityChanged(bool visibility)
	{
		m_EditorVisibility = visibility;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::OnEnableChanged()
	{
		if (m_Enable)
			m_Emitter.Start();
		else
			m_Emitter.Kill();
		return AZ::Edit::PropertyRefreshLevels::EntireTree;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::OnStartStopButtonPressed()
	{
		if (m_IsPlaying)
		{
			m_Emitter.Stop();
		}
		else
		{
			m_Emitter.Start();
		}
		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::OnRestartButtonPressed()
	{
		Restart(true);
		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::OnKillButtonPressed()
	{
		m_Emitter.Kill();
		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::OnResetPrewarmButtonPressed()
	{
		if (m_Asset.GetId().IsValid())
		{
			PopcornFXAsset	*pkAsset = m_Asset.GetAs<PopcornFXAsset>();
			if (pkAsset != null && pkAsset->m_Effect != null)
			{
				m_PrewarmEnable = pkAsset->m_Effect->EnablePrewarm();
				m_PrewarmTime = pkAsset->m_Effect->PrewarmTime();
				return AZ::Edit::PropertyRefreshLevels::EntireTree;
			}
		}
		return AZ::Edit::PropertyRefreshLevels::None;
	}

	AZStd::string	PopcornFXEmitterEditorComponent::StartStopButtonText() const
	{
		if (m_IsPlaying)
			return "Stop";
		return "Start";
	}

	AZ::Crc32	PopcornFXEmitterEditorComponent::GetButtonsVisibility() const
	{
		if (m_Enable)
			return AZ::Edit::PropertyVisibility::Show;
		return AZ::Edit::PropertyVisibility::Hide;
	}

	bool	PopcornFXEmitterEditorComponent::IsPrewarmTimeReadOnly() const
	{
		return !m_PrewarmEnable;
	}

	void PopcornFXEmitterEditorComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
	{
		bool isInGameMode = true;
		AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(isInGameMode, &AzToolsFramework::EditorEntityContextRequestBus::Events::IsEditorRunningGame);

		if (!isInGameMode && asset.GetId() == m_Asset.GetId())
		{
			m_UpdateUI = true;
			m_Emitter.Clear();

			PopcornFXAsset	*pkAsset = asset.GetAs<PopcornFXAsset>();
			if (pkAsset != null)
			{
				if (m_PrewarmTime < 0.0f)
				{
					m_PrewarmEnable = pkAsset->m_Effect->EnablePrewarm();
					m_PrewarmTime = pkAsset->m_Effect->PrewarmTime();
				}

				const AZ::EntityId				entityId = GetEntityId();
				const CParticleAttributeList	*defaultList = pkAsset->m_Effect->AttributeFlatList().Get();

				m_AttributeList.Prepare(defaultList);
				m_EditorAttributeList.Prepare(	defaultList, m_AttributeList.AttributeRawDataAttributes(),
												m_AttributeList.Samplers(), entityId);
				m_Emitter.Set(pkAsset, m_AttributeList);
				if (m_Enable)
					m_Emitter.Start();

				PopcornFXEmitterComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterComponentEventsBus::Events::OnEmitterReady);
			}

			SetDirty();
			//refresh the selected emitter UI
			EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// AssetEventHandler handlers

	void PopcornFXEmitterEditorComponent::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
	{
		m_Asset = asset;
		OnAssetReady(asset);
	}

	void PopcornFXEmitterEditorComponent::OnAssetChanged()
	{
		m_UpdateUI = false;
		m_Emitter.Clear();
		m_EditorAttributeList.Clear();

		if (AZ::Data::AssetBus::Handler::BusIsConnected())
		{
			AZ::Data::AssetBus::Handler::BusDisconnect();
		}

		if (m_Asset.GetId().IsValid())
		{
			AZ::Data::AssetBus::Handler::BusConnect(m_Asset.GetId());
			m_Asset.QueueLoad();
		}
		else
		{
			m_IsPlaying = false;
			m_AttributeList.Clear();
		}

		EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
	}

	//////////////////////////////////////////////////////////////////////////
	// AssetCatalogEventBus
	void	PopcornFXEmitterEditorComponent::OnCatalogAssetAdded(const AZ::Data::AssetId &assetId)
	{
		AZ::Data::AssetId cmAssetId;
		EBUS_EVENT_RESULT(cmAssetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, m_EmitterToLoad.c_str(), azrtti_typeid<PopcornFXAsset>(), false);

		if (cmAssetId == assetId)
		{
			m_EmitterToLoad.clear();
			AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
			m_Asset.Create(assetId, true);
			OnAssetChanged();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// PopcornFXEmitterComponentRequestBus handlers

	void	PopcornFXEmitterEditorComponent::Enable(bool enable)
	{
		if (enable == m_Enable)
			return;

		if (enable)
		{
			if (IsPlaying()) //Can happen if the KillDeferred is not ended
				m_Emitter.SetRestartOnDeath(true);
			else
				m_Emitter.Start();
		}
		else
		{
			m_Emitter.SetRestartOnDeath(false);
			m_Emitter.Kill();
		}
		m_Enable = enable;
	}

	bool	PopcornFXEmitterEditorComponent::Start()
	{
		if (!IsEnabled())
			return false;
		if (IsPlaying())
			return false;
		return m_Emitter.Start();
	}

	bool	PopcornFXEmitterEditorComponent::Stop()
	{
		if (!IsEnabled())
			return false;
		return m_Emitter.Stop();
	}

	bool	PopcornFXEmitterEditorComponent::Terminate()
	{
		if (!IsEnabled())
			return false;
		return m_Emitter.Terminate();
	}

	bool	PopcornFXEmitterEditorComponent::Kill()
	{
		if (!IsEnabled())
			return false;
		return m_Emitter.Kill();
	}

	bool	PopcornFXEmitterEditorComponent::Restart(bool killOnRestart)
	{
		if (!IsEnabled())
			return false;

		if (!IsPlaying())
			return m_Emitter.Start();
		else
		{
			m_Emitter.SetRestartOnDeath(true);
			if (killOnRestart)
				return m_Emitter.Kill();
			else
				return m_Emitter.Terminate();
		}
	}

	bool	PopcornFXEmitterEditorComponent::IsLoaded()
	{
		return m_Emitter.GetEffect() != null;
	}

	bool	PopcornFXEmitterEditorComponent::IsEnabled()
	{
		return m_Enable;
	}

	bool	PopcornFXEmitterEditorComponent::IsPlaying()
	{
		return m_Emitter.IsPlaying();
	}

	bool	PopcornFXEmitterEditorComponent::SetupEmitterById(const AZ::Data::AssetId &assetId, bool enable)
	{
		m_Enable = enable;

		_SetupAsset(assetId);
		return true;
	}

	bool	PopcornFXEmitterEditorComponent::SetupEmitterByName(const AZStd::string &path, bool enable)
	{
		m_Enable = enable;

		AZ::Data::AssetId	assetId;
		EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, path.c_str(), azrtti_typeid<PopcornFXAsset>(), false);

		_SetupAsset(assetId);
		return assetId.IsValid();
	}

	void	PopcornFXEmitterEditorComponent::_SetupAsset(const AZ::Data::AssetId &assetId)
	{
		if (!assetId.IsValid())
		{
			//asset does not exist, listen to the asset add event from AssetCatalogEventBus
			AZStd::string	emitterPath;
			EBUS_EVENT_RESULT(emitterPath, AZ::Data::AssetCatalogRequestBus, GetAssetPathById, m_Asset.GetId());
			m_EmitterToLoad = emitterPath;
			AzFramework::AssetCatalogEventBus::Handler::BusConnect();
		}
		else
		{
			//assign to the asset
			m_Asset.Create(assetId, true);
			//Populates the emitter list and create emitter from selected effect.
			OnAssetChanged();
		}
	}

	void	PopcornFXEmitterEditorComponent::SetVisible(bool visible)
	{
		m_Visible = visible;
	}

	bool	PopcornFXEmitterEditorComponent::GetVisible()
	{
		return m_Visible;
	}

	void	PopcornFXEmitterEditorComponent::SetTimeScale(float timeScale)
	{
		if (timeScale >= 0.0f)
		{
			m_TimeScale = timeScale;
		}
		else
		{
			AZ_Error("PopcornFX", false, "Error while setting time scale: can't be lower than 0");
		}
	}

	float	PopcornFXEmitterEditorComponent::GetTimeScale()
	{
		return m_TimeScale;
	}

	void	PopcornFXEmitterEditorComponent::SetPrewarmEnable(bool enable)
	{
		m_PrewarmEnable = enable;
	}

	bool	PopcornFXEmitterEditorComponent::GetPrewarmEnable()
	{
		return m_PrewarmEnable;
	}

	void	PopcornFXEmitterEditorComponent::SetPrewarmTime(float time)
	{
		if (time >= 0.0f)
		{
			m_PrewarmTime = time;
		}
		else
		{
			AZ_Error("PopcornFX", false, "Error while setting prewarm time: can't be lower than 0");
		}
	}

	float	PopcornFXEmitterEditorComponent::GetPrewarmTime()
	{
		return m_PrewarmTime;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::GetAttributesCount()
	{
		return m_Emitter.GetAttributesCount();
	}

	AZStd::string	PopcornFXEmitterEditorComponent::GetAttributeName(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeName(attribId);
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeType(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeType(attribId);
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeId(const AZStd::string &name)
	{
		return m_Emitter.GetAttributeId(name);
	}

	bool	PopcornFXEmitterEditorComponent::ResetAttributeToDefault(AZ::u32 attribId)
	{
		if (m_Emitter.ResetAttribute(attribId))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsFloat(AZ::u32 attribId, float value)
	{
		if (m_Emitter.SetAttribute(attribId, CFloat1(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsFloat2(AZ::u32 attribId, const AZ::Vector2 &value)
	{
		if (m_Emitter.SetAttribute(attribId, ToPk(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsFloat3(AZ::u32 attribId, const AZ::Vector3 &value)
	{
		if (m_Emitter.SetAttribute(attribId, ToPk(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsFloat4(AZ::u32 attribId, const AZ::Vector4 &value)
	{
		if (m_Emitter.SetAttribute(attribId, ToPk(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsInt(AZ::u32 attribId, AZ::s32 value)
	{
		if (m_Emitter.SetAttribute(attribId, CInt1(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsInt2(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY)
	{
		if (m_Emitter.SetAttribute(attribId, CInt2(valueX, valueY)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsInt3(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ)
	{
		if (m_Emitter.SetAttribute(attribId, CInt3(valueX, valueY, valueZ)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsInt4(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW)
	{
		if (m_Emitter.SetAttribute(attribId, CInt4(valueX, valueY, valueZ, valueW)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsBool(AZ::u32 attribId, bool value)
	{
		if (m_Emitter.SetAttribute(attribId, CBool1(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsBool2(AZ::u32 attribId, bool valueX, bool valueY)
	{
		if (m_Emitter.SetAttribute(attribId, CBool2(valueX, valueY)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsBool3(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ)
	{
		if (m_Emitter.SetAttribute(attribId, CBool3(valueX, valueY, valueZ)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsBool4(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW)
	{
		if (m_Emitter.SetAttribute(attribId, CBool4(valueX, valueY, valueZ, valueW)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsQuaternion(AZ::u32 attribId, const AZ::Quaternion &value)
	{
		if (m_Emitter.SetAttribute(attribId, ToPk(value)))
		{
			m_EditorAttributeList.RefreshAttribute(attribId);
			PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
			SetDirty();
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeAsColor(AZ::u32 attribId, const AZ::Color &value)
	{
		const EPopcornFXType	type = static_cast<EPopcornFXType>(m_Emitter.GetAttributeType(attribId));

		if (type == EPopcornFXType::Type_Float3)
		{
			if (m_Emitter.SetAttribute(attribId, ToPk(value.GetAsVector3())))
			{
				m_EditorAttributeList.RefreshAttribute(attribId);
				PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
				SetDirty();
				return true;
			}
		}
		else
		{
#if PK_ATTRIB_ENABLE_CHECKS
			if (type != EPopcornFXType::Type_Float4)
			{
				AZ_Warning(	"PopcornFX", false, "SetAttribute: the Attribute [%d] \"%s\" cannot be set as Color: the attribute is %s",
							attribId, m_Emitter.GetAttributeName(attribId).c_str(), O3DEPopcornFXTypeToString(type).c_str());
				return false;
			}
#endif
			if (m_Emitter.SetAttribute(attribId, ToPk(value.GetAsVector4())))
			{
				m_EditorAttributeList.RefreshAttribute(attribId);
				PopcornFXEmitterEditorComponentEventsBus::Event(GetEntityId(), &PopcornFXEmitterEditorComponentEventsBus::Events::OnAttributeChanged);
				SetDirty();
				return true;
			}
		}
		return false;
	}

	float	PopcornFXEmitterEditorComponent::GetAttributeAsFloat(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32f[0];
		}
		return 0.0f;
	}

	AZ::Vector2	PopcornFXEmitterEditorComponent::GetAttributeAsFloat2(AZ::u32 attribId)
	{
		AZ::Vector2						value(0.0f);
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
		}
		return value;
	}

	AZ::Vector3	PopcornFXEmitterEditorComponent::GetAttributeAsFloat3(AZ::u32 attribId)
	{
		AZ::Vector3						value(0.0f);
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
			value.SetZ(attrib.m_Data32f[2]);
		}
		return value;
	}

	AZ::Vector4	PopcornFXEmitterEditorComponent::GetAttributeAsFloat4(AZ::u32 attribId)
	{
		AZ::Vector4						value(0.0f);
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
			value.SetZ(attrib.m_Data32f[2]);
			value.SetW(attrib.m_Data32f[3]);
		}
		return value;
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeXAsInt(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[0];
		}
		return 0;
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeYAsInt(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[1];
		}
		return 0;
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeZAsInt(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[2];
		}
		return 0;
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeWAsInt(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[3];
		}
		return 0;
	}

	bool	PopcornFXEmitterEditorComponent::GetAttributeXAsBool(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[0];
		}
		return 0;
	}

	bool	PopcornFXEmitterEditorComponent::GetAttributeYAsBool(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[1];
		}
		return 0;
	}

	bool	PopcornFXEmitterEditorComponent::GetAttributeZAsBool(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[2];
		}
		return 0;
	}

	bool	PopcornFXEmitterEditorComponent::GetAttributeWAsBool(AZ::u32 attribId)
	{
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[3];
		}
		return 0;
	}

	AZ::Quaternion	PopcornFXEmitterEditorComponent::GetAttributeAsQuaternion(AZ::u32 attribId)
	{
		AZ::Quaternion value = AZ::Quaternion::CreateIdentity();

		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
			value.SetZ(attrib.m_Data32f[2]);
			value.SetW(attrib.m_Data32f[3]);
		}
		return value;
	}

	AZ::Color	PopcornFXEmitterEditorComponent::GetAttributeAsColor(AZ::u32 attribId)
	{
		AZ::Color	value;

		EPopcornFXType	type = static_cast<EPopcornFXType>(m_Emitter.GetAttributeType(attribId));
		if (type == EPopcornFXType::Type_Float3)
		{
			value = AZ::Color::CreateFromVector3(GetAttributeAsFloat3(attribId));
		}
		else
		{
			AZ::Vector4	vec4Value = GetAttributeAsFloat4(attribId);
			value = AZ::Color::CreateFromVector3AndFloat(vec4Value.GetAsVector3(), vec4Value.GetW());
		}
		return value;
	}

	AZ::u32	PopcornFXEmitterEditorComponent::GetAttributeSamplersCount()
	{
		return m_Emitter.GetAttributeSamplersCount();
	}

	AZ::s32	PopcornFXEmitterEditorComponent::GetAttributeSamplerId(const AZStd::string &name)
	{
		return m_Emitter.GetAttributeSamplerId(name);
	}

	bool	PopcornFXEmitterEditorComponent::SetAttributeSampler(AZ::u32 attribSamplerId, AZ::EntityId entityId)
	{
		AZ::EntityId	newEntityId = entityId;
		const bool		res = m_Emitter.SetAttributeSampler(attribSamplerId, newEntityId);

		if (!res)
			newEntityId = m_Emitter.GetAttributeSampler(attribSamplerId);

		m_EditorAttributeList.RefreshSampler(attribSamplerId);
		SetDirty();
		return res;
	}

	bool	PopcornFXEmitterEditorComponent::ResetSamplerToDefault(AZ::u32 attribSamplerId)
	{
		return SetAttributeSampler(attribSamplerId, (AZ::EntityId)AZ::EntityId::InvalidEntityId);
	}

	AZ::EntityId	PopcornFXEmitterEditorComponent::GetAttributeSampler(AZ::u32 attribSamplerId)
	{
		return m_Emitter.GetAttributeSampler(attribSamplerId);
	}

	void	PopcornFXEmitterEditorComponent::SetTeleportThisFrame()
	{
		return m_Emitter.SetTeleportThisFrame();
	}


}
