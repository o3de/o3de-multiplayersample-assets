//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "PopcornFXIntegration.h"

#if defined(O3DE_USE_PK)

//Startup
#include "Startup/PopcornFxStartup.h"
#include "Startup/PopcornFxPlugins.h"
#include "File/FileSystemController_O3DE.h"
#include "File/JsonPackPath.h"

#if defined(POPCORNFX_EDITOR)
#include "Editor/PackLoader.h"
#endif //POPCORNFX_EDITOR

#include <AzCore/Console/IConsole.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/Utils/Utils.h>

#include "Components/Emitter/PopcornFXEmitterRuntime.h"

#include <pk_kernel/include/kr_thread_pool_default.h>
#include <pk_kernel/include/kr_caps_cpu.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>
#include <Atom/RPI.Public/RPISystemInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Integration/Render/AtomIntegration/PopcornFXFeatureProcessor.h>

namespace PopcornFX {

	// CVar: p_PopcornFXParticles
	AZ_CVAR(bool, p_PopcornFXParticles, true,
			null,
			AZ::ConsoleFunctorFlags::DontReplicate,
			"Set to true/false to globally enable/disable PopcornFX particles.");

	// CVar: p_PopcornFXBuildVersion
	AZ_CVAR(AZ::CVarFixedString, p_PopcornFXBuildVersion, "VeryHigh",
			[]([[maybe_unused]] const AZ::CVarFixedString &buildVersion) -> void
			{
				PopcornFXIntegrationBus::Broadcast(&PopcornFXIntegrationBus::Events::ReloadAssets);
			},
			AZ::ConsoleFunctorFlags::DontReplicate,
			"Set PopcornFX particles Build Version.");

void	PopcornFXIntegration::StartUpdate(float deltaTime)
{
	if (!m_Enabled)
		return;

	if (!m_FeatureProcessorEnabled)
	{
		const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
		if (scene)
		{
			CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
			if (pkfxFeatureProc)
			{
				pkfxFeatureProc->Init(m_MediumCollectionManager.MediumCollection(), m_SceneViewsManager.SceneViews());
				pkfxFeatureProc->GetRenderManager().Activate(m_MediumCollectionManager.MediumCollection(), m_PackPath);
				m_FeatureProcessorEnabled = true;
			}
		}
	}

	// First, we reset the stats and start the main thread overhead timer:
	CParticleMediumCollection	*mediumCollection = m_MediumCollectionManager.MediumCollection();
	m_StatsManager.Reset(mediumCollection);

	STATS_START_MAIN_THREAD_TIMER_SCOPED(m_StatsManager);

	if (!p_PopcornFXParticles)
		return;

	PopcornFXDestructionRequestBus::ExecuteQueuedEvents();

	m_WindManager.Update();
	m_SceneViewsManager.Update(mediumCollection);

	// This binds the CollectFrame function to the m_OnUpdateComplete:
	//m_RenderManager.StartUpdate(mediumCollection, m_SceneViewsManager.SceneViews());
	const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
	if (scene)
	{
		CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
		if (pkfxFeatureProc)
			pkfxFeatureProc->GetRenderManager().StartUpdate(m_MediumCollectionManager.MediumCollection(), m_SceneViewsManager.SceneViews());
	}

	// Start CPU timer (stopped by PopcornFXIntegration::_OnUpdateComplete):
	m_StatsManager.StartUpdateSpanTimer();

	// Start the medium collection update:
	m_MediumCollectionManager.StartUpdate(deltaTime);
}

void	PopcornFXIntegration::StopUpdate()
{
	if (!m_Enabled)
		return;

	STATS_START_MAIN_THREAD_TIMER_SCOPED(m_StatsManager);

	if (!p_PopcornFXParticles)
		return;

	m_StatsManager.StopMainThreadUpdateSpanTimer();

	CParticleMediumCollection	*mediumCollection = m_MediumCollectionManager.MediumCollection();
	m_MediumCollectionManager.StopUpdate();

	m_BroadcastManager.Update();

	m_StatsManager.StartBillboardingSpanTimer();

	//m_RenderManager.StopUpdate(mediumCollection);
	const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
	if (scene)
	{
		CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
		if (pkfxFeatureProc)
		{
			pkfxFeatureProc->GetRenderManager().StopUpdate(m_MediumCollectionManager.MediumCollection());
		}
	}

	m_StatsManager.StopBillboardingSpanTimer();

	// Update scene timings
	m_StatsManager.Update(mediumCollection);
}

void	PopcornFXIntegration::Activate()
{
	CrySystemEventBus::Handler::BusConnect();
	PopcornFXDestructionRequestBus::Handler::BusConnect();
	PopcornFXLoadBus::Handler::BusConnect();
	PopcornFXIntegrationBus::Handler::BusConnect();
	PopcornFXRequestBus::Handler::BusConnect();
#if defined(POPCORNFX_EDITOR)
	AzToolsFramework::EditorLegacyGameModeNotificationBus::Handler::BusConnect();
#endif
	_SetEnabled(true);
	AZ::RPI::FeatureProcessorFactory	*factory = AZ::RPI::FeatureProcessorFactory::Get();
	if (factory != null)
		factory->RegisterFeatureProcessorWithInterface<CPopcornFXFeatureProcessor, CPopcornFXFeatureProcessorInterface>();
}

void	PopcornFXIntegration::Deactivate()
{
	const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
	if (scene)
	{
		CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
		if (pkfxFeatureProc)
		{
			pkfxFeatureProc->Init(null, null);
			pkfxFeatureProc->GetRenderManager().Deactivate();
		}
	}
	m_FeatureProcessorEnabled = false;

	AZ::RPI::FeatureProcessorFactory	*factory = AZ::RPI::FeatureProcessorFactory::Get();
	if (factory != null)
		factory->UnregisterFeatureProcessor<CPopcornFXFeatureProcessor>();
	_SetEnabled(false);

#if defined(POPCORNFX_EDITOR)
	AzToolsFramework::EditorLegacyGameModeNotificationBus::Handler::BusDisconnect();
#endif
	CrySystemEventBus::Handler::BusDisconnect();
	PopcornFXDestructionRequestBus::Handler::BusDisconnect();
	PopcornFXIntegrationBus::Handler::BusDisconnect();
	PopcornFXLoadBus::Handler::BusDisconnect();
	PopcornFXRequestBus::Handler::BusDisconnect();
}

bool	PopcornFXIntegration::_ActivateManagers()
{
	m_WindManager.Activate();
	m_WindManager.Reset(m_LibraryPath);
	m_StatsManager.Activate();
	if (!PK_VERIFY(m_MediumCollectionManager.Activate(&m_SceneInterface)))
		return false;
	m_SceneViewsManager.Activate();

	m_StartUpdate.Activate(this);
	m_StopUpdate.Activate(this);

	return true;
}

void	PopcornFXIntegration::_DeactivateManagers()
{
	m_StopUpdate.Deactivate();
	m_StartUpdate.Deactivate();

	m_SceneViewsManager.Deactivate();
	m_MediumCollectionManager.Deactivate();
	m_StatsManager.Deactivate();
	m_WindManager.Deactivate();
}

void	PopcornFXIntegration::_SetEnabled(bool enable)
{
	if (enable == m_Enabled)
	{
		return;
	}

	if (enable)
	{
		if (!PopcornStartup())
			return;
		if (!PopcornRegisterPlugins(EPlugin_CompilerBackendVM))
			return;
#if defined(POPCORNFX_EDITOR)
		if (!PK_VERIFY(m_BakerManager.Activate()))
			return;
#endif
	}
	else
	{
#if defined(POPCORNFX_EDITOR)
		m_BakerManager.Deactivate();
#endif
		File::DefaultFileSystem()->UnmountAllPacks();
		PopcornUnregisterPlugins();
		PopcornShutdown();
	}
	m_Enabled = enable;
}

void	PopcornFXIntegration::_Clean(bool unloadPreloadedEffects)
{
	m_MediumCollectionManager.Reset();
	const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
	if (scene)
	{
		CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
		if (pkfxFeatureProc)
			pkfxFeatureProc->GetRenderManager().Reset();
	}
	m_EmittersManager.Reset();
	if (unloadPreloadedEffects)
		m_Preloader.Clear();
}

#if defined(POPCORNFX_EDITOR)
// AzToolsFramework::EditorLegacyGameModeNotificationBus
void PopcornFXIntegration::OnStartGameModeRequest()
{
	_Clean(false);
	m_SceneInterface.SetInGameMode(true);
}

void PopcornFXIntegration::OnStopGameModeRequest()
{
	_Clean(false);
	m_SceneInterface.SetInGameMode(false);
}
#endif //POPCORNFX_EDITOR

//////////////////////////////////////////////////////////////////////////
// ISystemEventListener
// CrySystemEventsBus::Handler interface implementation
////////////////////////////////////////////////////////////////////////

void	PopcornFXIntegration::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	(void)wparam; (void)lparam;

