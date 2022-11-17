//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXEmitterRuntime.h"

#if defined(O3DE_USE_PK)
#include "Integration/PopcornFXUtils.h"
#endif

#include <PopcornFX/PopcornFXBus.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Asset/AssetSerializer.h>

namespace PopcornFX {

	PopcornFXEmitterRuntime::PopcornFXEmitterRuntime()
		: m_Asset(AZ::Data::AssetLoadBehavior::QueueLoad)
	{
	}

	void	PopcornFXEmitterRuntime::Reflect(AZ::ReflectContext *reflection)
	{
		PopcornFXAttributeList::Reflect(reflection);

		if (auto *serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			// Serializer:
			serializeContext->Class<PopcornFXEmitterRuntime>()
				->Version(1)
				->Field("Enable", &PopcornFXEmitterRuntime::m_Enable)
				->Field("TimeScale", &PopcornFXEmitterRuntime::m_TimeScale)
				->Field("PrewarmEnable", &PopcornFXEmitterRuntime::m_PrewarmEnable)
				->Field("PrewarmTime", &PopcornFXEmitterRuntime::m_PrewarmTime)
				->Field("Visible", &PopcornFXEmitterRuntime::m_Visible)
				->Field("ParticleSystem", &PopcornFXEmitterRuntime::m_Asset)
				->Field("AttributeList", &PopcornFXEmitterRuntime::m_AttributeList)
				;
		}
	}

	// Private Static
	bool	PopcornFXEmitterRuntime::VersionConverter(	AZ::SerializeContext &context,
														AZ::SerializeContext::DataElementNode &classElement)
	{
		(void)context; (void)classElement;
		//if (classElement.GetVersion() == 0)
		//{
		//}
		return true;
	}

	void	PopcornFXEmitterRuntime::Init(bool enable, const PopcornFXAttributeList &attributeList)
	{
		m_Enable = enable;
		m_AttributeList = attributeList;
	}

	void	PopcornFXEmitterRuntime::Init(bool enable, StandaloneEmitter *standaloneEmitter)
	{
		m_Enable = enable;
		m_StandaloneEmitter = standaloneEmitter;
	}

	void	PopcornFXEmitterRuntime::Activate()
	{
#if defined(O3DE_USE_PK)
		m_Emitter.Activate();
#endif //O3DE_USE_PK
		_SetupAsset();
	}

	void	PopcornFXEmitterRuntime::Deactivate()
	{
#if defined(O3DE_USE_PK)
		m_Emitter.Deactivate();
		m_Emitter.Clear();
		m_Emitter.AttachToEntity(AZ::EntityId(), false);
#endif //O3DE_USE_PK
		AZ::Data::AssetBus::Handler::BusDisconnect();
		m_Asset.Release();
	}

	void	PopcornFXEmitterRuntime::Enable(bool enable)
	{
		if (enable == m_Enable)
			return;
		m_Enable = enable;

#if defined(O3DE_USE_PK)
		if (enable)
		{
			if (m_Emitter.GetEffect() == null)
			{
				//asset not loaded yet, will be started in _LoadAndSet
				return;
			}

			PK_ASSERT(!m_Emitter.IsPlaying());
			const bool	started = m_Emitter.Start();
			if (started)
			{
				m_Emitter.SetTimeScale(m_TimeScale);
				m_Emitter.SetVisible(m_Visible);
			}
		}
		else
		{
			m_Emitter.Kill();
		}
#endif //O3DE_USE_PK
	}

	bool	PopcornFXEmitterRuntime::_LoadAndSet()
	{
		PopcornFXAsset	*pkAsset = m_Asset.GetAs<PopcornFXAsset>();

#if !defined(O3DE_USE_PK)
		return pkAsset != null;
#else

		if (pkAsset != null && pkAsset->m_Effect != null)
		{
			const CParticleAttributeList	*defaultList = pkAsset->m_Effect->AttributeFlatList().Get();

			m_AttributeList.Prepare(defaultList);
			m_Emitter.Clear();
			m_Emitter.Set(pkAsset, m_AttributeList);
			_SetupStandaloneEmitterAttributes();
			if (m_PrewarmTime < 0.0f)
			{
				m_PrewarmEnable = pkAsset->m_Effect->EnablePrewarm();
				m_PrewarmTime = pkAsset->m_Effect->PrewarmTime();
			}
			m_Emitter.SetPrewarmEnable(m_PrewarmEnable);
			m_Emitter.SetPrewarmTime(m_PrewarmTime);
			if (m_Enable)
			{
				const bool	started = m_Emitter.Start();
				if (started)
				{
					m_Emitter.SetTimeScale(m_TimeScale);
					m_Emitter.SetVisible(m_Visible);
				}
				return started;
			}
			return true;
		}
		else
		{
			AZStd::string	path;
			EBUS_EVENT_RESULT(path, AZ::Data::AssetCatalogRequestBus, GetAssetPathById, m_Asset.GetId());
			AZ_Warning("PopcornFX", false, "Unable to load the effect %s.", path.c_str());
		}
		return false;
#endif //O3DE_USE_PK
	}

