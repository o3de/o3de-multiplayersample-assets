//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXAttributeList.h"

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_classes.h>
#endif //O3DE_USE_PK

namespace PopcornFX {

void	PopcornFXAttribute::Reflect(AZ::ReflectContext *context)
{
	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXAttribute>()
			->Version(1, &VersionConverter)
			->Field("Name", &PopcornFXAttribute::m_Name)
			->Field("Type", &PopcornFXAttribute::m_Type)
			;
	}
}

// Private Static
bool	PopcornFXAttribute::VersionConverter(	AZ::SerializeContext &context,
												AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context; (void)classElement;
	return true;
}

void	PopcornFXSampler::Reflect(AZ::ReflectContext *context)
{
	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXSampler>()
			->Version(1, &VersionConverter)
			->Field("Name", &PopcornFXSampler::m_Name)
			->Field("Type", &PopcornFXSampler::m_Type)
			->Field("EntityId", &PopcornFXSampler::m_EntityId)
			;
	}
}

// Private Static
bool	PopcornFXSampler::VersionConverter(	AZ::SerializeContext &context,
											AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context; (void)classElement;
	return true;
}

PopcornFXAttributeList::PopcornFXAttributeList()
{
}

void	PopcornFXAttributeList::Reflect(AZ::ReflectContext *context)
{
	PopcornFXAttribute::Reflect(context);
	PopcornFXSampler::Reflect(context);

	AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
	if (serializeContext)
	{
		serializeContext->Class<PopcornFXAttributeList>()
			->Version(1, &VersionConverter)
			->Field("Attributes", &PopcornFXAttributeList::m_Attributes)
			->Field("Samplers", &PopcornFXAttributeList::m_Samplers)
			->Field("AttributesRawData", &PopcornFXAttributeList::m_AttributesRawData)
			;
	}
}

// Private Static
bool	PopcornFXAttributeList::VersionConverter(	AZ::SerializeContext &context,
													AZ::SerializeContext::DataElementNode &classElement)
{
	(void)context; (void)classElement;
	return true;
}

void	PopcornFXAttributeList::OnSamplerReady(const AZ::EntityId &entityId)
{
	PopcornFXSamplerComponentEventsBus::MultiHandler::BusDisconnect(entityId);

	for (size_t i = 0; i < m_Samplers.size(); ++i)
	{
		if (m_Samplers[i].m_EntityId == entityId)
		{
			m_Samplers[i].m_Ready = true;
		}
	}
}

#if defined(O3DE_USE_PK)

bool	PopcornFXAttributeList::SamplersReady()
{
	bool	allSamplersReady = true;
	for (size_t i = 0; i < m_Samplers.size(); ++i)
	{
		allSamplersReady &= m_Samplers[i].m_Ready;
	}
	return allSamplersReady;
}