	if (!m_Enabled)
		return;

	switch (event)
	{
	// This event happens unload, which still triggers on map transition in the standalone client
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
		_Clean(true);
		break;
	//case ESYSTEM_EVENT_GAME_MODE_SWITCH_START:
	//	_Clean(false);
	//	break;
	}
}

void	PopcornFXIntegration::OnCrySystemInitialized(ISystem &system, const SSystemInitParams&)
{
#if !defined(AZ_MONOLITHIC_BUILD)
	// When module is linked dynamically, we must set our gEnv pointer.
	// When module is linked statically, we'll share the application's gEnv pointer.
	gEnv = system.GetGlobalEnvironment();
#endif

	system.GetISystemEventDispatcher()->RegisterListener(this);

	if (m_Enabled)
	{
#if !defined(POPCORNFX_EDITOR)
		if (!LoadPackPathFromJson(m_PackPath, m_LibraryPath))
		{
			AZ_Error("PopcornFX", false, "Unable to load PopcornFX paths from json. Open a scene with PopcornFX effects in the Editor");
		}
		if (!m_PackPath.empty())
		{
			PFilePack	pack = File::DefaultFileSystem()->MountPack(m_PackPath.c_str());
			AZ_Error("PopcornFX", pack != null, "Unable to mount PopcornFX pack '%s'", m_PackPath.c_str());
		}
		else
		{
			AZ_Error("PopcornFX", false, "Empty PopcornFX pack path in json.");
		}
#else
		if (LoadPackPathFromJson(m_PackPath, m_LibraryPath))
		{
			if (!m_PackPath.empty() && !m_LibraryPath.empty())
			{
				PFilePack	pack = File::DefaultFileSystem()->MountPack(m_PackPath.c_str());
				if (pack != null)
					PackChanged(m_PackPath, m_LibraryPath);
			}
		}
#endif

		PK_VERIFY(_ActivateManagers());
	}

	// setup handler for load pass template mappings
	m_LoadTemplatesHandler = AZ::RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler([this]() { this->_LoadPassTemplateMappings(); });
	AZ::RPI::PassSystemInterface::Get()->ConnectEvent(m_LoadTemplatesHandler);
}

