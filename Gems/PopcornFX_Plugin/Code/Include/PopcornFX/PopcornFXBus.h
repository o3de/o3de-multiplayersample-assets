//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/EBus/Policies.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Asset/AssetManager.h>

namespace PopcornFX
{
	class PopcornFXEmitterRuntime;
}

namespace PopcornFX
{
	class CFilePack;

	struct StandaloneEmitter
	{
	public:
		AZ_TYPE_INFO(StandaloneEmitter, "{0C899DAC-6B19-4BDD-AD8C-8A11EF2A6729}");

		PopcornFX::PopcornFXEmitterRuntime	*m_EmitterInstance;

		StandaloneEmitter()
			: m_EmitterInstance(nullptr)
		{
		}
	};

	class SpawnParams
	{
	public:
		AZ_TYPE_INFO(SpawnParams, "{168487C1-3975-4ACF-8A0A-1A44FB1F9C88}");

		struct Attribute
		{
			Attribute(const AZStd::string &name, float value) : m_Name(name), m_FloatX(value) {}
			Attribute(const AZStd::string &name, const AZ::Vector2 &value) : m_Name(name), m_FloatX(value.GetX()), m_FloatY(value.GetY()) {}
			Attribute(const AZStd::string &name, const AZ::Vector3 &value) : m_Name(name), m_FloatX(value.GetX()), m_FloatY(value.GetY()), m_FloatZ(value.GetZ()) {}
			Attribute(const AZStd::string &name, const AZ::Vector4 &value) : m_Name(name), m_FloatX(value.GetX()), m_FloatY(value.GetY()), m_FloatZ(value.GetZ()), m_FloatW(value.GetW()) {}
			Attribute(const AZStd::string &name, const AZ::Quaternion &value) : m_Name(name), m_FloatX(value.GetX()), m_FloatY(value.GetY()), m_FloatZ(value.GetZ()), m_FloatW(value.GetW()) {}
			Attribute(const AZStd::string &name, AZ::s32 value) : m_Name(name), m_IntX(value) {}
			Attribute(const AZStd::string &name, AZ::s32 valueX, AZ::s32 valueY) : m_Name(name), m_IntX(valueX), m_IntY(valueY) {}
			Attribute(const AZStd::string &name, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ) : m_Name(name), m_IntX(valueX), m_IntY(valueY), m_IntZ(valueZ) {}
			Attribute(const AZStd::string &name, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW) : m_Name(name), m_IntX(valueX), m_IntY(valueY), m_IntZ(valueZ), m_IntW(valueW) {}
			Attribute(const AZStd::string &name, bool value) : m_Name(name), m_IntX(value) {}
			Attribute(const AZStd::string &name, bool valueX, bool valueY) : m_Name(name), m_IntX(valueX), m_IntY(valueY) {}
			Attribute(const AZStd::string &name, bool valueX, bool valueY, bool valueZ) : m_Name(name), m_IntX(valueX), m_IntY(valueY), m_IntZ(valueZ) {}
			Attribute(const AZStd::string &name, bool valueX, bool valueY, bool valueZ, bool valueW) : m_Name(name), m_IntX(valueX), m_IntY(valueY), m_IntZ(valueZ), m_IntW(valueW) {}

			AZStd::string	m_Name;
			float			m_FloatX = 0.0f;
			float			m_FloatY = 0.0f;
			float			m_FloatZ = 0.0f;
			float			m_FloatW = 0.0f;
			AZ::s32			m_IntX = 0;
			AZ::s32			m_IntY = 0;
			AZ::s32			m_IntZ = 0;
			AZ::s32			m_IntW = 0;
		};

		struct AttributeSampler
		{
			AttributeSampler(const AZStd::string &name, AZ::EntityId entityId) : m_Name(name), m_SamplerEntity(entityId) {}

			AZStd::string	m_Name;
			AZ::EntityId	m_SamplerEntity;
		};

