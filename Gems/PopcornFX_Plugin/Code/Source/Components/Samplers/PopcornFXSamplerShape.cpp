//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXSamplerShape.h"

#if defined(PK_USE_EMOTIONFX)
#include <Source/Integration/Rendering/RenderActor.h>
#include <EMotionFX/Source/ActorInstance.h>
#endif //PK_USE_EMOTIONFX

#include <AzCore/Math/Matrix4x4.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/ComponentApplicationBus.h>

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_classes.h>
#include <pk_geometrics/include/ge_matrix_tools.h>
#include <pk_geometrics/include/ge_mesh_kdtree.h>
#include <pk_geometrics/include/ge_mesh_sampler_accel.h>
#include <pk_kernel/include/kr_units.h>
#endif //O3DE_USE_PK

namespace PopcornFX {

	void	PopcornFXSamplerShape::Reflect(AZ::ReflectContext *context)
	{
		AZ::SerializeContext	*serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
		if (serializeContext)
		{
			serializeContext->Class<PopcornFXSamplerShape>()
				->Version(1, &VersionConverter)
				->Field("UseRelativeTransform", &PopcornFXSamplerShape::m_UseRelativeTransform)
				->Field("ShapeType", &PopcornFXSamplerShape::m_ShapeType)
				->Field("ValueX", &PopcornFXSamplerShape::m_ValueX)
				->Field("ValueY", &PopcornFXSamplerShape::m_ValueY)
				->Field("ValueZ", &PopcornFXSamplerShape::m_ValueZ)
				->Field("LodIndex", &PopcornFXSamplerShape::m_LodIndex)
				->Field("SubMeshIndex", &PopcornFXSamplerShape::m_SubMeshIndex)
				->Field("SampleNormals", &PopcornFXSamplerShape::m_SampleNormals)
				->Field("SampleTangents", &PopcornFXSamplerShape::m_SampleTangents)
				->Field("SampleUVs", &PopcornFXSamplerShape::m_SampleUVs)
				->Field("SampleColors", &PopcornFXSamplerShape::m_SampleColors)
				->Field("SampleVelocities", &PopcornFXSamplerShape::m_SampleVelocities)
				->Field("Info_NeedBuildKdTree", &PopcornFXSamplerShape::m_NeedBuildKdTree)
				->Field("Info_NeedBuildSamplingInfo", &PopcornFXSamplerShape::m_NeedBuildSamplingInfo)
				;

			// edit context:
			if (AZ::EditContext *editContext = serializeContext->GetEditContext())
			{
				editContext->Class<PopcornFXSamplerShape>("PopcornFX Sampler Shape", "")
					->DataElement(0, &PopcornFXSamplerShape::m_UseRelativeTransform, "UseRelativeTransform", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnUseRelativeTransformChanged)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &PopcornFXSamplerShape::m_ShapeType, "ShapeType", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnShapeTypeChanged)
						->EnumAttribute(PopcornFXSamplerShape::EShapeType::BoxShape, "Box")
						->EnumAttribute(PopcornFXSamplerShape::EShapeType::SphereShape, "Sphere")
						->EnumAttribute(PopcornFXSamplerShape::EShapeType::CylinderShape, "Cylinder")
						->EnumAttribute(PopcornFXSamplerShape::EShapeType::CapsuleShape, "Capsule")
						->EnumAttribute(PopcornFXSamplerShape::EShapeType::MeshShape, "Mesh")
						->EnumAttribute(PopcornFXSamplerShape::EShapeType::SkinnedMeshShape, "SkinnedMesh")
					->DataElement(0, &PopcornFXSamplerShape::m_ValueX, "", "")
						->Attribute(AZ::Edit::Attributes::NameLabelOverride, &PopcornFXSamplerShape::m_ValueXName)
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnShapeDimensionsChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsShape)
					->DataElement(0, &PopcornFXSamplerShape::m_ValueY, "", "")
						->Attribute(AZ::Edit::Attributes::NameLabelOverride, &PopcornFXSamplerShape::m_ValueYName)
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnShapeDimensionsChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsShape)
					->DataElement(0, &PopcornFXSamplerShape::m_ValueZ, "", "")
						->Attribute(AZ::Edit::Attributes::NameLabelOverride, &PopcornFXSamplerShape::m_ValueZName)
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnShapeDimensionsChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_ShowValueZ)
					//->DataElement(0, &PopcornFXSamplerShape::m_LodIndex, "Lod Index", "")
					//	->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
					//	->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(0, &PopcornFXSamplerShape::m_SubMeshIndex, "SubMesh Index", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(0, &PopcornFXSamplerShape::m_SampleNormals, "Sample Normals", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(0, &PopcornFXSamplerShape::m_SampleTangents, "Sample Tangents", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(0, &PopcornFXSamplerShape::m_SampleUVs, "Sample UVs", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(0, &PopcornFXSamplerShape::m_SampleColors, "Sample Colors", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(AZ::Edit::UIHandlers::CheckBox, &PopcornFXSamplerShape::m_NeedBuildKdTree, "Build KD Tree", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_TriggerRebuildKdTree)
					->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(AZ::Edit::UIHandlers::CheckBox, &PopcornFXSamplerShape::m_NeedBuildSamplingInfo, "Build Sampling Info", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_TriggerRebuildSamplingInfo)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsMesh)
					->DataElement(0, &PopcornFXSamplerShape::m_SampleVelocities, "Sample Velocities", "")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &PopcornFXSamplerShape::_OnValueChanged)
						->Attribute(AZ::Edit::Attributes::Visibility, &PopcornFXSamplerShape::m_IsSkinnedMesh)
					;
			}
		}
	}

	// Private Static
	bool	PopcornFXSamplerShape::VersionConverter(AZ::SerializeContext &context,
													AZ::SerializeContext::DataElementNode &classElement)
	{
		(void)context; (void)classElement;
		return true;
	}

	void	PopcornFXSamplerShape::CopyFrom(const PopcornFXSamplerShape &other)
	{
		m_UseRelativeTransform = other.m_UseRelativeTransform;
		m_ShapeType = other.m_ShapeType;
		m_ValueX = other.m_ValueX;
		m_ValueY = other.m_ValueY;
		m_ValueZ = other.m_ValueZ;
		m_ValueXName = other.m_ValueXName;
		m_ValueYName = other.m_ValueYName;
		m_ValueZName = other.m_ValueZName;
		m_LodIndex = other.m_LodIndex;
		m_SubMeshIndex = other.m_SubMeshIndex;
		m_SampleNormals = other.m_SampleNormals;
		m_SampleTangents = other.m_SampleTangents;
		m_SampleUVs = other.m_SampleUVs;
		m_SampleColors = other.m_SampleColors;
		m_NeedBuildKdTree = other.m_NeedBuildKdTree;
		m_NeedBuildSamplingInfo = other.m_NeedBuildSamplingInfo;
		m_SampleVelocities = other.m_SampleVelocities;
	}

