//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXEmitterGameComponent.h"

#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include "Integration/PopcornFXIntegrationBus.h"

namespace PopcornFX {

	namespace Internal
	{
		struct PopcornFXEmitterComponentEventsBusHandler final
			: public PopcornFXEmitterComponentEventsBus::Handler
			, public AZ::BehaviorEBusHandler
		{
			AZ_EBUS_BEHAVIOR_BINDER(
				PopcornFXEmitterComponentEventsBusHandler, "{CAECE674-6197-4437-AEAA-8FE8563F089F}", AZ::SystemAllocator, OnEmitterReady);

			void OnEmitterReady() override
			{
				Call(FN_OnEmitterReady);
			}
		};
	} // namespace Internal

	PopcornFXEmitterGameComponent::PopcornFXEmitterGameComponent()
	{
	}

	void	PopcornFXEmitterGameComponent::Reflect(AZ::ReflectContext *reflection)
	{
		PopcornFXEmitterRuntime::Reflect(reflection);

		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXEmitterGameComponent, AZ::Component>()
				->Version(2)
				->Field("Emitter", &PopcornFXEmitterGameComponent::m_Emitter)
				;
		}

		AZ::BehaviorContext	*behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection);
		if (behaviorContext)
		{
			behaviorContext->Class<PopcornFXEmitterGameComponent>()
				->RequestBus("PopcornFXEmitterComponentRequestBus");

			behaviorContext->EBus<PopcornFXEmitterComponentRequestBus>("PopcornFXEmitterComponentRequestBus")
				->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
				->Event("Enable", &PopcornFXEmitterComponentRequestBus::Events::Enable)
				->Event("Start", &PopcornFXEmitterComponentRequestBus::Events::Start)
				->Event("Stop", &PopcornFXEmitterComponentRequestBus::Events::Stop)
				->Event("Terminate", &PopcornFXEmitterComponentRequestBus::Events::Terminate)
				->Event("Kill", &PopcornFXEmitterComponentRequestBus::Events::Kill)
				->Event("Restart", &PopcornFXEmitterComponentRequestBus::Events::Restart)
				->Event("IsEnabled", &PopcornFXEmitterComponentRequestBus::Events::IsEnabled)
				->Event("IsPlaying", &PopcornFXEmitterComponentRequestBus::Events::IsPlaying)
				//->Event("SetupEmitterById", &PopcornFXEmitterComponentRequestBus::Events::SetupEmitterById)
				->Event("SetupEmitterByName", &PopcornFXEmitterComponentRequestBus::Events::SetupEmitterByName)
				->Event("SetVisible", &PopcornFXEmitterComponentRequestBus::Events::SetVisible)
				->Event("GetVisible", &PopcornFXEmitterComponentRequestBus::Events::GetVisible)
				->Event("SetTimeScale", &PopcornFXEmitterComponentRequestBus::Events::SetTimeScale)
				->Event("GetTimeScale", &PopcornFXEmitterComponentRequestBus::Events::GetTimeScale)
				->Event("SetPrewarmEnable", &PopcornFXEmitterComponentRequestBus::Events::SetPrewarmEnable)
				->Event("GetPrewarmEnable", &PopcornFXEmitterComponentRequestBus::Events::GetPrewarmEnable)
				->Event("SetPrewarmTime", &PopcornFXEmitterComponentRequestBus::Events::SetPrewarmTime)
				->Event("GetPrewarmTime", &PopcornFXEmitterComponentRequestBus::Events::GetPrewarmTime)
				->Event("GetAttributesCount", &PopcornFXEmitterComponentRequestBus::Events::GetAttributesCount)
				->Event("GetAttributeType", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeType)
				->Event("GetAttributeId", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeId)
				->Event("ResetAttributeToDefault", &PopcornFXEmitterComponentRequestBus::Events::ResetAttributeToDefault)
				->Event("SetAttributeAsFloat", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsFloat)
				->Event("SetAttributeAsFloat2", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsFloat2)
				->Event("SetAttributeAsFloat3", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsFloat3)
				->Event("SetAttributeAsFloat4", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsFloat4)
				->Event("SetAttributeAsInt", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsInt)
				->Event("SetAttributeAsInt2", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsInt2)
				->Event("SetAttributeAsInt3", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsInt3)
				->Event("SetAttributeAsInt4", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsInt4)
				->Event("SetAttributeAsBool", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsBool)
				->Event("SetAttributeAsBool2", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsBool2)
				->Event("SetAttributeAsBool3", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsBool3)
				->Event("SetAttributeAsBool4", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsBool4)
				->Event("SetAttributeAsQuaternion", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsQuaternion)
				->Event("SetAttributeAsColor", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeAsColor)
				->Event("GetAttributeAsFloat", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeAsFloat)
				->Event("GetAttributeAsFloat2", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeAsFloat2)
				->Event("GetAttributeAsFloat3", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeAsFloat3)
				->Event("GetAttributeAsFloat4", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeAsFloat4)
				->Event("GetAttributeXAsInt", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeXAsInt)
				->Event("GetAttributeYAsInt", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeYAsInt)
				->Event("GetAttributeZAsInt", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeZAsInt)
				->Event("GetAttributeWAsInt", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeWAsInt)
				->Event("GetAttributeXAsBool", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeXAsBool)
				->Event("GetAttributeYAsBool", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeYAsBool)
				->Event("GetAttributeZAsBool", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeZAsBool)
				->Event("GetAttributeWAsBool", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeWAsBool)
				->Event("GetAttributeAsQuaternion", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeAsQuaternion)
				->Event("GetAttributeAsColor", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeAsColor)
				->Event("GetAttributeSamplersCount", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeSamplersCount)
				->Event("GetAttributeSamplerId", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeSamplerId)
				->Event("SetAttributeSampler", &PopcornFXEmitterComponentRequestBus::Events::SetAttributeSampler)
				->Event("ResetSamplerToDefault", &PopcornFXEmitterComponentRequestBus::Events::ResetSamplerToDefault)
				->Event("GetAttributeSampler", &PopcornFXEmitterComponentRequestBus::Events::GetAttributeSampler)
				->Event("SetTeleportThisFrame", &PopcornFXEmitterComponentRequestBus::Events::SetTeleportThisFrame)
				->VirtualProperty("Enable", "IsEnabled", "Enable")
				;

			behaviorContext->EBus<PopcornFXEmitterComponentEventsBus>("PopcornFXEmitterComponentEventsBus")
				->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
				->Attribute(AZ::Edit::Attributes::Category, "PopcornFX")
				->Handler<Internal::PopcornFXEmitterComponentEventsBusHandler>();
		}
	}