bool	PopcornFXAttributeList::Prepare(const CParticleAttributeList *defaultList)
{
	if (defaultList == null)
	{
		Clear();
		return false;
	}

	if (IsEmpty())
	{
		_CopyAttributes(defaultList);
		_CopySamplers(defaultList);
		_SetSamplersStatus();
		return true;
	}

	//Re-match attributes
	if (defaultList->AttributeList().Count() == 0)
	{
		m_Attributes.clear();
		m_AttributesRawData.clear();
	}
	else if (m_Attributes.size() == 0)
	{
		_CopyAttributes(defaultList);
	}
	else
	{
		//Update Attributes
		TMemoryView<const CParticleAttributeDeclaration * const>	defAttribDescs = defaultList->UniqueAttributeList();
		TMemoryView<const SAttributesContainer::SAttrib>			defAttribs = (*defaultList->DefaultAttributes())->Attributes();

		PK_ASSERT(defAttribDescs.Count() == defAttribs.Count());

		m_Attributes.reserve(defAttribs.Count());
		m_AttributesRawData.reserve(defAttribs.CoveredBytes());

		AZ::u32	attri = 0;
		for (; attri < defAttribs.Count() && attri < m_Attributes.size(); ++attri)
		{
			const char		*attrName = defAttribDescs[attri]->ExportedName().Data();
			const AZ::u32	attrType = defAttribDescs[attri]->ExportedType();

			//Check name
			if (m_Attributes[attri].m_Name != attrName)
			{
				CGuid	found;
				for (AZ::u32 findAttri = attri + 1; findAttri < m_Attributes.size(); ++findAttri)
				{
					if (m_Attributes[findAttri].m_Name == attrName)
					{
						found = findAttri;
						break;
					}
				}
				if (!found.Valid())
				{
					// (Can result to superfluous copies, but, here, dont care, and prefer simpler code)
					// Copy the refAttr/default to the back
					// and make it the found one...
					m_Attributes.push_back({});
					found = m_Attributes.size() - 1;
					m_Attributes[found].m_Name = attrName;
					m_Attributes[found].m_Type = attrType;
					m_AttributesRawData.resize(m_AttributesRawData.size() + 1 * sizeof(SAttributesContainer::SAttrib));
					_CopyAttributeToRaw(found, defAttribs[attri]);
				}
				_SwapAttributes(attri, found);
				_SwapRawData(attri, found);
				PK_ASSERT(m_Attributes[attri].m_Name == attrName);
			}

			//Check type
			if (m_Attributes[attri].m_Type != attrType)
			{
				// If attribute changed float <-> int, reset value to default
				if (CBaseTypeTraits::Traits((EBaseTypeID)m_Attributes[attri].m_Type).IsFp != CBaseTypeTraits::Traits((EBaseTypeID)attrType).IsFp)
				{
					_CopyAttributeToRaw(attri, defAttribs[attri]);
				}
				m_Attributes[attri].m_Type = attrType;
			}
			// Name and Type must match now
			PK_ASSERT(m_Attributes[attri].m_Name == attrName);
			PK_ASSERT(m_Attributes[attri].m_Type == attrType);
		}
		// Removes removed attributes OR Sets new one to default
		m_Attributes.resize(defAttribs.Count());
		m_AttributesRawData.resize(defAttribs.CoveredBytes());
		for (; attri < defAttribs.Count(); ++attri)
		{
			m_Attributes[attri].m_Name = defAttribDescs[attri]->ExportedName().Data();
			m_Attributes[attri].m_Type = defAttribDescs[attri]->ExportedType();
			_CopyAttributeToRaw(attri, defAttribs[attri]);
		}
	}

	//Re-match samplers
	if (defaultList->UniqueSamplerList().Count() == 0)
	{
		m_Samplers.clear();
	}
	else if (m_Samplers.size() == 0)
	{
		_CopySamplers(defaultList);
	}
	else
	{
		//Update Samplers
		TMemoryView<const CParticleAttributeSamplerDeclaration * const>	srcSamplers = defaultList->UniqueSamplerList();

		m_Samplers.reserve(srcSamplers.Count());

		AZ::u32	sampleri = 0;
		for (; sampleri < srcSamplers.Count() && sampleri < m_Samplers.size(); ++sampleri)
		{
			const char	*samplerName = srcSamplers[sampleri] == null ? "" : srcSamplers[sampleri]->ExportedName().Data();
			AZ::u32		samplerType = (AZ::u32)-1;

			if (srcSamplers[sampleri] != null)
			{
				const PParticleSamplerDescriptor	&desc = srcSamplers[sampleri]->GetSamplerDefaultDescriptor();
				if (desc != null)
					samplerType = (AZ::u32)desc->SamplerTypeID();
			}

			//Check name
			if (m_Samplers[sampleri].m_Name != samplerName)
			{
				CGuid	found;
				for (AZ::u32 findSampleri = 0; findSampleri < m_Samplers.size(); ++findSampleri)
				{
					if (m_Samplers[findSampleri].m_Name == samplerName)
					{
						found = findSampleri;
						break;
					}
				}
				if (!found.Valid())
				{
					// (Can result to superfluous copies, but, here, dont care, and prefer simpler code)
					m_Samplers.push_back({});
					found = m_Samplers.size() - 1;
					m_Samplers[found].m_Name = samplerName;
					m_Samplers[found].m_Type = samplerType;
				}
				_SwapSamplers(sampleri, found);
				PK_ASSERT(m_Samplers[sampleri].m_Name == samplerName);
			}

			//Check type
			if (m_Samplers[sampleri].m_Type != samplerType)
			{
				// If type missmatch, just set to default (ResetValue())
				m_Samplers[sampleri].m_EntityId = (AZ::EntityId)AZ::EntityId::InvalidEntityId;
				m_Samplers[sampleri].m_Type = samplerType;
			}
			// Name and Type must match now
			PK_ASSERT(m_Samplers[sampleri].m_Name == samplerName);
			PK_ASSERT(m_Samplers[sampleri].m_Type == samplerType);
		}
		// Removes removed samplers OR Sets new one to default
		m_Samplers.resize(srcSamplers.Count());
		for (; sampleri < srcSamplers.Count(); ++sampleri)
		{
			if (srcSamplers[sampleri] != null)
			{
				const PParticleSamplerDescriptor	&desc = srcSamplers[sampleri]->GetSamplerDefaultDescriptor();
				if (!srcSamplers[sampleri]->ExportedName().Empty())
					m_Samplers[sampleri].m_Name = srcSamplers[sampleri]->ExportedName().Data();
				if (desc != null)
					m_Samplers[sampleri].m_Type = (AZ::u32)desc->SamplerTypeID();
			}
		}
	}

	_SetSamplersStatus();
	return true;
}