#if !defined(O3DE_USE_PK)

	void	PopcornFXSamplerShape::Activate()
	{
	}

	void	PopcornFXSamplerShape::Deactivate()
	{
	}

	AZ::u32	PopcornFXSamplerShape::_OnShapeTypeChanged()
	{
		return AZ::Edit::PropertyRefreshLevels::None;
	}

	AZ::u32	PopcornFXSamplerShape::_OnShapeDimensionsChanged()
	{
		return AZ::Edit::PropertyRefreshLevels::None;
	}

	void	PopcornFXSamplerShape::_OnUseRelativeTransformChanged()
	{
	}

	void PopcornFXSamplerShape::_TriggerRebuildKdTree()
	{
	}

	void PopcornFXSamplerShape::_TriggerRebuildSamplingInfo()
	{
	}

#else

	void	PopcornFXSamplerShape::Activate()
	{
		AZ::TickBus::Handler::BusConnect();
		AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(m_AttachedToEntityId);
		AZ::TransformNotificationBus::Handler::BusConnect(m_AttachedToEntityId);
#if defined(PK_USE_EMOTIONFX)
		EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(m_AttachedToEntityId);
#endif
		PopcornFXSamplerComponentRequestBus::Handler::BusConnect(m_AttachedToEntityId);
		_Init();
	}

	void	PopcornFXSamplerShape::Deactivate()
	{
		if (PopcornFXSamplerComponentRequestBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
			PopcornFXSamplerComponentRequestBus::Handler::BusDisconnect(m_AttachedToEntityId);
#if defined(PK_USE_EMOTIONFX)
		EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
#endif
		if (AZ::TransformNotificationBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
			AZ::TransformNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
		if (AZ::Render::MeshComponentNotificationBus::Handler::BusIsConnectedId(m_AttachedToEntityId))
			AZ::Render::MeshComponentNotificationBus::Handler::BusDisconnect(m_AttachedToEntityId);
		AZ::TickBus::Handler::BusDisconnect();
		_CleanShapeDescriptor();
		_CleanMeshAssets();
#if defined(PK_USE_EMOTIONFX)
		m_ActorInstance = null;
#endif
		m_SamplerDescriptor = null;
	}

	AZ::u32	PopcornFXSamplerShape::_OnShapeTypeChanged()
	{
		m_ValueX = 1.0f;
		m_ValueY = 1.0f;
		m_ValueZ = 1.0f;
		m_SubMeshIndex = 0;
		_Init();
		return AZ::Edit::PropertyRefreshLevels::EntireTree;
	}

	AZ::u32	PopcornFXSamplerShape::_OnValueChanged()
	{
		_Init();
		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::u32	PopcornFXSamplerShape::_OnShapeDimensionsChanged()
	{
		_CleanShapeDescriptor();
		_SetTransform();
		_LoadShape();
		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void	PopcornFXSamplerShape::_OnUseRelativeTransformChanged()
	{
		_SetTransform();
	}

	void PopcornFXSamplerShape::_TriggerRebuildKdTree()
	{
		if (m_MeshNew != null)
		{
			_RebuildKdTree(m_MeshNew);
			_SetMeshIFN();
		}
	}

	void PopcornFXSamplerShape::_TriggerRebuildSamplingInfo()
	{
		if (m_MeshNew != null)
		{
			_RebuildSamplingInfo(m_MeshNew);
			_SetMeshIFN();
		}
	}

	void	PopcornFXSamplerShape::_Init()
	{
		_CleanShapeDescriptor();
		m_IsMesh = m_ShapeType == EShapeType::MeshShape || m_ShapeType == EShapeType::SkinnedMeshShape;
		m_IsSkinnedMesh = m_ShapeType == EShapeType::SkinnedMeshShape;
		m_IsShape = !m_IsMesh;
		m_ShowValueZ = m_IsShape;
		_SetTransform();
		if (m_IsShape)
		{
			_SetValuesNames();
			_LoadShape();
		}
		else
			_LoadMesh();
	}

	void	PopcornFXSamplerShape::OnTransformChanged(const AZ::Transform &/*local*/, const AZ::Transform &/*world*/)
	{
		_SetTransform();
	}

	void PopcornFXSamplerShape::_RebuildKdTree(const PMeshNew &mesh)
	{
		if (mesh == null)
			return;
		if (m_NeedBuildKdTree)
		{
			SMeshKdTreeBuildConfig	buildConfig; // the default config is fine
#if 0
			buildConfig.m_Flags |= SMeshKdTreeBuildConfig::LowQualityButFasterBuild;
#endif
			if (!mesh->BuildKdTree(buildConfig, true))
			{
				AZ_Warning("PopcornFX", false, "Build KdTree Failed!");
			}
		}
		else
			//clear KdTree
			mesh->SetKdTree_GiveOwnership(null);
	}

	void PopcornFXSamplerShape::_RebuildSamplingInfo(const PMeshNew &mesh)
	{
		if (m_NeedBuildSamplingInfo && mesh != null)
		{
#if	(PK_GEOMETRICS_BUILD_MESH_SAMPLER_SURFACE != 0)
			mesh->SetupDefaultSurfaceSamplingAccelStructsIFN(false);
#endif
#if	(PK_GEOMETRICS_BUILD_MESH_SAMPLER_VOLUME != 0)
			mesh->SetupDefaultVolumeSamplingAccelStructsIFN(false);
#endif
#if	(PK_GEOMETRICS_BUILD_MESH_SAMPLER_SURFACE != 0)
			mesh->SetupDefaultSurfaceSamplingAccelStructsUVIFN(SMeshUV2PCBuildConfig(), false);
#endif
		}
	}

	void	PopcornFXSamplerShape::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		AZ_UNUSED(time);
		AZ_UNUSED(deltaTime);

		m_PreviousMat = m_CurrentMat;
		if (m_SkinnedMesh != null)
		{
#if defined(PK_USE_EMOTIONFX)
			if (m_ActorInstance)
				m_SkinnedMesh->UpdateSkinning(m_ActorInstance, deltaTime);
#endif
		}
	}

	void	PopcornFXSamplerShape::_SetTransform()
	{
		// Get entity's world transforms
		AZ::Transform parentTransform = AZ::Transform::CreateIdentity();
		if (m_UseRelativeTransform)
		{
			EBUS_EVENT_ID_RESULT(parentTransform, m_AttachedToEntityId, AZ::TransformBus, GetLocalTM);
		}
		else
		{
			EBUS_EVENT_ID_RESULT(parentTransform, m_AttachedToEntityId, AZ::TransformBus, GetWorldTM);
		}
		// Setup effect transforms
		AZ::Matrix4x4 tr = AZ::Matrix4x4::CreateFromTransform(parentTransform);
		Mem::Copy(&m_CurrentMat, &tr, sizeof(CFloat4x4));
		m_CurrentMat.Transpose();
		m_CurrentMat.XAxis().w() = 0.0f;
		m_CurrentMat.YAxis().w() = 0.0f;
		m_CurrentMat.ZAxis().w() = 0.0f;
		m_CurrentMat.WAxis().w() = 1.0f;
		m_PreviousMat = m_CurrentMat;
	}

	//----------------------------------------------------------------------------

	typedef CShapeDescriptor*(*CbNewShapeDesc)(float x, float y, float z);

	template <typename _ShapeType>
	CShapeDescriptor	*_NewShapeDesc(float x, float y, float z)
	{
		PK_ASSERT_NOT_REACHED();
		return null;
	}

	template<>
	CShapeDescriptor	*_NewShapeDesc<CShapeDescriptor_Box>(float x, float y, float z)
	{
		return PK_NEW(CShapeDescriptor_Box(CFloat3(x, y, z)));
	}

	template<>
	CShapeDescriptor	*_NewShapeDesc<CShapeDescriptor_Sphere>(float x, float y, float /*z*/)
	{
		return PK_NEW(CShapeDescriptor_Sphere(x, y));
	}

	template<>
	CShapeDescriptor	*_NewShapeDesc<CShapeDescriptor_Cylinder>(float x, float y, float z)
	{
		return PK_NEW(CShapeDescriptor_Cylinder(x, y, z));
	}

	template<>
	CShapeDescriptor	*_NewShapeDesc<CShapeDescriptor_Capsule>(float x, float y, float z)
	{
		return PK_NEW(CShapeDescriptor_Capsule(x, y, z));
	}

	template<>
	CShapeDescriptor	*_NewShapeDesc<CShapeDescriptor_Mesh>(float /*x*/, float /*y*/, float /*z*/)
	{
		return PK_NEW(CShapeDescriptor_Mesh());
	}

	//----------------------------------------------------------------------------

	CbNewShapeDesc const	kCbNewShapeDescriptors[] =
	{														// ESShapeType :
		&_NewShapeDesc<CShapeDescriptor_Box>,				// BoxShape = 0,
		&_NewShapeDesc<CShapeDescriptor_Sphere>,			// SphereShape,
		&_NewShapeDesc<CShapeDescriptor_Cylinder>,			// CylinderShape,
		&_NewShapeDesc<CShapeDescriptor_Capsule>,			// CapsuleShape,
		&_NewShapeDesc<CShapeDescriptor_Mesh>,				// Mesh,
		&_NewShapeDesc<CShapeDescriptor_Mesh>				// SkinnedMesh,
	};

	PK_STATIC_ASSERT(PK_ARRAY_COUNT(kCbNewShapeDescriptors) == (u32)PopcornFXSamplerShape::EShapeType::ShapeType_Max);

	bool	PopcornFXSamplerShape::_BuildDescriptor(const AZ::RPI::ModelLodAsset::Mesh *mesh /*= null*/)
	{
		if (m_SamplerDescriptor == null)
			m_SamplerDescriptor = PK_NEW(CParticleSamplerDescriptor_Shape_Default());

		PParticleSamplerDescriptor_Shape_Default	desc = reinterpret_cast<CParticleSamplerDescriptor_Shape_Default *>(m_SamplerDescriptor.Get());
		if (!PK_VERIFY(desc != null))
			return false;

		if (m_ShapeDescriptor == null)
			m_ShapeDescriptor = kCbNewShapeDescriptors[(u32)m_ShapeType](m_ValueX, m_ValueY, m_ValueZ);
		if (!PK_VERIFY(m_ShapeDescriptor != null))
			return false;
		desc->m_Shape = m_ShapeDescriptor;

		desc->m_WorldTr_Current = &m_CurrentMat;
		desc->m_WorldTr_Previous = &m_PreviousMat;

		if (mesh != null)
		{
			const AZ::RPI::BufferAssetView	&bufferIndices = mesh->GetIndexBufferAssetView();
			const AZ::RPI::BufferAssetView	*bufferPositions = mesh->GetSemanticBufferAssetView(AZ::Name{ "POSITION" });
			const AZ::RPI::BufferAssetView	*bufferNormals = mesh->GetSemanticBufferAssetView(AZ::Name{ "NORMAL" });
			const AZ::RPI::BufferAssetView	*bufferTangents = mesh->GetSemanticBufferAssetView(AZ::Name{ "TANGENT" });
			const AZ::RPI::BufferAssetView	*bufferTexcoords = mesh->GetSemanticBufferAssetView(AZ::Name{ "UV" });
			const AZ::RPI::BufferAssetView	*bufferColors = mesh->GetSemanticBufferAssetView(AZ::Name{ "COLOR" });

			TStaticCountedArray<SVStreamCode, 5>	streamCodes;
			if (bufferPositions != null)
			{
				streamCodes.PushBack(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Position,
					SVStreamCode::Element_Float3,
					SVStreamCode::SIMD_Friendly));
			}
			if (bufferNormals != null && m_SampleNormals)
			{
				streamCodes.PushBack(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Normal,
					SVStreamCode::Element_Float3,
					SVStreamCode::SIMD_Friendly));
			}
			if (bufferTangents != null && m_SampleTangents)
			{
				streamCodes.PushBack(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Tangent,
					SVStreamCode::Element_Float4,
					SVStreamCode::SIMD_Friendly));
			}
			if (bufferTexcoords != null && m_SampleUVs)
			{
				streamCodes.PushBack(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Texcoord,
					SVStreamCode::Element_Float2,
					SVStreamCode::SIMD_Friendly));
			}
			if (bufferColors != null && m_SampleColors)
			{
				streamCodes.PushBack(SVStreamCode(CVStreamSemanticDictionnary::Ordinal_Color,
					SVStreamCode::Element_Float4,
					SVStreamCode::SIMD_Friendly));
			}

			m_MeshNew = PK_NEW(CMeshNew);
			if (!PK_VERIFY(m_MeshNew != null))
				return false;
			CMeshTriangleBatch	&triBatch = m_MeshNew->TriangleBatch();
			CMeshVStream		&vstream = triBatch.m_VStream;
			CMeshIStream		&istream = triBatch.m_IStream;

			istream.SetPrimitiveType(CMeshIStream::Triangles);

			const u32	indexCount = mesh->GetIndexCount();
			const u32	vertexCount = mesh->GetVertexCount();

#if	defined(PK_ANDROID)
			if (!PK_VERIFY(istream.Reformat(CMeshIStream::U16Indices)) ||
#else
			if (!PK_VERIFY(istream.Reformat(CMeshIStream::U32Indices)) ||
#endif
				!PK_VERIFY(vstream.Reformat(streamCodes)) ||
				!PK_VERIFY(istream.Resize(indexCount)) ||
				!PK_VERIFY(vstream.Resize(vertexCount)))
			{
				return false;
			}
			u32	*dstIndices = istream.StreamForWriting<u32>();
			PK_ASSERT(dstIndices != null);
			PK_ASSERT(sizeof(*dstIndices) == bufferIndices.GetBufferViewDescriptor().m_elementSize);

			Mem::Copy(dstIndices, bufferIndices.GetBufferAsset()->GetBuffer().data(), indexCount * sizeof(*dstIndices));

			if (bufferPositions != null)
			{
				const TStridedMemoryView<CFloat3>	dstPositions = vstream.Positions();
				const CFloat3						*src = reinterpret_cast<const CFloat3*>(&bufferPositions->GetBufferAsset()->GetBuffer()[0]) + bufferPositions->GetBufferViewDescriptor().m_elementOffset;
				PK_ASSERT(dstPositions.ElementSizeInBytes() == bufferPositions->GetBufferViewDescriptor().m_elementSize);
				PK_ASSERT(!dstPositions.Empty());
				for (u32 i = 0; i < vertexCount; ++i)
					dstPositions[i] = src[i];
			}
			if (bufferNormals != null && m_SampleNormals)
			{
				const TStridedMemoryView<CFloat3>	dstNormals = vstream.Normals();
				const CFloat3						*src = reinterpret_cast<const CFloat3*>(&bufferNormals->GetBufferAsset()->GetBuffer()[0]) + bufferNormals->GetBufferViewDescriptor().m_elementOffset;
				PK_ASSERT(dstNormals.ElementSizeInBytes() == bufferNormals->GetBufferViewDescriptor().m_elementSize);
				PK_ASSERT(!dstNormals.Empty());
				for (u32 i = 0; i < vertexCount; ++i)
					dstNormals[i] = src[i];
			}
			if (bufferTangents != null && m_SampleTangents)
			{
				const TStridedMemoryView<CFloat4>	dstTangents = vstream.Tangents();
				const CFloat4						*src = reinterpret_cast<const CFloat4*>(&bufferTangents->GetBufferAsset()->GetBuffer()[0]) + bufferTangents->GetBufferViewDescriptor().m_elementOffset;
				PK_ASSERT(dstTangents.ElementSizeInBytes() == bufferTangents->GetBufferViewDescriptor().m_elementSize);
				PK_ASSERT(!dstTangents.Empty());
				for (u32 i = 0; i < vertexCount; ++i)
					dstTangents[i] = src[i];
			}
			if (bufferTexcoords != null && m_SampleUVs)
			{
				const TStridedMemoryView<CFloat2>	dstUVs = vstream.Texcoords();
				const CFloat2						*src = reinterpret_cast<const CFloat2*>(&bufferTexcoords->GetBufferAsset()->GetBuffer()[0]) + bufferTexcoords->GetBufferViewDescriptor().m_elementOffset;
				PK_ASSERT(dstUVs.ElementSizeInBytes() == bufferTexcoords->GetBufferViewDescriptor().m_elementSize);
				PK_ASSERT(!dstUVs.Empty());
				for (u32 i = 0; i < vertexCount; ++i)
					dstUVs[i] = src[i];
			}
			if (bufferColors != null && m_SampleColors)
			{
				const TStridedMemoryView<CFloat4>	dstColors = vstream.Colors();
				const CUbyte4						*src = reinterpret_cast<const CUbyte4*>(&bufferColors->GetBufferAsset()->GetBuffer()[0]) + bufferColors->GetBufferViewDescriptor().m_elementOffset;
				PK_ASSERT(dstColors.ElementSizeInBytes() == bufferColors->GetBufferViewDescriptor().m_elementSize);
				PK_ASSERT(!dstColors.Empty());
				for (u32 i = 0; i < vertexCount; ++i)
					dstColors[i] = src[i];
			}

			_RebuildKdTree(m_MeshNew);
			_RebuildSamplingInfo(m_MeshNew);

#if defined(PK_USE_EMOTIONFX)
			if (m_ShapeType == PopcornFX::PopcornFXSamplerShape::EShapeType::SkinnedMeshShape)
			{
				if (m_SkinnedMesh == null)
					m_SkinnedMesh = PK_NEW(CPopcornFXSkinnedMesh);
				if (!PK_VERIFY(m_SkinnedMesh != null))
					return false;
				u32	samplingChannels = 0;
				samplingChannels |= m_SampleNormals ? CPopcornFXSkinnedMesh::Channel_Normal : 0;
				samplingChannels |= m_SampleTangents ? CPopcornFXSkinnedMesh::Channel_Tangent : 0;
				samplingChannels |= m_SampleUVs ? CPopcornFXSkinnedMesh::Channel_UV : 0;
				samplingChannels |= m_SampleColors ? CPopcornFXSkinnedMesh::Channel_VertexColor : 0;
				samplingChannels |= m_SampleVelocities ? CPopcornFXSkinnedMesh::Channel_Velocity : 0;

				if (m_ActorInstance != null)
				{
					if (!m_SkinnedMesh->Init(m_ActorInstance->GetActor(), m_LodIndex, m_SubMeshIndex, mesh->GetVertexCount(), vstream, samplingChannels))
					{
						m_SkinnedMesh = null;
						return false;
					}
				}
			}
#endif
		}

		_SetMeshIFN();

		PopcornFXSamplerComponentEventsBus::Event(m_AttachedToEntityId, &PopcornFXSamplerComponentEventsBus::Events::OnSamplerReady, m_AttachedToEntityId);

		return true;
	}

	void	PopcornFXSamplerShape::_SetMeshIFN()
	{
		if (m_MeshNew != null)
		{
			CShapeDescriptor_Mesh	*meshDesc = checked_cast<CShapeDescriptor_Mesh*>(m_ShapeDescriptor.Get());
			if (!PK_VERIFY(meshDesc != null))
				return;
			if (m_SkinnedMesh != null)
				meshDesc->SetMesh(m_MeshNew, m_SkinnedMesh->Override());
			else
				meshDesc->SetMesh(m_MeshNew);
		}
	}

	void	PopcornFXSamplerShape::OnModelReady(const AZ::Data::Asset<AZ::RPI::ModelAsset> &modelAsset, const AZ::Data::Instance<AZ::RPI::Model> &model)
	{
		AZ_UNUSED(model);
		m_ModelAsset = modelAsset;
		if (m_IsMesh)
			_LoadMesh();
	}