#if !defined(O3DE_USE_PK)

	void	PopcornFXEmitterGameComponent::Activate()
	{
	}

	void	PopcornFXEmitterGameComponent::Deactivate()
	{
	}

#else

	void	PopcornFXEmitterGameComponent::Activate()
	{
		m_Emitter.AttachToEntity(GetEntityId());
		m_Emitter.Activate();

		PopcornFXEmitterComponentRequestBus::Handler::BusConnect(m_entity->GetId());
	}

	void	PopcornFXEmitterGameComponent::Deactivate()
	{
		m_Emitter.Deactivate();
		PopcornFXEmitterComponentRequestBus::Handler::BusDisconnect();
	}

	// PopcornFXEmitterComponentRequestBus handlers
	void	PopcornFXEmitterGameComponent::Enable(bool enable)
	{
		m_Emitter.Enable(enable);
	}

	bool	PopcornFXEmitterGameComponent::Start()
	{
		if (!IsEnabled())
			return false;
		if (IsPlaying())
			return false;
		return m_Emitter.Emitter().Start();
	}

	bool	PopcornFXEmitterGameComponent::Stop()
	{
		if (!IsEnabled())
			return false;
		return m_Emitter.Emitter().Stop();
	}

	bool	PopcornFXEmitterGameComponent::Terminate()
	{
		if (!IsEnabled())
			return false;
		return m_Emitter.Emitter().Terminate();
	}

	bool	PopcornFXEmitterGameComponent::Kill()
	{
		if (!IsEnabled())
			return false;
		return m_Emitter.Emitter().Kill();
	}

	bool	PopcornFXEmitterGameComponent::Restart(bool killOnRestart)
	{
		if (!IsEnabled())
			return false;
		if (!IsPlaying())
			return m_Emitter.Emitter().Start();
		else
		{
			m_Emitter.Emitter().SetRestartOnDeath(true);
			if (killOnRestart)
				return m_Emitter.Emitter().Kill();
			else
				return m_Emitter.Emitter().Terminate();
		}
	}

	bool	PopcornFXEmitterGameComponent::IsLoaded()
	{
		return m_Emitter.Emitter().GetEffect() != null;
	}

	bool	PopcornFXEmitterGameComponent::IsEnabled()
	{
		return m_Emitter.IsEnabled();
	}

	bool	PopcornFXEmitterGameComponent::IsPlaying()
	{
		return m_Emitter.Emitter().IsPlaying();
	}

	bool	PopcornFXEmitterGameComponent::SetupEmitterById(const AZ::Data::AssetId &assetId, bool enable)
	{
		if (assetId.IsValid())
		{
			m_Emitter.Deactivate();
			m_Emitter.SetAssetId(assetId);
			m_Emitter.AttachToEntity(GetEntityId());
			m_Emitter.Activate();
			m_Emitter.Enable(enable);
			return true;
		}
		return false;
	}

	bool	PopcornFXEmitterGameComponent::SetupEmitterByName(const AZStd::string &path, bool enable)
	{
		AZ::Data::AssetId	assetId;
		EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, path.c_str(), azrtti_typeid<PopcornFXAsset>(), false);

		return SetupEmitterById(assetId, enable);
	}

	void	PopcornFXEmitterGameComponent::SetVisible(bool visible)
	{
		m_Emitter.SetVisible(visible);
	}

	bool	PopcornFXEmitterGameComponent::GetVisible()
	{
		return m_Emitter.GetVisible();
	}

	void	PopcornFXEmitterGameComponent::SetTimeScale(float timeScale)
	{
		if (timeScale >= 0.0f)
		{
			m_Emitter.SetTimeScale(timeScale);
		}
		else
		{
			AZ_Error("PopcornFX", false, "Error while setting time scale: can't be lower than 0");
		}
	}

	float	PopcornFXEmitterGameComponent::GetTimeScale()
	{
		return m_Emitter.GetTimeScale();
	}

	void	PopcornFXEmitterGameComponent::SetPrewarmEnable(bool enable)
	{
		m_Emitter.SetPrewarmEnable(enable);
	}

	bool	PopcornFXEmitterGameComponent::GetPrewarmEnable()
	{
		return m_Emitter.GetPrewarmEnable();
	}

	void	PopcornFXEmitterGameComponent::SetPrewarmTime(float time)
	{
		if (time >= 0.0f)
		{
			m_Emitter.SetPrewarmTime(time);
		}
		else
		{
			AZ_Error("PopcornFX", false, "Error while setting prewarm time: can't be lower than 0");
		}
	}

	float	PopcornFXEmitterGameComponent::GetPrewarmTime()
	{
		return m_Emitter.GetPrewarmTime();
	}

	AZ::u32	PopcornFXEmitterGameComponent::GetAttributesCount()
	{
		return m_Emitter.Emitter().GetAttributesCount();
	}

	AZStd::string	PopcornFXEmitterGameComponent::GetAttributeName(AZ::u32 attribId)
	{
		return m_Emitter.Emitter().GetAttributeName(attribId);
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeType(AZ::u32 attribId)
	{
		return m_Emitter.Emitter().GetAttributeType(attribId);
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeId(const AZStd::string &name)
	{
		return m_Emitter.Emitter().GetAttributeId(name);
	}

	bool	PopcornFXEmitterGameComponent::ResetAttributeToDefault(AZ::u32 attribId)
	{
		return m_Emitter.Emitter().ResetAttribute(attribId);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsFloat(AZ::u32 attribId, float value)
	{
		return m_Emitter.SetAttributeAsFloat(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsFloat2(AZ::u32 attribId, const AZ::Vector2 &value)
	{
		return m_Emitter.SetAttributeAsFloat2(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsFloat3(AZ::u32 attribId, const AZ::Vector3 &value)
	{
		return m_Emitter.SetAttributeAsFloat3(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsFloat4(AZ::u32 attribId, const AZ::Vector4 &value)
	{
		return m_Emitter.SetAttributeAsFloat4(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsInt(AZ::u32 attribId, AZ::s32 value)
	{
		return m_Emitter.SetAttributeAsInt(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsInt2(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY)
	{
		return m_Emitter.SetAttributeAsInt2(attribId, valueX, valueY);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsInt3(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ)
	{
		return m_Emitter.SetAttributeAsInt3(attribId, valueX, valueY, valueZ);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsInt4(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW)
	{
		return m_Emitter.SetAttributeAsInt4(attribId, valueX, valueY, valueZ, valueW);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsBool(AZ::u32 attribId, bool value)
	{
		return m_Emitter.SetAttributeAsBool(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsBool2(AZ::u32 attribId, bool valueX, bool valueY)
	{
		return m_Emitter.SetAttributeAsBool2(attribId, valueX, valueY);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsBool3(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ)
	{
		return m_Emitter.SetAttributeAsBool3(attribId, valueX, valueY, valueZ);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsBool4(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW)
	{
		return m_Emitter.SetAttributeAsBool4(attribId, valueX, valueY, valueZ, valueW);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsQuaternion(AZ::u32 attribId, const AZ::Quaternion &value)
	{
		return m_Emitter.SetAttributeAsQuaternion(attribId, value);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeAsColor(AZ::u32 attribId, const AZ::Color &value)
	{
		return m_Emitter.SetAttributeAsColor(attribId, value);
	}

	float	PopcornFXEmitterGameComponent::GetAttributeAsFloat(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeAsFloat(attribId);
	}

	AZ::Vector2	PopcornFXEmitterGameComponent::GetAttributeAsFloat2(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeAsFloat2(attribId);
	}

	AZ::Vector3	PopcornFXEmitterGameComponent::GetAttributeAsFloat3(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeAsFloat3(attribId);
	}

	AZ::Vector4	PopcornFXEmitterGameComponent::GetAttributeAsFloat4(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeAsFloat4(attribId);
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeXAsInt(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeXAsInt(attribId);
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeYAsInt(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeYAsInt(attribId);
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeZAsInt(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeZAsInt(attribId);
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeWAsInt(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeWAsInt(attribId);
	}

	bool	PopcornFXEmitterGameComponent::GetAttributeXAsBool(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeXAsBool(attribId);
	}

	bool	PopcornFXEmitterGameComponent::GetAttributeYAsBool(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeYAsBool(attribId);
	}

	bool	PopcornFXEmitterGameComponent::GetAttributeZAsBool(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeZAsBool(attribId);
	}

	bool	PopcornFXEmitterGameComponent::GetAttributeWAsBool(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeWAsBool(attribId);
	}

	AZ::Quaternion	PopcornFXEmitterGameComponent::GetAttributeAsQuaternion(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeAsQuaternion(attribId);
	}

	AZ::Color	PopcornFXEmitterGameComponent::GetAttributeAsColor(AZ::u32 attribId)
	{
		return m_Emitter.GetAttributeAsColor(attribId);
	}

	AZ::u32	PopcornFXEmitterGameComponent::GetAttributeSamplersCount()
	{
		return m_Emitter.Emitter().GetAttributeSamplersCount();
	}

	AZ::s32	PopcornFXEmitterGameComponent::GetAttributeSamplerId(const AZStd::string &name)
	{
		return m_Emitter.Emitter().GetAttributeSamplerId(name);
	}

	bool	PopcornFXEmitterGameComponent::SetAttributeSampler(AZ::u32 attribSamplerId, AZ::EntityId entityId)
	{
		return m_Emitter.Emitter().SetAttributeSampler(attribSamplerId, entityId);
	}

	bool	PopcornFXEmitterGameComponent::ResetSamplerToDefault(AZ::u32 attribSamplerId)
	{
		return SetAttributeSampler(attribSamplerId, (AZ::EntityId)AZ::EntityId::InvalidEntityId);
	}

	AZ::EntityId	PopcornFXEmitterGameComponent::GetAttributeSampler(AZ::u32 attribSamplerId)
	{
		return m_Emitter.Emitter().GetAttributeSampler(attribSamplerId);
	}

	void	PopcornFXEmitterGameComponent::SetTeleportThisFrame()
	{
		m_Emitter.Emitter().SetTeleportThisFrame();
	}

#endif //O3DE_USE_PK

}