void	PopcornFXAttributeList::_CopyAttributes(const CParticleAttributeList *attributeList)
{
	if (attributeList != null)
	{
		const u32												attrCount = attributeList->UniqueAttributeList().Count();
		TMemoryView<const CParticleAttributeDeclaration * const>	srcAttribs = attributeList->UniqueAttributeList();

		m_Attributes.resize(attrCount);
		for (u32 i = 0; i < attrCount; i++)
		{
			if (srcAttribs[i] != null)
			{
				if (!srcAttribs[i]->ExportedName().Empty())
					m_Attributes[i].m_Name = srcAttribs[i]->ExportedName().Data();
				m_Attributes[i].m_Type = (AZ::u32)srcAttribs[i]->ExportedType();
			}
		}
		const SAttributesContainer		*defContainer = *(attributeList->DefaultAttributes());
		PK_ASSERT(defContainer != null);

		const AZ::u32	attribBytes = defContainer->Attributes().CoveredBytes();
		m_AttributesRawData.resize(attribBytes);
		if (attribBytes > 0)
		{
			Mem::Copy(m_AttributesRawData.data(), defContainer->Attributes().Data(), attribBytes);
		}
	}
}

void	PopcornFXAttributeList::_CopySamplers(const CParticleAttributeList *attributeList)
{
	if (attributeList != null)
	{
		TMemoryView<const CParticleAttributeSamplerDeclaration * const>	srcSamplers = attributeList->UniqueSamplerList();
		const u32														samplerCount = srcSamplers.Count();

		m_Samplers.resize(samplerCount);
		for (u32 i = 0; i < samplerCount; i++)
		{
			if (srcSamplers[i] != null)
			{
				const PParticleSamplerDescriptor	&desc = srcSamplers[i]->GetSamplerDefaultDescriptor();
				if (!srcSamplers[i]->ExportedName().Empty())
					m_Samplers[i].m_Name = srcSamplers[i]->ExportedName().Data();
				if (desc != null)
					m_Samplers[i].m_Type = (AZ::u32)desc->SamplerTypeID();
			}
		}
	}
}

void	PopcornFXAttributeList::_CopyAttributeToRaw(AZ::u32 idxInRaw, const SAttributesContainer::SAttrib &attrib)
{
	const AZ::u32	attribSize = sizeof(SAttributesContainer::SAttrib);
	Mem::Copy(&m_AttributesRawData[idxInRaw * attribSize], &attrib, attribSize);
}

void	PopcornFXAttributeList::_SwapAttributes(AZ::u32 idxA, AZ::u32 idxB)
{
	const PopcornFXAttribute	attribTemp = m_Attributes[idxA];

	m_Attributes[idxA] = m_Attributes[idxB];
	m_Attributes[idxB] = attribTemp;
}

void	PopcornFXAttributeList::_SwapRawData(AZ::u32 idxInRawA, AZ::u32 idxInRawB)
{
	const AZ::u32	attribSize = sizeof(SAttributesContainer::SAttrib);
	AZ::u32			temp[attribSize];

	Mem::Copy(temp, &m_AttributesRawData[idxInRawA * attribSize], attribSize);
	Mem::Copy(&m_AttributesRawData[idxInRawA * attribSize], &m_AttributesRawData[idxInRawB * attribSize], attribSize);
	Mem::Copy(&m_AttributesRawData[idxInRawB * attribSize], temp, attribSize);
}

void	PopcornFXAttributeList::_SwapSamplers(AZ::u32 idxA, AZ::u32 idxB)
{
	const PopcornFXSampler	samplerTemp = m_Samplers[idxA];

	m_Samplers[idxA] = m_Samplers[idxB];
	m_Samplers[idxB] = samplerTemp;
}