void	PopcornFXIntegration::_LoadPassTemplateMappings()
{
	const char	*passTemplatesFile = "passes/PKPassTemplates.azasset";
	AZ::RPI::PassSystemInterface::Get()->LoadPassTemplateMappings(passTemplatesFile);
}

void	PopcornFXIntegration::OnCrySystemShutdown(ISystem &system)
{
	if (m_Enabled)
	{
		_Clean(true);
		_DeactivateManagers();
	}

	system.GetISystemEventDispatcher()->RemoveListener(this);
}

////////////////////////////////////////////////////////////////////////
// PopcornFXDestructionRequestBus implementation
////////////////////////////////////////////////////////////////////////

void	PopcornFXIntegration::DestroyEffect(StandaloneEmitter *emitter)
{
	m_EmittersManager.DestroyEffect(emitter);
}

////////////////////////////////////////////////////////////////////////
// AZ::Data::PopcornFXLoadBus::Handler
////////////////////////////////////////////////////////////////////////

bool	PopcornFXIntegration::LoadEffect(PopcornFXAsset *asset, const char *assetPath, const AZ::u8 *assetData, const AZ::IO::SizeType assetDataSize)
{
#if defined(POPCORNFX_EDITOR)
	AZStd::string	assetFullPath;
	AZ::IO::FixedMaxPath projectPath = AZ::Utils::GetProjectPath();
	AZ::StringFunc::Path::Join(projectPath.c_str(), assetPath, assetFullPath);

	AZStd::string	rootPath = m_PackPath;
	AZStd::string	libraryPath;

	if (ChangePackIFN(assetFullPath.c_str(), File::DefaultFileSystem(), rootPath, libraryPath, false))
	{
		AZStd::string	oldRootPath;
		AZStd::string	oldLibraryPath;
		if (LoadPackPathFromJson(oldRootPath, oldLibraryPath))
		{
			if (!oldRootPath.empty())
			{
				AZ_Error("PopcornFX", false, "The effect '%s' is not part of the pack '%s'.\n If you want to change the current pack delete the popcornfx_pack.json file at the root of your project and restart the O3DE Editor.", assetPath, oldRootPath.c_str());
				return false;
			}
		}
		PackChanged(rootPath, libraryPath);
	}
#endif //POPCORNFX_EDITOR

	const CString	pkPath = File::DefaultFileSystem()->PhysicalToVirtual(assetPath);
	if (pkPath.Empty())
		return false;

	asset->m_Path = pkPath.Data();

	asset->m_Context = PK_NEW(PopcornFX::HBO::CContext());

	CConstMemoryStream	rawStream = CConstMemoryStream(assetData, static_cast<u32>(assetDataSize));
	asset->m_File = asset->m_Context->LoadFileFromStream(rawStream, pkPath, true);

	const CStringView	buildVersionStringView = CStringView::FromNullTerminatedString(static_cast<AZ::CVarFixedString>(p_PopcornFXBuildVersion).c_str());
	asset->m_Effect = PopcornFX::CParticleEffect::Load(asset->m_File, SEffectLoadCtl::kDefault, buildVersionStringView);

	if (asset->m_Effect == null)
	{
		AZ_Error("PopcornFX", false, "PopcornFXAssetHandler: Unable to load the asset %s", assetPath);
		return false;
	}
	return true;
}

