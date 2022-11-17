//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "EmittersManager.h"

#if defined(O3DE_USE_PK)

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzCore/Component/TransformBus.h>

#include "Components/Emitter/PopcornFXEmitterRuntime.h"

namespace PopcornFX {
//----------------------------------------------------------------------------

void	CEmittersManager::Reset()
{
	for (auto it = m_Emitters.begin(); it != m_Emitters.end(); ++it)
	{
		PopcornFXEmitterRuntime	*emitterInstance = it->m_EmitterInstance;
		if (emitterInstance)
		{
			emitterInstance->Deactivate();
			delete emitterInstance;
		}
	}

	m_Emitters.clear();
}

AZ::EntityId	CEmittersManager::SpawnEmitterEntityAtLocation(const AZStd::string &path, const AZ::Transform &transform, bool start)
{
	AZ::Entity	*newEntity;
	EBUS_EVENT_RESULT(newEntity, AzFramework::GameEntityContextRequestBus, CreateGameEntity, "FX");
	newEntity->CreateComponent(AZ::TransformComponentTypeId); // TransformComponent
	newEntity->CreateComponent(PopcornFX::EmitterComponentTypeId);
	newEntity->Activate();

	AZ::TransformBus::Event(newEntity->GetId(), &AZ::TransformInterface::SetWorldTM, transform);

	EBUS_EVENT_ID(newEntity->GetId(), PopcornFX::PopcornFXEmitterComponentRequestBus, SetupEmitterByName, path, start);

	return newEntity->GetId();
}

StandaloneEmitter	*CEmittersManager::SpawnEffectById(const AZ::Data::AssetId &assetId, const PopcornFX::SpawnParams &spawnParams)
{
	m_Emitters.push_back({});
	AZStd::list<StandaloneEmitter>::iterator it = --m_Emitters.end(); //Get last elem
	StandaloneEmitter	*standaloneEmitter = &(*it);

	PopcornFXEmitterRuntime	*emitter = aznew PopcornFXEmitterRuntime;
	if (emitter == null)
		return null;
	standaloneEmitter->m_EmitterInstance = emitter;

	emitter->Init(spawnParams.m_Enable, standaloneEmitter);
	if (spawnParams.m_ParentEntityId.IsValid())
		emitter->AttachToEntity(spawnParams.m_ParentEntityId);
	if (spawnParams.m_PrewarmTime > 0.0f)
		emitter->SetPrewarmEnable(true);
	emitter->SetPrewarmTime(spawnParams.m_PrewarmTime);
	emitter->SetTransform(spawnParams.m_Transform);
	emitter->EnableAutoRemove(spawnParams.m_AutoRemove);
	emitter->SetAssetId(assetId);
	emitter->SetSpawnAttributes(spawnParams.m_Attributes, spawnParams.m_AttributeSamplers);
	emitter->Activate();

	return standaloneEmitter;
}

bool	CEmittersManager::IsEffectAlive(StandaloneEmitter *emitter)
{
	AZStd::list<StandaloneEmitter>::iterator it;
	for (it = m_Emitters.begin(); it != m_Emitters.end(); ++it)
	{
		if (&(*it) == emitter)
			break;
	}
	return it != m_Emitters.end();
}

void	CEmittersManager::DestroyEffect(StandaloneEmitter *emitter)
{
	AZStd::list<StandaloneEmitter>::iterator it;
	for (it = m_Emitters.begin(); it != m_Emitters.end(); ++it)
	{
		if (&(*it) == emitter)
			break;
	}
	if (it != m_Emitters.end())
	{
		PopcornFXEmitterRuntime	*emitterInstance = emitter->m_EmitterInstance;
		emitterInstance->Deactivate();
		delete emitterInstance;
		m_Emitters.erase(it);
	}
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
