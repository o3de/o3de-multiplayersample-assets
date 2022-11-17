//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <PopcornFX/PopcornFXBus.h>

#include <CryCommon/ISystem.h>
#include <CrySystemBus.h>

#include "PopcornFXIntegrationBus.h"
#include "PopcornFXStartUpdate.h"
#include "PopcornFXStopUpdate.h"

#include "SceneInterface/SceneInterface.h"
#include "Preloader/PopcornFXPreloader.h"
#include "Managers/MediumCollectionManager.h"
#include "Managers/EmittersManager.h"
#include "Managers/SceneViewsManager.h"
#include "Managers/RenderManager.h"
#include "Managers/BroadcastManager.h"
#include "Managers/StatsManager.h"
#include "Managers/WindManager.h"

#include <Atom/RPI.Public/Pass/PassSystemInterface.h>

#if defined(POPCORNFX_EDITOR)
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include "Editor/BakerManager.h"
#endif //POPCORNFX_EDITOR

namespace PopcornFX {

	class PopcornFXIntegration
		: protected ISystemEventListener
		, protected CrySystemEventBus::Handler
		, protected PopcornFXDestructionRequestBus::Handler
		, protected PopcornFXLoadBus::Handler
		, protected PopcornFXIntegrationBus::Handler
		, protected PopcornFXRequestBus::Handler
#if defined(POPCORNFX_EDITOR)
		, private AzToolsFramework::EditorLegacyGameModeNotificationBus::Handler
#endif
{
	public:
		void	StartUpdate(float deltaTime);
		void	StopUpdate();

		void	Activate();
		void	Deactivate();

	private:
#if defined(POPCORNFX_EDITOR)
		// AzToolsFramework::EditorLegacyGameModeNotificationBus
		void OnStartGameModeRequest() override;
		void OnStopGameModeRequest() override;
#endif //POPCORNFX_EDITOR

	protected:
		//////////////////////////////////////////////////////////////////////////
		// ISystemEventListener
		virtual void						OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
		// CrySystemEventsBus::Handler interface implementation
		virtual void						OnCrySystemInitialized(ISystem &system, const SSystemInitParams&) override;
		virtual void						OnCrySystemShutdown(ISystem &system) override;
		////////////////////////////////////////////////////////////////////////

		// PopcornFXDestructionRequestBus implementation
		virtual void						DestroyEffect(StandaloneEmitter *emitter) override;
		////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// AZ::Data::PopcornFXLoadBus::Handler
		virtual bool						LoadEffect(PopcornFXAsset *asset, const char *assetPath, const AZ::u8 *assetData, const AZ::IO::SizeType assetDataSize) override;
		virtual void						UnloadEffect(PopcornFXAsset *asset) override;
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// AZ::Data::PopcornFXIntegrationBus::Handler
		virtual void						ReloadAssets() override;
		virtual CParticleMediumCollection	*GetMediumCollection() override;
		virtual bool						RegisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName) override;
		virtual bool						UnregisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName) override;
		virtual const SPayloadValue			*GetCurrentPayloadValue(const AZStd::string &payloadName) const override;
#if defined(POPCORNFX_EDITOR)
		virtual AZStd::string				BakeSingleAsset(const AZStd::string &assetPath, const AZStd::string &outDir, const AZStd::string &platform) override;
		virtual bool						GatherDependencies(const AZStd::string &assetPath, AZStd::vector<AZStd::string> &dependencies) override;
		virtual void						PackChanged(const AZStd::string &packPath, const AZStd::string &libraryPath) override;
		virtual void						SetPkProjPathCache(const AZStd::string &pkProjPath) override { m_PkProjPathCache = pkProjPath; }
		virtual AZStd::string				GetPkProjPathCache() override { return m_PkProjPathCache; }