void	PopcornFXIntegration::UnloadEffect(PopcornFXAsset *asset)
{
	AZ::RPI::RPISystemInterface *rpiSystemInterface = AZ::RPI::RPISystemInterface::Get();

	if (rpiSystemInterface != null)
	{
		const auto	scene = rpiSystemInterface->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
		if (asset->m_Effect != null && scene != null)
		{
			CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
			if (pkfxFeatureProc != null)
				pkfxFeatureProc->GetRenderManager().UnregisterEffectMaterials(asset->m_Effect);
		}
	}

	asset->m_Effect = null;
	if (asset->m_File != null)
	{
		if (asset->m_File->RefCount() > 1) // if == 1, the file is already unloaded in the LoadAssetData during the reload
			asset->m_File->Unload();
	}
	asset->m_File = null;
	PK_SAFE_DELETE(asset->m_Context);
}

////////////////////////////////////////////////////////////////////////
// AZ::Data::PopcornFXIntegrationBus::Handler
////////////////////////////////////////////////////////////////////////

void	PopcornFXIntegration::ReloadAssets()
{
	AZStd::lock_guard<AZStd::mutex>	lock(m_ParticleQualityLock);

	//temp: reload all the assets until we can change the build versions on the CParticleEffect
	AZ::Data::AssetCatalogRequests::AssetEnumerationCB	popcornFxAssetReloadCb = [](const AZ::Data::AssetId id, const AZ::Data::AssetInfo &info)
	{
		if (info.m_assetType == azrtti_typeid<PopcornFXAsset>())
		{
			AZ::Data::AssetManager::Instance().ReloadAsset(id, AZ::Data::AssetLoadBehavior::QueueLoad);
		}
	};
	AZ::Data::AssetCatalogRequestBus::Broadcast(&AZ::Data::AssetCatalogRequestBus::Events::EnumerateAssets, null, popcornFxAssetReloadCb, null);
}