void	PopcornFXAttributeList::_SetSamplersStatus()
{
	for (size_t i = 0; i < m_Samplers.size(); ++i)
	{
		if (m_Samplers[i].m_EntityId != (AZ::EntityId)AZ::EntityId::InvalidEntityId)
		{
			m_Samplers[i].m_Ready = false;
			PopcornFXSamplerComponentEventsBus::MultiHandler::BusConnect(m_Samplers[i].m_EntityId);
		}
		else
			m_Samplers[i].m_Ready = true;
	}
}

EBaseTypeID	PopcornFXAttributeList::GetAttributeBaseType(CGuid id)
{
	if (id >= m_Attributes.size())
		return EBaseTypeID::BaseType_Void;
	return (EBaseTypeID)m_Attributes[id].m_Type;
}

s32	PopcornFXAttributeList::GetAttributeId(const AZStd::string &name)
{
	const u32	attrCount = AttributeCount();
	for (u32 i = 0; i < attrCount; ++i)
	{
		if (m_Attributes[i].m_Name == name)
			return static_cast<s32>(i);
	}
	return -1;
}

s32	PopcornFXAttributeList::GetAttributeSamplerId(const AZStd::string &name)
{
	const u32	samplerCount = SamplerCount();
	for (u32 i = 0; i < samplerCount; ++i)
	{
		if (m_Samplers[i].m_Name == name)
			return static_cast<s32>(i);
	}
	return -1;
}

bool	PopcornFXAttributeList::SetSampler(u32 samplerId, AZ::EntityId entityId)
{
	const u32	samplerCount = SamplerCount();
	if (!PK_VERIFY(samplerId < samplerCount))
		return false;

	if (m_Samplers[samplerId].m_EntityId != (AZ::EntityId)AZ::EntityId::InvalidEntityId &&
		PopcornFXSamplerComponentEventsBus::MultiHandler::BusIsConnectedId(m_Samplers[samplerId].m_EntityId))
		PopcornFXSamplerComponentEventsBus::MultiHandler::BusDisconnect(m_Samplers[samplerId].m_EntityId);

	if (entityId != (AZ::EntityId)AZ::EntityId::InvalidEntityId)
	{
		AZ::u32	type = (AZ::u32) - 1;
		EBUS_EVENT_ID_RESULT(type, entityId, PopcornFX::PopcornFXSamplerComponentRequestBus, GetType);

		if (m_Samplers[samplerId].m_Type != type)
		{
			AZ_Warning("PopcornFX", false, "SetSampler failed for sampler %s: invalid type.", m_Samplers[samplerId].m_Name.c_str());
			return false;
		}
	}

	m_Samplers[samplerId].m_EntityId = entityId;
	if (m_Samplers[samplerId].m_EntityId != (AZ::EntityId)AZ::EntityId::InvalidEntityId)
	{
		m_Samplers[samplerId].m_Ready = false;
		PopcornFXSamplerComponentEventsBus::MultiHandler::BusConnect(m_Samplers[samplerId].m_EntityId);
	}
	return true;
}

PopcornFXSampler	*PopcornFXAttributeList::GetSampler(u32 samplerId)
{
	const u32	samplerCount = SamplerCount();
	if (!PK_VERIFY(samplerId < samplerCount))
		return null;
	return &m_Samplers[samplerId];
}

SAttributesContainer	*PopcornFXAttributeList::AttributesContainer()
{
	SAttributesContainer	*attributesContainer = SAttributesContainer::Create(AttributeCount(), SamplerCount());

	if (PK_VERIFY(attributesContainer != null))
	{
		const u32	attributeBytes = attributesContainer->Attributes().CoveredBytes();
		if (PK_VERIFY(attributeBytes == m_AttributesRawData.size()))
		{
			Mem::Copy(attributesContainer->Attributes().Data(), m_AttributesRawData.data(), attributeBytes);
		}
	}
	return attributesContainer;
}

void	PopcornFXAttributeList::Clear()
{
	m_Attributes.clear();
	m_AttributesRawData.clear();
	for (size_t i = 0; i < m_Samplers.size(); ++i)
	{
		if (m_Samplers[i].m_EntityId != (AZ::EntityId)AZ::EntityId::InvalidEntityId &&
			PopcornFXSamplerComponentEventsBus::MultiHandler::BusIsConnectedId(m_Samplers[i].m_EntityId))
		{
			PopcornFXSamplerComponentEventsBus::MultiHandler::BusDisconnect(m_Samplers[i].m_EntityId);
		}
	}
	m_Samplers.clear();
}

#endif //O3DE_USE_PK

}