		//If the prewarmTime is < 0.0f, it will get the default time from the effect asset
		SpawnParams(bool enable, bool autoRemove, const AZ::Transform &transform, float prewarmTime = -1.0f, AZ::EntityId parentEntityId = AZ::EntityId())
			: m_Enable(enable)
			, m_AutoRemove(autoRemove)
			, m_Transform(transform)
			, m_PrewarmTime(prewarmTime)
			, m_ParentEntityId(parentEntityId)
		{
		}

		void	AddAttribute(const Attribute &attribute) { m_Attributes.push_back(attribute); }
		void	AddAttributeSampler(const AttributeSampler &attributeSampler) { m_AttributeSamplers.push_back(attributeSampler); }

		bool							m_Enable;
		bool							m_AutoRemove;
		AZ::Transform					m_Transform;
		float							m_PrewarmTime;
		AZ::EntityId					m_ParentEntityId;
		AZStd::vector<Attribute>		m_Attributes;
		AZStd::vector<AttributeSampler>	m_AttributeSamplers;
	};

	struct SBroadcastParams
	{
		AZStd::string	m_EventName;
	};

	enum	EPopcornFXType
	{
		Type_Unknown = -1,
		Type_Bool,
		Type_Bool2,
		Type_Bool3,
		Type_Bool4,
		Type_Float,
		Type_Float2,
		Type_Float3,
		Type_Float4,
		Type_Int,
		Type_Int2,
		Type_Int3,
		Type_Int4,
		Type_Quaternion
	};

	class PopcornFXEvents
		: public AZ::EBusTraits
	{
	public:
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		// Public functions
		virtual ~PopcornFXEvents() {}

		virtual void	OnEmitterReady(StandaloneEmitter *emitter) { (void)emitter; }
		//virtual void	OnEmitterBroadcastEvent(SBroadcastParams *params) { (void)params; }

	};
	using PopcornFXEventsBus = AZ::EBus <PopcornFXEvents>;