	void	PopcornFXEmitterRuntime::_SetupStandaloneEmitterAttributes()
	{
#if defined(O3DE_USE_PK)
		for (const auto &attr : m_StandaloneEmitterAttributes)
		{
			AZ::s32	attrId = m_Emitter.GetAttributeId(attr.m_Name);
			if (attrId != -1)
			{
				AZ::s32	type = m_Emitter.GetAttributeType(attrId);
				switch (type)
				{
				case EPopcornFXType::Type_Float:
					m_Emitter.SetAttribute(attrId, CFloat1(attr.m_FloatX));
					break;
				case EPopcornFXType::Type_Float2:
					m_Emitter.SetAttribute(attrId, CFloat2(attr.m_FloatX, attr.m_FloatY));
					break;
				case EPopcornFXType::Type_Float3:
					m_Emitter.SetAttribute(attrId, CFloat3(attr.m_FloatX, attr.m_FloatY, attr.m_FloatZ));
					break;
				case EPopcornFXType::Type_Float4:
				case EPopcornFXType::Type_Quaternion:
					m_Emitter.SetAttribute(attrId, CFloat4(attr.m_FloatX, attr.m_FloatY, attr.m_FloatZ, attr.m_FloatW));
					break;
				case EPopcornFXType::Type_Int:
					m_Emitter.SetAttribute(attrId, CInt1(attr.m_IntX));
					break;
				case EPopcornFXType::Type_Int2:
					m_Emitter.SetAttribute(attrId, CInt2(attr.m_IntX, attr.m_IntY));
					break;
				case EPopcornFXType::Type_Int3:
					m_Emitter.SetAttribute(attrId, CInt3(attr.m_IntX, attr.m_IntY, attr.m_IntZ));
					break;
				case EPopcornFXType::Type_Int4:
					m_Emitter.SetAttribute(attrId, CInt4(attr.m_IntX, attr.m_IntY, attr.m_IntZ, attr.m_IntW));
					break;
				case EPopcornFXType::Type_Bool:
					m_Emitter.SetAttribute(attrId, CBool1(attr.m_IntX));
					break;
				case EPopcornFXType::Type_Bool2:
					m_Emitter.SetAttribute(attrId, CBool2(attr.m_IntX, attr.m_IntY));
					break;
				case EPopcornFXType::Type_Bool3:
					m_Emitter.SetAttribute(attrId, CBool3(attr.m_IntX, attr.m_IntY, attr.m_IntZ));
					break;
				case EPopcornFXType::Type_Bool4:
					m_Emitter.SetAttribute(attrId, CBool4(attr.m_IntX, attr.m_IntY, attr.m_IntZ, attr.m_IntW));
					break;
				default:
					break;
				}
			}
			else
			{
				AZ_Warning("PopcornFX", false, "Unable to set attribute %s: it does not exist in this effect.", attr.m_Name.c_str());
			}
		}

		for (const auto &smp : m_StandaloneEmitterAttributeSamplers)
		{
			AZ::s32	smpId = m_Emitter.GetAttributeSamplerId(smp.m_Name);
			if (smpId != -1)
			{
				m_Emitter.SetAttributeSampler(smpId, smp.m_SamplerEntity);
			}
			else
			{
				AZ_Warning("PopcornFX", false, "Unable to set attribute sampler %s: it does not exist in this effect.", smp.m_Name.c_str());
			}
		}
#endif //O3DE_USE_PK
	}