CParticleMediumCollection	*PopcornFXIntegration::GetMediumCollection()
{
	return m_MediumCollectionManager.MediumCollection();
}

bool	PopcornFXIntegration::RegisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName)
{
	return m_BroadcastManager.RegisterToBroadcast(entityId, eventName);
}

bool	PopcornFXIntegration::UnregisterToBroadcast(AZ::EntityId entityId, const AZStd::string &eventName)
{
	return m_BroadcastManager.UnregisterToBroadcast(entityId, eventName);
}

const SPayloadValue	*PopcornFXIntegration::GetCurrentPayloadValue(const AZStd::string &payloadName) const
{
	return m_BroadcastManager.GetCurrentPayloadValue(payloadName);
}

#if defined(POPCORNFX_EDITOR)
AZStd::string	PopcornFXIntegration::BakeSingleAsset(const AZStd::string &assetPath, const AZStd::string &outDir, const AZStd::string &platform)
{
	return m_BakerManager.BakeSingleAsset(assetPath, outDir, platform, m_WindManager);
}

bool	PopcornFXIntegration::GatherDependencies(const AZStd::string &assetPath, AZStd::vector<AZStd::string> &dependencies)
{
	return m_BakerManager.GatherDependencies(assetPath, dependencies, m_WindManager);
}

void	PopcornFXIntegration::PackChanged(const AZStd::string &packPath, const AZStd::string &libraryPath)
{
	if (!LoadPackPathFromJson(m_PackPath, m_LibraryPath) ||
		m_PackPath != packPath || m_LibraryPath != libraryPath)
	{
		m_PackPath = packPath;
		m_LibraryPath = libraryPath;
		SavePackPathToJson(m_PackPath, m_LibraryPath);
	}
	const auto	scene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name(AzFramework::Scene::MainSceneName));
	if (scene)
	{
		CPopcornFXFeatureProcessor	*pkfxFeatureProc = static_cast<CPopcornFXFeatureProcessor*>(scene->GetFeatureProcessor<CPopcornFXFeatureProcessor>());
		if (pkfxFeatureProc)
			pkfxFeatureProc->GetRenderManager().SetPackPath(m_PackPath);
	}
	m_WindManager.Reset(m_LibraryPath);
}
#endif

////////////////////////////////////////////////////////////////////////
// PopcornFXRequestBus interface implementation
////////////////////////////////////////////////////////////////////////

bool	PopcornFXIntegration::PreloadEffectById(const AZ::Data::AssetId &assetId)
{
	return m_Preloader.PreloadEffect(assetId);
}

bool	PopcornFXIntegration::PreloadEffectByName(const AZStd::string &path)
{
	AZ::Data::AssetId	assetId;
	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, path.c_str(), azrtti_typeid<PopcornFXAsset>(), false);

	return m_Preloader.PreloadEffect(assetId);
}

bool	PopcornFXIntegration::IsEffectsPreloadingDone()
{
	return m_Preloader.IsPreloadingDone();
}

void	PopcornFXIntegration::SpawnEmitterAtLocation(const AZStd::string &path, const AZ::Transform &transform)
{
	SpawnEffectByName(path, PopcornFX::SpawnParams(true, true, transform));
}

AZ::EntityId	PopcornFXIntegration::SpawnEmitterEntityAtLocation(const AZStd::string &path, const AZ::Transform &transform, bool start)
{
	return m_EmittersManager.SpawnEmitterEntityAtLocation(path, transform, start);
}