#endif
		//////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// PopcornFXRequestBus interface implementation
		virtual bool						PreloadEffectById(const AZ::Data::AssetId &assetId) override;
		virtual bool						PreloadEffectByName(const AZStd::string &path) override;
		virtual bool						IsEffectsPreloadingDone() override;
		virtual void						SpawnEmitterAtLocation(const AZStd::string &path, const AZ::Transform &transform) override;
		virtual AZ::EntityId				SpawnEmitterEntityAtLocation(const AZStd::string &path, const AZ::Transform &transform, bool start) override;
		virtual StandaloneEmitter			*SpawnEffectById(const AZ::Data::AssetId &assetId, const PopcornFX::SpawnParams &spawnParams) override;
		virtual StandaloneEmitter			*SpawnEffectByName(const AZStd::string &path, const PopcornFX::SpawnParams &spawnParams) override;
		virtual bool						IsEffectAlive(StandaloneEmitter *emitter) override;
		virtual void						EffectAttachToEntity(StandaloneEmitter *emitter, AZ::EntityId entityId) override;
		virtual void						EffectDetachFromEntity(StandaloneEmitter *emitter) override;
		virtual AZ::EntityId				EffectGetAttachedEntity(StandaloneEmitter *emitter) override;
		virtual void						EffectSetTransform(StandaloneEmitter *emitter, const AZ::Transform &transform) override;
		virtual void						EffectEnable(StandaloneEmitter *emitter, bool enable) override;
		virtual bool						EffectStart(StandaloneEmitter *emitter) override;
		virtual bool						EffectStop(StandaloneEmitter *emitter) override;
		virtual bool						EffectTerminate(StandaloneEmitter *emitter) override;
		virtual bool						EffectKill(StandaloneEmitter *emitter) override;
		virtual bool						EffectRestart(StandaloneEmitter *emitter, bool killOnRestart) override;
		virtual bool						EffectIsEnabled(StandaloneEmitter *emitter) override;
		virtual bool						EffectIsPlaying(StandaloneEmitter *emitter) override;
		virtual void						EffectSetVisible(StandaloneEmitter *emitter, bool visible) override;
		virtual bool						EffectGetVisible(StandaloneEmitter *emitter) override;
		virtual void						EffectSetTimeScale(StandaloneEmitter *emitter, float timeScale) override;
		virtual float						EffectGetTimeScale(StandaloneEmitter *emitter) override;
		virtual void						EffectSetPrewarmEnable(StandaloneEmitter *emitter, bool enable) override;
		virtual bool						EffectGetPrewarmEnable(StandaloneEmitter *emitter) override;
		virtual void						EffecSetPrewarmTime(StandaloneEmitter *emitter, float time) override;
		virtual float						EffecGetPrewarmTime(StandaloneEmitter *emitter) override;
		virtual AZ::u32						GetAttributesCount(StandaloneEmitter *emitter) override;
		virtual AZ::s32						GetAttributeType(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::s32						EffectGetAttributeId(StandaloneEmitter *emitter, const AZStd::string &name) override;
		virtual bool						EffectResetAttributeToDefault(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual bool						EffectSetAttributeAsFloat(StandaloneEmitter *emitter, AZ::u32 attribId, float value) override;
		virtual bool						EffectSetAttributeAsFloat2(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector2 &value) override;
		virtual bool						EffectSetAttributeAsFloat3(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector3 &value) override;
		virtual bool						EffectSetAttributeAsFloat4(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector4 &value) override;
		virtual bool						EffectSetAttributeAsInt(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 value) override;
		virtual bool						EffectSetAttributeAsInt2(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY) override;
		virtual bool						EffectSetAttributeAsInt3(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ) override;
		virtual bool						EffectSetAttributeAsInt4(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW) override;
		virtual bool						EffectSetAttributeAsBool(StandaloneEmitter *emitter, AZ::u32 attribId, bool value) override;
		virtual bool						EffectSetAttributeAsBool2(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY) override;
		virtual bool						EffectSetAttributeAsBool3(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY, bool valueZ) override;
		virtual bool						EffectSetAttributeAsBool4(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW) override;
		virtual bool						EffectSetAttributeAsQuaternion(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Quaternion &value) override;
		virtual bool						EffectSetAttributeAsColor(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Color &value) override;
		virtual float						EffectGetAttributeAsFloat(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::Vector2					EffectGetAttributeAsFloat2(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::Vector3					EffectGetAttributeAsFloat3(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::Vector4					EffectGetAttributeAsFloat4(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::s32						EffectGetAttributeXAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::s32						EffectGetAttributeYAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::s32						EffectGetAttributeZAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::s32						EffectGetAttributeWAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual bool						EffectGetAttributeXAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual bool						EffectGetAttributeYAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual bool						EffectGetAttributeZAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual bool						EffectGetAttributeWAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::Quaternion				EffectGetAttributeAsQuaternion(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::Color					EffectGetAttributeAsColor(StandaloneEmitter *emitter, AZ::u32 attribId) override;
		virtual AZ::s32						EffectGetAttributeSamplersCount(StandaloneEmitter *emitter) override;
		virtual AZ::s32						EffectGetAttributeSamplerId(StandaloneEmitter *emitter, const AZStd::string &samplerName) override;
		virtual bool						EffectSetAttributeSampler(StandaloneEmitter *emitter, AZ::u32 attribSamplerId, AZ::EntityId entityId) override;
		virtual bool						EffectResetSamplerToDefault(StandaloneEmitter *emitter, AZ::u32 attribSamplerId) override;
		virtual AZ::EntityId				EffectGetAttributeSampler(StandaloneEmitter *emitter, AZ::u32 attribSamplerId) override;
		virtual void						EffectSetTeleportThisFrame(StandaloneEmitter *emitter) override;
		virtual float						GetLODBias() override;
		virtual void						SetLODBias(float bias) override;
		virtual void						SetBakingThreadpool() override;
		////////////////////////////////////////////////////////////////////////
		void								_LoadPassTemplateMappings();
		AZ::RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler m_LoadTemplatesHandler;

	private:
		bool			_ActivateManagers();
		void			_DeactivateManagers();
		void			_SetEnabled(bool enable);
		void			_Clean(bool unloadPreloadedEffects);

		bool						m_Enabled = false;
		bool						m_FeatureProcessorEnabled = false;
		AZStd::string				m_PackPath;
		AZStd::string				m_LibraryPath;
		CSceneInterface				m_SceneInterface;
		PopcornFXPreloader			m_Preloader;

		CMediumCollectionManager	m_MediumCollectionManager;
		CEmittersManager			m_EmittersManager;
		CSceneViewsManager			m_SceneViewsManager;
		CBroadcastManager			m_BroadcastManager;
		CStatsManager				m_StatsManager;
		CWindManager				m_WindManager;
#if defined(POPCORNFX_EDITOR)
		CBakerManager				m_BakerManager;
		AZStd::string				m_PkProjPathCache;
#endif
		AZStd::mutex				m_ParticleQualityLock;

		PopcornFXStartUpdate		m_StartUpdate;
		PopcornFXStopUpdate			m_StopUpdate;
	};

}

#endif //O3DE_USE_PK