	bool	PopcornFXEmitterRuntime::_SetupAsset()
	{
		if (m_AssetId.IsValid())
		{
			PopcornFXRequestBus::Broadcast(&PopcornFXRequestBus::Events::PreloadEffectById, m_AssetId);
			//NoLoad because it is queue load in PreloadEffectById
			m_Asset = AZ::Data::AssetManager::Instance().GetAsset(m_AssetId, azrtti_typeid<PopcornFXAsset>(), AZ::Data::AssetLoadBehavior::NoLoad);
		}
		else if (m_Asset.GetId().IsValid())
		{
			PopcornFXRequestBus::Broadcast(&PopcornFXRequestBus::Events::PreloadEffectById, m_Asset.GetId());
		}
		else
			return false;


		if (!AZ::Data::AssetBus::Handler::BusIsConnectedId(m_Asset.GetId()))
		{
			AZ::Data::AssetBus::Handler::BusDisconnect();
			AZ::Data::AssetBus::Handler::BusConnect(m_Asset.GetId());
		}
		return true;
	}

	void	PopcornFXEmitterRuntime::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
	{
		OnAssetReady(asset);
	}

	void	PopcornFXEmitterRuntime::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
	{
		m_Asset = asset;

		if (m_Asset.IsReady())
			_LoadAndSet();
		if (m_StandaloneEmitter != null)
			PopcornFXEventsBus::Broadcast(&PopcornFXEventsBus::Events::OnEmitterReady, m_StandaloneEmitter);
		else if (m_EntityId.IsValid())
			PopcornFXEmitterComponentEventsBus::Event(m_EntityId, &PopcornFXEmitterComponentEventsBus::Events::OnEmitterReady);
	}

	void	PopcornFXEmitterRuntime::SetVisible(bool visible)
	{
		m_Visible = visible;
#if defined(O3DE_USE_PK)
		m_Emitter.SetVisible(m_Visible);
#endif
	}

	bool	PopcornFXEmitterRuntime::GetVisible()
	{
		return m_Visible;
	}

	void	PopcornFXEmitterRuntime::SetTimeScale(float timeScale)
	{
		m_TimeScale = timeScale;
#if defined(O3DE_USE_PK)
		m_Emitter.SetTimeScale(m_TimeScale);
#endif
	}

	float	PopcornFXEmitterRuntime::GetTimeScale()
	{
		return m_TimeScale;
	}

	void	PopcornFXEmitterRuntime::SetPrewarmEnable(bool enable)
	{
		m_PrewarmEnable = enable;
#if defined(O3DE_USE_PK)
		m_Emitter.SetPrewarmEnable(m_PrewarmEnable);
#endif
	}

	bool	PopcornFXEmitterRuntime::GetPrewarmEnable()
	{
		return m_PrewarmEnable;
	}

	void	PopcornFXEmitterRuntime::SetPrewarmTime(float time)
	{
		m_PrewarmTime = time;
#if defined(O3DE_USE_PK)
		m_Emitter.SetPrewarmTime(m_PrewarmTime);
#endif
	}