StandaloneEmitter	*PopcornFXIntegration::SpawnEffectById(const AZ::Data::AssetId &assetId, const PopcornFX::SpawnParams &spawnParams)
{
	return m_EmittersManager.SpawnEffectById(assetId, spawnParams);
}

StandaloneEmitter	*PopcornFXIntegration::SpawnEffectByName(const AZStd::string &path, const PopcornFX::SpawnParams &spawnParams)
{
	AZ::Data::AssetId	assetId;
	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, path.c_str(), azrtti_typeid<PopcornFXAsset>(), false);

	if (!assetId.IsValid())
		return null;

	return SpawnEffectById(assetId, spawnParams);
}

bool	PopcornFXIntegration::IsEffectAlive(StandaloneEmitter *emitter)
{
	return m_EmittersManager.IsEffectAlive(emitter);
}


void	PopcornFXIntegration::EffectAttachToEntity(StandaloneEmitter *emitter, AZ::EntityId entityId)
{
	emitter->m_EmitterInstance->AttachToEntity(entityId);
}

AZ::EntityId	PopcornFXIntegration::EffectGetAttachedEntity(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->Emitter().GetAttachedEntity();
}

void	PopcornFXIntegration::EffectDetachFromEntity(StandaloneEmitter *emitter)
{
	emitter->m_EmitterInstance->Emitter().DetachFromEntity();
}

void	PopcornFXIntegration::EffectSetTransform(StandaloneEmitter *emitter, const AZ::Transform &transform)
{
	emitter->m_EmitterInstance->SetTransform(transform);
}

void	PopcornFXIntegration::EffectEnable(StandaloneEmitter *emitter, bool enable)
{
	emitter->m_EmitterInstance->Enable(enable);
}

bool	PopcornFXIntegration::EffectStart(StandaloneEmitter *emitter)
{
	if (!EffectIsEnabled(emitter))
		return false;
	if (EffectIsPlaying(emitter))
		return false;
	return emitter->m_EmitterInstance->Emitter().Start();
}

bool	PopcornFXIntegration::EffectStop(StandaloneEmitter *emitter)
{
	if (!EffectIsEnabled(emitter))
		return false;
	return emitter->m_EmitterInstance->Emitter().Stop();
}

bool	PopcornFXIntegration::EffectTerminate(StandaloneEmitter *emitter)
{
	if (!EffectIsEnabled(emitter))
		return false;
	return emitter->m_EmitterInstance->Emitter().Terminate();
}

bool	PopcornFXIntegration::EffectKill(StandaloneEmitter *emitter)
{
	if (!EffectIsEnabled(emitter))
		return false;
	return emitter->m_EmitterInstance->Emitter().Kill();
}

bool	PopcornFXIntegration::EffectRestart(StandaloneEmitter *emitter, bool killOnRestart)
{
	if (!EffectIsEnabled(emitter))
		return false;
	if (killOnRestart)
		emitter->m_EmitterInstance->Emitter().Kill();
	else
		emitter->m_EmitterInstance->Emitter().Terminate();
	return emitter->m_EmitterInstance->Emitter().Start();
}

bool	PopcornFXIntegration::EffectIsEnabled(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->IsEnabled();
}

bool	PopcornFXIntegration::EffectIsPlaying(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->Emitter().IsPlaying();
}

void	PopcornFXIntegration::EffectSetVisible(StandaloneEmitter *emitter, bool visible)
{
	emitter->m_EmitterInstance->SetVisible(visible);
}

bool	PopcornFXIntegration::EffectGetVisible(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->GetVisible();
}

void	PopcornFXIntegration::EffectSetTimeScale(StandaloneEmitter *emitter, float timeScale)
{
	if (timeScale >= 0.0f)
	{
		emitter->m_EmitterInstance->SetTimeScale(timeScale);
	}
	else
	{
		AZ_Error("PopcornFX", false, "Error while setting time scale: can't be lower than 0");
	}
}