	class PopcornFXRequests
		: public AZ::EBusTraits
	{
	public:
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		// Public functions
		virtual ~PopcornFXRequests() {}

		virtual bool						PreloadEffectById(const AZ::Data::AssetId &assetId) = 0;
		virtual bool						PreloadEffectByName(const AZStd::string &path) = 0;
		virtual bool						IsEffectsPreloadingDone() = 0;

		virtual void						SpawnEmitterAtLocation(const AZStd::string &path, const AZ::Transform &transform) = 0;
		virtual AZ::EntityId				SpawnEmitterEntityAtLocation(const AZStd::string &path, const AZ::Transform &transform, bool start) = 0;

		virtual StandaloneEmitter			*SpawnEffectById(const AZ::Data::AssetId &assetId, const SpawnParams &spawnParams) = 0;
		virtual StandaloneEmitter			*SpawnEffectByName(const AZStd::string &path, const SpawnParams &spawnParams) = 0;
		virtual void						DestroyEffect(StandaloneEmitter *emitter) = 0;

		// Check if the StandaloneEmitter still exists
		virtual bool						IsEffectAlive(StandaloneEmitter *emitter) = 0;

		// Attach the effect on an entity
		virtual void						EffectAttachToEntity(StandaloneEmitter *emitter, AZ::EntityId entityId) = 0;
		// Detach the effect from an entity
		virtual void						EffectDetachFromEntity(StandaloneEmitter *emitter) = 0;
		// Get the attached entity.
		virtual AZ::EntityId				EffectGetAttachedEntity(StandaloneEmitter *emitter) = 0;
		// Set WorldTM if not attached on an entity and LocalTM otherwise
		virtual void						EffectSetTransform(StandaloneEmitter *emitter, const AZ::Transform &transform) = 0;

		// Enable/disable current emitter.
		virtual void						EffectEnable(StandaloneEmitter *emitter, bool enable) = 0;
		// Start the particle emission on the current emitter.
		virtual bool						EffectStart(StandaloneEmitter *emitter) = 0;
		// Stop the particle emission on the current emitter.
		virtual bool						EffectStop(StandaloneEmitter *emitter) = 0;
		// Terminate the current emitter (attributes and transform are no longer updated).
		virtual bool						EffectTerminate(StandaloneEmitter *emitter) = 0;
		// Kill all particles of the current emitter.
		virtual bool						EffectKill(StandaloneEmitter *emitter) = 0;
		// Restart the current emitter.
		virtual bool						EffectRestart(StandaloneEmitter *emitter, bool killOnRestart) = 0;
		// Get whether current emitter is enabled.
		virtual bool						EffectIsEnabled(StandaloneEmitter *emitter) = 0;
		// Get whether current emitter is playing.
		virtual bool						EffectIsPlaying(StandaloneEmitter *emitter) = 0;
		// Set effect visibility.
		virtual void						EffectSetVisible(StandaloneEmitter *emitter, bool visible) = 0;
		// Get effect visibility.
		virtual bool						EffectGetVisible(StandaloneEmitter *emitter) = 0;
		// Set effect time scale.
		virtual void						EffectSetTimeScale(StandaloneEmitter *emitter, float timeScale) = 0;
		// Get effect time scale.
		virtual float						EffectGetTimeScale(StandaloneEmitter *emitter) = 0;
		// Enable/disable effect prewarm.
		virtual void						EffectSetPrewarmEnable(StandaloneEmitter *emitter, bool enable) = 0;
		// Get whether effect prewarm is enabled.
		virtual bool						EffectGetPrewarmEnable(StandaloneEmitter *emitter) = 0;
		/**
		*	Sets the emitter's prewarm time (overrides the default parent effect's prewarm time).
		*	This needs to be called before the emitter is started and the emitter needs to have Prewarm Enable true.
		*/
		virtual void						EffecSetPrewarmTime(StandaloneEmitter *emitter, float time) = 0;
		// Get prewarm override time
		virtual float						EffecGetPrewarmTime(StandaloneEmitter *emitter) = 0;
		// Get the number of attributes in the effect.
		virtual AZ::u32						GetAttributesCount(StandaloneEmitter *emitter) = 0;
		// Get the type of an attribute.
		virtual AZ::s32						GetAttributeType(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		// Get Attribute Id
		virtual AZ::s32						EffectGetAttributeId(StandaloneEmitter *emitter, const AZStd::string &name) = 0;
		// Reset Attribute
		virtual bool						EffectResetAttributeToDefault(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		// Sets Attribute
		virtual bool						EffectSetAttributeAsFloat(StandaloneEmitter *emitter, AZ::u32 attribId, float value) = 0;
		virtual bool						EffectSetAttributeAsFloat2(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector2 &value) = 0;
		virtual bool						EffectSetAttributeAsFloat3(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector3 &value) = 0;
		virtual bool						EffectSetAttributeAsFloat4(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Vector4 &value) = 0;
		virtual bool						EffectSetAttributeAsInt(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 value) = 0;
		virtual bool						EffectSetAttributeAsInt2(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY) = 0;
		virtual bool						EffectSetAttributeAsInt3(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ) = 0;
		virtual bool						EffectSetAttributeAsInt4(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW) = 0;
		virtual bool						EffectSetAttributeAsBool(StandaloneEmitter *emitter, AZ::u32 attribId, bool value) = 0;
		virtual bool						EffectSetAttributeAsBool2(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY) = 0;
		virtual bool						EffectSetAttributeAsBool3(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY, bool valueZ) = 0;
		virtual bool						EffectSetAttributeAsBool4(StandaloneEmitter *emitter, AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW) = 0;
		virtual bool						EffectSetAttributeAsQuaternion(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Quaternion &value) = 0;
		virtual bool						EffectSetAttributeAsColor(StandaloneEmitter *emitter, AZ::u32 attribId, const AZ::Color &value) = 0;
		// Gets Attribute
		virtual float						EffectGetAttributeAsFloat(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::Vector2					EffectGetAttributeAsFloat2(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::Vector3					EffectGetAttributeAsFloat3(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::Vector4					EffectGetAttributeAsFloat4(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::s32						EffectGetAttributeXAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::s32						EffectGetAttributeYAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::s32						EffectGetAttributeZAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::s32						EffectGetAttributeWAsInt(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual bool						EffectGetAttributeXAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual bool						EffectGetAttributeYAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual bool						EffectGetAttributeZAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual bool						EffectGetAttributeWAsBool(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::Quaternion				EffectGetAttributeAsQuaternion(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		virtual AZ::Color					EffectGetAttributeAsColor(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;

		virtual AZ::s32						EffectGetAttributeSamplersCount(StandaloneEmitter *emitter) = 0;
		// Sets Attribute Sampler Id
		virtual AZ::s32						EffectGetAttributeSamplerId(StandaloneEmitter *emitter, const AZStd::string &name) = 0;
		// Sets Sampler
		virtual bool						EffectSetAttributeSampler(StandaloneEmitter *emitter, AZ::u32 attribId, AZ::EntityId entityId) = 0;
		// Reset Sampler
		virtual bool						EffectResetSamplerToDefault(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;
		// Gets Attribute
		virtual AZ::EntityId				EffectGetAttributeSampler(StandaloneEmitter *emitter, AZ::u32 attribId) = 0;

		virtual void						EffectSetTeleportThisFrame(StandaloneEmitter *emitter) = 0;

		// Set LODBias
		virtual float						GetLODBias() = 0;
		virtual void						SetLODBias(float bias) = 0;

		// Change threadpool
		virtual void						SetBakingThreadpool() = 0;
	};
	using PopcornFXRequestBus = AZ::EBus<PopcornFXRequests>;

	class PopcornFXDestructionRequests
		: public AZ::EBusTraits
	{
	public:
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		static const bool EnableEventQueue = true;
		typedef AZStd::recursive_mutex EventQueueMutexType;

		virtual void DestroyEffect(StandaloneEmitter *emitter) = 0;
	};
	using PopcornFXDestructionRequestBus = AZ::EBus<PopcornFXDestructionRequests>;

	class PopcornFXEmitterComponentRequests
		: public AZ::ComponentBus
	{
	public:
		// Enable/disable current emitter.
		virtual void			Enable(bool enable) = 0;
		// Start the particle emission on the current emitter.
		virtual bool			Start() = 0;
		// Stop the particle emission on the current emitter.
		virtual bool			Stop() = 0;
		// Terminate the current emitter (attributes and transform are no longer updated).
		virtual bool			Terminate() = 0;
		// Kill all particles of the current emitter.
		virtual bool			Kill() = 0;
		// Restart the current emitter.
		virtual bool			Restart(bool killOnRestart) = 0;
		// Get whether current emitter is loaded.
		virtual bool			IsLoaded() = 0;
		// Get whether current emitter is enabled.
		virtual bool			IsEnabled() = 0;
		// Get whether current emitter is playing.
		virtual bool			IsPlaying() = 0;
		// Sets up an effect emitter by id and enable.
		virtual bool			SetupEmitterById(const AZ::Data::AssetId &assetId, bool enable) = 0;
		// Sets up an effect emitter by name and enable.
		virtual bool			SetupEmitterByName(const AZStd::string &path, bool enable) = 0;
		// Set effect visibility.
		virtual void			SetVisible(bool visible) = 0;
		// Get effect visibility.
		virtual bool			GetVisible() = 0;
		// Set effect time scale.
		virtual void			SetTimeScale(float timeScale) = 0;
		// Get effect time scale.
		virtual float			GetTimeScale() = 0;
		// Enable/disable effect prewarm.
		virtual void			SetPrewarmEnable(bool enable) = 0;
		// Get whether effect prewarm is enabled.
		virtual bool			GetPrewarmEnable() = 0;
		/**
		*	Sets the emitter's prewarm time (overrides the default parent effect's prewarm time).
		*	This needs to be called before the emitter is started and the emitter needs to have Prewarm Enable true.
		*/
		virtual void			SetPrewarmTime(float time) = 0;
		// Get prewarm override time
		virtual float			GetPrewarmTime() = 0;
		// Get the number of attributes in the effect.
		virtual AZ::u32			GetAttributesCount() = 0;
		// Get the name of an attribute.
		virtual AZStd::string	GetAttributeName(AZ::u32 attribId) = 0;
		// Get the type of an attribute.
		virtual AZ::s32			GetAttributeType(AZ::u32 attribId) = 0;
		// Get Attribute Id
		virtual AZ::s32			GetAttributeId(const AZStd::string &name) = 0;
		// Reset Attribute
		virtual bool			ResetAttributeToDefault(AZ::u32 attribId) = 0;
		// Sets Attribute
		virtual bool			SetAttributeAsFloat(AZ::u32 attribId, float value) = 0;
		virtual bool			SetAttributeAsFloat2(AZ::u32 attribId, const AZ::Vector2 &value) = 0;
		virtual bool			SetAttributeAsFloat3(AZ::u32 attribId, const AZ::Vector3 &value) = 0;
		virtual bool			SetAttributeAsFloat4(AZ::u32 attribId, const AZ::Vector4 &value) = 0;
		virtual bool			SetAttributeAsInt(AZ::u32 attribId, AZ::s32 value) = 0;
		virtual bool			SetAttributeAsInt2(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY) = 0;
		virtual bool			SetAttributeAsInt3(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ) = 0;
		virtual bool			SetAttributeAsInt4(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW) = 0;
		virtual bool			SetAttributeAsBool(AZ::u32 attribId, bool value) = 0;
		virtual bool			SetAttributeAsBool2(AZ::u32 attribId, bool valueX, bool valueY) = 0;
		virtual bool			SetAttributeAsBool3(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ) = 0;
		virtual bool			SetAttributeAsBool4(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW) = 0;
		virtual bool			SetAttributeAsQuaternion(AZ::u32 attribId, const AZ::Quaternion &value) = 0;
		virtual bool			SetAttributeAsColor(AZ::u32 attribId, const AZ::Color &value) = 0;
		// Gets Attribute
		virtual float			GetAttributeAsFloat(AZ::u32 attribId) = 0;
		virtual AZ::Vector2		GetAttributeAsFloat2(AZ::u32 attribId) = 0;
		virtual AZ::Vector3		GetAttributeAsFloat3(AZ::u32 attribId) = 0;
		virtual AZ::Vector4		GetAttributeAsFloat4(AZ::u32 attribId) = 0;
		virtual AZ::s32			GetAttributeXAsInt(AZ::u32 attribId) = 0;
		virtual AZ::s32			GetAttributeYAsInt(AZ::u32 attribId) = 0;
		virtual AZ::s32			GetAttributeZAsInt(AZ::u32 attribId) = 0;
		virtual AZ::s32			GetAttributeWAsInt(AZ::u32 attribId) = 0;
		virtual bool			GetAttributeXAsBool(AZ::u32 attribId) = 0;
		virtual bool			GetAttributeYAsBool(AZ::u32 attribId) = 0;
		virtual bool			GetAttributeZAsBool(AZ::u32 attribId) = 0;
		virtual bool			GetAttributeWAsBool(AZ::u32 attribId) = 0;
		virtual AZ::Quaternion	GetAttributeAsQuaternion(AZ::u32 attribId) = 0;
		virtual AZ::Color		GetAttributeAsColor(AZ::u32 attribId) = 0;

		virtual AZ::u32			GetAttributeSamplersCount() = 0;
		// Get Sampler ID
		virtual AZ::s32			GetAttributeSamplerId(const AZStd::string &name) = 0;
		// Sets Sampler
		virtual bool			SetAttributeSampler(AZ::u32 attribSamplerId, AZ::EntityId entityId) = 0;
		// Reset Sampler
		virtual bool			ResetSamplerToDefault(AZ::u32 attribSamplerId) = 0;
		// Gets Attribute
		virtual AZ::EntityId	GetAttributeSampler(AZ::u32 attribSamplerId) = 0;

		// Set teleport flag
		virtual void			SetTeleportThisFrame() = 0;
	};
	using PopcornFXEmitterComponentRequestBus = AZ::EBus <PopcornFXEmitterComponentRequests>;

	class PopcornFXEmitterComponentEvents
		: public AZ::ComponentBus
	{
	public:
		/**
		* Custom connection policy to make sure all we are fully in sync
		*/
		template <class Bus>
		struct PopcornFXEmitterComponentConnectionPolicy
			: public AZ::EBusConnectionPolicy<Bus>
		{
			static void Connect(typename Bus::BusPtr &busPtr, typename Bus::Context &context,
								typename Bus::HandlerNode &handler, typename Bus::Context::ConnectLockGuard &connectLock,
								const typename Bus::BusIdType &id = 0)
			{
				AZ::EBusConnectionPolicy<Bus>::Connect(busPtr, context, handler, connectLock, id);
				bool loaded = false;
				EBUS_EVENT_ID_RESULT(loaded, id, PopcornFXEmitterComponentRequestBus, IsLoaded);
				if (loaded)
					handler->OnEmitterReady();
			}
		};
		template<typename Bus>
		using ConnectionPolicy = PopcornFXEmitterComponentConnectionPolicy<Bus>;
		//////////////////////////////////////////////////////////////////////////

		virtual void	OnEmitterReady() { }
		virtual void	OnEmitterBroadcastEvent(SBroadcastParams*) { }
	};
	using PopcornFXEmitterComponentEventsBus = AZ::EBus <PopcornFXEmitterComponentEvents>;

	class PopcornFXEmitterEditorComponentEvents
		: public AZ::ComponentBus
	{
	public:
		virtual void	OnAttributeChanged() = 0;
	};
	using PopcornFXEmitterEditorComponentEventsBus = AZ::EBus <PopcornFXEmitterEditorComponentEvents>;

	static const AZ::Uuid EmitterComponentTypeId = AZ::Uuid("{515957e3-8354-4048-8d6c-98628ef21804}");
	static const AZ::Uuid EditorEmitterComponentTypeId = AZ::Uuid("{B62ED02E-731B-4ACD-BCA1-78EF92528228}");

	class PopcornFXProfilerRequests
		: public AZ::EBusTraits
	{
	public:
		virtual bool	WriteProfileReport(const AZStd::string &path) const = 0;
		virtual bool	ProfilerSetEnable(bool enable) const = 0;
	};
	using PopcornFXProfilerRequestBus = AZ::EBus<PopcornFXProfilerRequests>;

	class PopcornFXTrackViewComponentRequests
		: public AZ::ComponentBus
	{
	public:
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

		virtual void	SetValueFX(float value) = 0;
		virtual float	GetValueFX() const = 0;
		virtual void	SetValueFY(float value) = 0;
		virtual float	GetValueFY() const = 0;
		virtual void	SetValueFZ(float value) = 0;
		virtual float	GetValueFZ() const = 0;
		virtual void	SetValueFW(float value) = 0;
		virtual float	GetValueFW() const = 0;

		virtual void	SetValueIX(AZ::u32 value) = 0;
		virtual AZ::u32	GetValueIX() const = 0;
		virtual void	SetValueIY(AZ::u32 value) = 0;
		virtual AZ::u32	GetValueIY() const = 0;
		virtual void	SetValueIZ(AZ::u32 value) = 0;
		virtual AZ::u32	GetValueIZ() const = 0;
		virtual void	SetValueIW(AZ::u32 value) = 0;
		virtual AZ::u32	GetValueIW() const = 0;

		virtual void	SetValueBX(bool value) = 0;
		virtual bool	GetValueBX() const = 0;
		virtual void	SetValueBY(bool value) = 0;
		virtual bool	GetValueBY() const = 0;
		virtual void	SetValueBZ(bool value) = 0;
		virtual bool	GetValueBZ() const = 0;
		virtual void	SetValueBW(bool value) = 0;
		virtual bool	GetValueBW() const = 0;

		virtual void			SetValueQuat(const AZ::Quaternion &value) = 0;
		virtual AZ::Quaternion	GetValueQuat() const = 0;
		virtual void			SetValueColor(const AZ::Color &value) = 0;
		virtual AZ::Color		GetValueColor() const = 0;
		virtual void			SetValueColorAlpha(float value) = 0;
		virtual float			GetValueColorAlpha() const = 0;

	};
	using PopcornFXTrackViewComponentRequestBus = AZ::EBus<PopcornFXTrackViewComponentRequests>;
}