#if defined(PK_USE_EMOTIONFX)
	void	PopcornFXSamplerShape::OnActorInstanceCreated(EMotionFX::ActorInstance *actorInstance)
	{
		if (actorInstance != null)
		{
			m_ActorInstance = actorInstance;
			if (m_ActorInstance != null && m_IsMesh)
			{
				m_ModelAsset = m_ActorInstance->GetActor()->GetMeshAsset();
				_LoadMesh();
			}
		}
	}

	void	PopcornFXSamplerShape::OnActorInstanceDestroyed(EMotionFX::ActorInstance *actorInstance)
	{
		if (m_ActorInstance == actorInstance)
		{
			m_ActorInstance = null;
			if (m_IsSkinnedMesh)
			{
				_CleanShapeDescriptor();
				_CleanMeshAssets();
			}
		}
	}

#endif

	void	PopcornFXSamplerShape::_LoadMesh()
	{
		if (!m_ModelAsset.IsReady())
			return;
		if (m_LodIndex >= m_ModelAsset->GetLodCount())
		{
			AZStd::string	entityName;
			AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationBus::Events::GetEntityName, m_AttachedToEntityId);
			AZ_Error("PopcornFX", m_LodIndex < m_ModelAsset->GetLodCount(), "Invalid Lod Index for sampler shape %s", entityName.c_str());
			return;
		}

		AZ::Data::Asset<AZ::RPI::ModelLodAsset> modelLodAsset = m_ModelAsset->GetLodAssets()[m_LodIndex];

		if (m_SubMeshIndex >= modelLodAsset->GetMeshes().size())
		{
			AZStd::string	entityName;
			AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationBus::Events::GetEntityName, m_AttachedToEntityId);
			AZ_Error("PopcornFX", false, "Invalid sub-mesh index for sampler shape %s", entityName.c_str());
			return;
		}

		// Each mesh vertex stream is packed into a single buffer for the whole lod. Get the first mesh, which can be used to retrieve the underlying buffer assets
		const AZ::RPI::ModelLodAsset::Mesh &mesh0 = modelLodAsset->GetMeshes()[m_SubMeshIndex];

		if (!PK_VERIFY(_BuildDescriptor(&mesh0)))
		{
			AZStd::string	entityName;
			AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationBus::Events::GetEntityName, m_AttachedToEntityId);
			AZ_Error("PopcornFX", false, "Build descriptor failed for sampler shape %s", entityName.c_str());
		}
	}

	void	PopcornFXSamplerShape::_LoadShape()
	{
		if (!PK_VERIFY(_BuildDescriptor()))
		{
			AZStd::string	entityName;
			AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationBus::Events::GetEntityName, m_AttachedToEntityId);
			AZ_Error("PopcornFX", false, "Build descriptor failed for sampler shape %s", entityName.c_str());
		}
	}

	void	PopcornFXSamplerShape::_SetValuesNames()
	{
		switch (m_ShapeType)
		{
		case PopcornFX::PopcornFXSamplerShape::EShapeType::BoxShape:
			m_ValueXName = "Size X";
			m_ValueYName = "Size X";
			m_ValueZName = "Size Y";
			break;
		case PopcornFX::PopcornFXSamplerShape::EShapeType::SphereShape:
			m_ValueXName = "Radius";
			m_ValueYName = "InnerRadius";
			m_ShowValueZ = false;
			break;
		case PopcornFX::PopcornFXSamplerShape::EShapeType::CylinderShape:
		case PopcornFX::PopcornFXSamplerShape::EShapeType::CapsuleShape:
			m_ValueXName = "Radius";
			m_ValueYName = "Height";
			m_ValueZName = "InnerRadius";
			break;
		case PopcornFX::PopcornFXSamplerShape::EShapeType::MeshShape:
		case PopcornFX::PopcornFXSamplerShape::EShapeType::SkinnedMeshShape:
		case PopcornFX::PopcornFXSamplerShape::EShapeType::ShapeType_Max:
			break;
		default:
			break;
		}
	}

	void	PopcornFXSamplerShape::_CleanShapeDescriptor()
	{
		if (m_SamplerDescriptor != null)
		{
			PParticleSamplerDescriptor_Shape_Default	desc = reinterpret_cast<CParticleSamplerDescriptor_Shape_Default *>(m_SamplerDescriptor.Get());
			if (desc != null)
				desc->m_Shape = null;
		}
		m_ShapeDescriptor = null;
		m_MeshNew = null;
		m_SkinnedMesh = null;
	}

	void	PopcornFXSamplerShape::_CleanMeshAssets()
	{
		m_ModelAsset.Release();
	}

#endif //O3DE_USE_PK

}