float	PopcornFXIntegration::EffectGetTimeScale(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->GetTimeScale();
}

void	PopcornFXIntegration::EffectSetPrewarmEnable(StandaloneEmitter *emitter, bool enable)
{
	emitter->m_EmitterInstance->SetPrewarmEnable(enable);
}

bool	PopcornFXIntegration::EffectGetPrewarmEnable(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->GetPrewarmEnable();
}

void	PopcornFXIntegration::EffecSetPrewarmTime(StandaloneEmitter *emitter, float time)
{
	if (time >= 0.0f)
	{
		emitter->m_EmitterInstance->SetPrewarmTime(time);
	}
	else
	{
		AZ_Error("PopcornFX", false, "Error while setting prewarm time: can't be lower than 0");
	}
}

float	PopcornFXIntegration::EffecGetPrewarmTime(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->GetPrewarmTime();
}

AZ::u32	PopcornFXIntegration::GetAttributesCount(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->Emitter().GetAttributesCount();
}

AZ::s32	PopcornFXIntegration::GetAttributeType(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->Emitter().GetAttributeType(attribId);
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeId(StandaloneEmitter *emitter, const AZStd::string &name)
{
	return emitter->m_EmitterInstance->Emitter().GetAttributeId(name);
}

bool	PopcornFXIntegration::EffectResetAttributeToDefault(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->Emitter().ResetAttribute(attribId);
}

bool	PopcornFXIntegration::EffectSetAttributeAsFloat(StandaloneEmitter *emitter, AZ::u32 attribId, float value)
{
	return emitter->m_EmitterInstance->SetAttributeAsFloat(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsFloat2(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector2 &value)
{
	return emitter->m_EmitterInstance->SetAttributeAsFloat2(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsFloat3(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector3 &value)
{
	return emitter->m_EmitterInstance->SetAttributeAsFloat3(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsFloat4(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector4 &value)
{
	return emitter->m_EmitterInstance->SetAttributeAsFloat4(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsInt(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 value)
{
	return emitter->m_EmitterInstance->SetAttributeAsInt(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsInt2(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY)
{
	return emitter->m_EmitterInstance->SetAttributeAsInt2(attribId, valueX, valueY);
}

bool	PopcornFXIntegration::EffectSetAttributeAsInt3(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ)
{
	return emitter->m_EmitterInstance->SetAttributeAsInt3(attribId, valueX, valueY, valueZ);
}

bool	PopcornFXIntegration::EffectSetAttributeAsInt4(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW)
{
	return emitter->m_EmitterInstance->SetAttributeAsInt4(attribId, valueX, valueY, valueZ, valueW);
}

bool	PopcornFXIntegration::EffectSetAttributeAsBool(StandaloneEmitter *emitter, AZ::u32 attribId, bool value)
{
	return emitter->m_EmitterInstance->SetAttributeAsBool(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsBool2(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY)
{
	return emitter->m_EmitterInstance->SetAttributeAsBool2(attribId, valueX, valueY);
}

bool	PopcornFXIntegration::EffectSetAttributeAsBool3(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY, bool valueZ)
{
	return emitter->m_EmitterInstance->SetAttributeAsBool3(attribId, valueX, valueY, valueZ);
}

bool	PopcornFXIntegration::EffectSetAttributeAsBool4(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW)
{
	return emitter->m_EmitterInstance->SetAttributeAsBool4(attribId, valueX, valueY, valueZ, valueW);
}

bool	PopcornFXIntegration::EffectSetAttributeAsQuaternion(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Quaternion &value)
{
	return emitter->m_EmitterInstance->SetAttributeAsQuaternion(attribId, value);
}

bool	PopcornFXIntegration::EffectSetAttributeAsColor(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Color &value)
{
	return emitter->m_EmitterInstance->SetAttributeAsColor(attribId, value);
}

float	PopcornFXIntegration::EffectGetAttributeAsFloat(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeAsFloat(attribId);
}

AZ::Vector2	PopcornFXIntegration::EffectGetAttributeAsFloat2(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeAsFloat2(attribId);
}

AZ::Vector3	PopcornFXIntegration::EffectGetAttributeAsFloat3(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeAsFloat3(attribId);
}

AZ::Vector4	PopcornFXIntegration::EffectGetAttributeAsFloat4(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeAsFloat4(attribId);
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeXAsInt(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeXAsInt(attribId);
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeYAsInt(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeYAsInt(attribId);
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeZAsInt(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeZAsInt(attribId);
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeWAsInt(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeWAsInt(attribId);
}

bool	PopcornFXIntegration::EffectGetAttributeXAsBool(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeXAsBool(attribId);
}

bool	PopcornFXIntegration::EffectGetAttributeYAsBool(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeYAsBool(attribId);
}

bool	PopcornFXIntegration::EffectGetAttributeZAsBool(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeZAsBool(attribId);
}

bool	PopcornFXIntegration::EffectGetAttributeWAsBool(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeWAsBool(attribId);
}

AZ::Quaternion	PopcornFXIntegration::EffectGetAttributeAsQuaternion(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeAsQuaternion(attribId);
}

AZ::Color	PopcornFXIntegration::EffectGetAttributeAsColor(StandaloneEmitter *emitter, AZ::u32 attribId)
{
	return emitter->m_EmitterInstance->GetAttributeAsColor(attribId);
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeSamplersCount(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->Emitter().GetAttributeSamplersCount();
}

AZ::s32	PopcornFXIntegration::EffectGetAttributeSamplerId(StandaloneEmitter *emitter, const AZStd::string &samplerName)
{
	return emitter->m_EmitterInstance->Emitter().GetAttributeSamplerId(samplerName);
}

bool	PopcornFXIntegration::EffectSetAttributeSampler(StandaloneEmitter *emitter, AZ::u32 attribSamplerId, AZ::EntityId entityId)
{
	return emitter->m_EmitterInstance->Emitter().SetAttributeSampler(attribSamplerId, entityId);
}

bool	PopcornFXIntegration::EffectResetSamplerToDefault(StandaloneEmitter *emitter, AZ::u32 attribSamplerId)
{
	return EffectSetAttributeSampler(emitter, attribSamplerId, (AZ::EntityId)AZ::EntityId::InvalidEntityId);
}

AZ::EntityId	PopcornFXIntegration::EffectGetAttributeSampler(StandaloneEmitter *emitter, AZ::u32 attribSamplerId)
{
	return emitter->m_EmitterInstance->Emitter().GetAttributeSampler(attribSamplerId);
}

void	PopcornFXIntegration::EffectSetTeleportThisFrame(StandaloneEmitter *emitter)
{
	return emitter->m_EmitterInstance->Emitter().SetTeleportThisFrame();
}

float	PopcornFXIntegration::GetLODBias()
{
	CParticleMediumCollection *mediumCollection = m_MediumCollectionManager.MediumCollection();
	if (mediumCollection != null)
	{
		return mediumCollection->LODBias();
	}
	return 0.0f;
}

void	PopcornFXIntegration::SetLODBias(float bias)
{
	CParticleMediumCollection *mediumCollection = m_MediumCollectionManager.MediumCollection();
	if (mediumCollection != null)
	{
		mediumCollection->SetLODBias(bias);
	}
}

void	PopcornFXIntegration::SetBakingThreadpool()
{
	bool								success = true;
	CThreadManager::EPriority			workersPriority = CThreadManager::Priority_BackgroundLow;
	PopcornFX::PWorkerThreadPool		pool = PK_NEW(PopcornFX::CWorkerThreadPool);

	if (PK_VERIFY(pool != null))
	{
		u32		processorCount = 1;
		success = pool->AddFullAffinityWorkers(processorCount, CPU::Caps().ProcessAffinity(), workersPriority);
		if (!success)
			return;
		pool->StartWorkers();
	}
	Scheduler::SetThreadPool(pool);
}

}

#endif //O3DE_USE_PK