	float	PopcornFXEmitterRuntime::GetPrewarmTime()
	{
		return m_PrewarmTime;
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsFloat(AZ::u32 attribId, float value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		CFloat1	pkValue(value);
		return m_Emitter.SetAttribute(attribId, pkValue);
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsFloat2(AZ::u32 attribId, const AZ::Vector2 &value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, ToPk(value));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsFloat3(AZ::u32 attribId, const AZ::Vector3 &value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, ToPk(value));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsFloat4(AZ::u32 attribId, const AZ::Vector4 &value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, ToPk(value));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsInt(AZ::u32 attribId, AZ::s32 value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CInt1(value));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsInt2(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CInt2(valueX, valueY));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsInt3(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CInt3(valueX, valueY, valueZ));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsInt4(AZ::u32 attribId, AZ::s32 valueX, AZ::s32 valueY, AZ::s32 valueZ, AZ::s32 valueW)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CInt4(valueX, valueY, valueZ, valueW));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsBool(AZ::u32 attribId, bool value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CBool1(value));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsBool2(AZ::u32 attribId, bool valueX, bool valueY)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CBool2(valueX, valueY));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsBool3(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CBool3(valueX, valueY, valueZ));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsBool4(AZ::u32 attribId, bool valueX, bool valueY, bool valueZ, bool valueW)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, CBool4(valueX, valueY, valueZ, valueW));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsQuaternion(AZ::u32 attribId, const AZ::Quaternion &value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		return m_Emitter.SetAttribute(attribId, ToPk(value));
#endif
	}

	bool	PopcornFXEmitterRuntime::SetAttributeAsColor(AZ::u32 attribId, const AZ::Color &value)
	{
#if !defined(O3DE_USE_PK)
		return false;
#else
		const EPopcornFXType	type = static_cast<EPopcornFXType>(m_Emitter.GetAttributeType(attribId));

		if (type == EPopcornFXType::Type_Float3)
		{
			return SetAttributeAsFloat3(attribId, value.GetAsVector3());
		}
		else
		{
#if PK_ATTRIB_ENABLE_CHECKS
			if (type != EPopcornFXType::Type_Float4)
			{
				AZ_Warning("PopcornFX", false, "SetAttribute: the Attribute [%d] \"%s\" cannot be set as Color: the attribute is %s",
					attribId, m_Emitter.GetAttributeName(attribId).c_str(), O3DEPopcornFXTypeToString(type).c_str());
				return false;
			}
#endif
			return SetAttributeAsFloat4(attribId, value.GetAsVector4());
		}
#endif
	}

	float	PopcornFXEmitterRuntime::GetAttributeAsFloat(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32f[0];
		}
#endif
		return 0.0f;
	}

	AZ::Vector2	PopcornFXEmitterRuntime::GetAttributeAsFloat2(AZ::u32 attribId)
	{
		AZ::Vector2						value(0.0f);
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
		}
#endif
		return value;
	}

	AZ::Vector3	PopcornFXEmitterRuntime::GetAttributeAsFloat3(AZ::u32 attribId)
	{
		AZ::Vector3						value(0.0f);
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
			value.SetZ(attrib.m_Data32f[2]);
		}
#endif
		return value;
	}

	AZ::Vector4	PopcornFXEmitterRuntime::GetAttributeAsFloat4(AZ::u32 attribId)
	{
		AZ::Vector4						value(0.0f);
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
			value.SetZ(attrib.m_Data32f[2]);
			value.SetW(attrib.m_Data32f[3]);
		}
#endif
		return value;
	}

	AZ::s32	PopcornFXEmitterRuntime::GetAttributeXAsInt(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[0];
		}
#endif
		return 0;
	}

	AZ::s32	PopcornFXEmitterRuntime::GetAttributeYAsInt(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[1];
		}
#endif
		return 0;
	}

	AZ::s32	PopcornFXEmitterRuntime::GetAttributeZAsInt(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[2];
		}
#endif
		return 0;
	}

	AZ::s32	PopcornFXEmitterRuntime::GetAttributeWAsInt(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data32i[3];
		}
#endif
		return 0;
	}

	bool	PopcornFXEmitterRuntime::GetAttributeXAsBool(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[0];
		}
#endif
		return false;
	}

	bool	PopcornFXEmitterRuntime::GetAttributeYAsBool(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[1];
		}
#endif
		return false;
	}

	bool	PopcornFXEmitterRuntime::GetAttributeZAsBool(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[2];
		}
#endif
		return false;
	}

	bool	PopcornFXEmitterRuntime::GetAttributeWAsBool(AZ::u32 attribId)
	{
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			return attrib.m_Data8b[3];
		}
#endif
		return false;
	}

	AZ::Quaternion	PopcornFXEmitterRuntime::GetAttributeAsQuaternion(AZ::u32 attribId)
	{
		AZ::Quaternion value = AZ::Quaternion::CreateIdentity();
#if defined(O3DE_USE_PK)
		SAttributesContainer::SAttrib	attrib;
		if (m_Emitter.GetAttribute(attribId, attrib))
		{
			value.SetX(attrib.m_Data32f[0]);
			value.SetY(attrib.m_Data32f[1]);
			value.SetZ(attrib.m_Data32f[2]);
			value.SetW(attrib.m_Data32f[3]);
		}
#endif
		return value;
	}

	AZ::Color	PopcornFXEmitterRuntime::GetAttributeAsColor(AZ::u32 attribId)
	{
		AZ::Color value(0.0f);
#if defined(O3DE_USE_PK)
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
#endif
		return value;
	}

}
