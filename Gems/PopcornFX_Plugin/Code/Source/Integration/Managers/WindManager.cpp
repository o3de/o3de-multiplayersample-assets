//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "WindManager.h"

#if defined(O3DE_USE_PK)

#if defined(PK_USE_PHYSX)
	#include <AzCore/Interface/Interface.h>
	#include <LmbrCentral/Scripting/TagComponentBus.h>
	#include <PhysX/ColliderShapeBus.h>
	#include <PhysX/ForceRegionComponentBus.h>
#endif

#include <pk_compiler/include/cp_binders.h>
#include <pk_compiler/include/cp_backend.h>
#include <pk_compiler/include/cp_ir_ranges.h>
#include <pk_particles/include/ps_compiler_metadata.h>
#include <pk_particles/include/ps_bounds.h>
#include <pk_particles/include/ps_simulation_interface.h>
#include <pk_particles/include/Updaters/CPU/updater_cpu.h>
#include <pk_kernel/include/kr_memoryviews_utils.h>
#include <pk_maths/include/pk_maths_simd_vector.h>
#include <pk_maths/include/pk_maths_simd_quaternion.h>
#include <pk_maths/include/pk_maths_simd_matrix.h>

#include "Integration/PopcornFXUtils.h"

namespace PopcornFX {
//----------------------------------------------------------------------------

#if defined(PK_USE_PHYSX)

void	_SampleWindField(	const TStridedMemoryView<CFloat3>		&dstWind,
							const TStridedMemoryView<const CFloat3>	&srcLocation)
{
	CWindManager	*windManager = checked_cast<CWindManager*>(CWindManagerBase::Get());
	if (!PK_VERIFY(windManager != null))
	{
		Mem::ClearStream(dstWind);
		return;
	}
	windManager->SampleWindField(dstWind, srcLocation);
}

CWindManagerBase	*CWindManagerBase::s_Self = null;

CWindManagerBase::CWindManagerBase()
{
	PK_ASSERT(s_Self == null);
	s_Self = this;
}

//----------------------------------------------------------------------------

#if	(PK_COMPILER_BUILD_COMPILER != 0)
static bool		_CustomRangeBuild_SceneSampleWindField(	const CCompilerIR						*ir,
														Compiler::IR::CRangeAnalysis			*ranges,
														const Compiler::IR::SOptimizerConfig	&optimizerConfig,
														const Compiler::IR::SOp_Generic			&op,
														PopcornFX::Range::SConstantRange		&outputRange)
{
	AZ_UNUSED(ir);
	AZ_UNUSED(ranges);
	AZ_UNUSED(optimizerConfig);
	if (!PK_VERIFY(op.m_Opcode == Compiler::IR::Opcode_FunctionCall) ||
		!PK_VERIFY(op.m_Inputs.Count() == 2))	// SI_SceneSampleWindField(float3 location, pCtxS)
		return false;

	// Set an exclusive inf range. Otherwise, it will default to inclusive inf,
	// and will cause VRP in the physics node to evaluate 'inf*0 = inf' (NaNs are not considered valid in VRP),
	// and velocities with no wind influence will be incorrectly seen as bumped to an infinite range even
	// if the input velocity has a finite range
	outputRange = PopcornFX::Range::SConstantRange::kF32_Finite;	// ]-inf,+inf[
	return true;
}
#endif

//----------------------------------------------------------------------------

static bool	_BuildSimInterfaceDef_SceneSampleWindField(SSimulationInterfaceDefinition &def)
{
	PK_ASSERT(def.m_Inputs.Empty() && def.m_Outputs.Empty());

	// Build the sim interface definition
	def.m_FnNameBase = "SceneSampleWindField";
	def.m_Traits = Compiler::F_StreamedReturnValue;

#if	(PK_COMPILER_BUILD_COMPILER != 0)
	const Compiler::STypeMetaData	kMetaData_Location = u32(MetaData_XForm_World | MetaData_XForm_Full);
	const Compiler::STypeMetaData	kMetaData_Wind = u32(MetaData_XForm_World | MetaData_XForm_Direction);
	def.m_OptimizerDefs.m_FnRangeBuildPtr = &_CustomRangeBuild_SceneSampleWindField;
#else
	const Compiler::STypeMetaData	kMetaData_Location = 0;
	const Compiler::STypeMetaData	kMetaData_Wind = 0;
#endif

	// Declare all inputs
	if (!def.m_Inputs.PushBack(SSimulationInterfaceDefinition::SValueIn("Location", Nodegraph::DataType_Float3, Compiler::Attribute_Stream, kMetaData_Location)).Valid())
		return false;

	// Declare all outputs
	if (!def.m_Outputs.PushBack(SSimulationInterfaceDefinition::SValueOut("Wind", Nodegraph::DataType_Float3, Compiler::Attribute_Stream, kMetaData_Wind)).Valid())
		return false;

	return true;
}

//----------------------------------------------------------------------------

bool	CWindManagerBase::Reset(const AZStd::string &libraryPath)
{
	if (!m_SceneWindSimInterfacePath.Empty())
		_UnbindSceneSimInterface();

	//TODO: remove this when the sim interface lookup will be case-insensitive
	//Find the template path
	//We cannot test with Exists because it is case-insensitive
	//For now check if the library path has case and choose the template based on this
	CString	pkLibraryPath = libraryPath.c_str();

	if (pkLibraryPath.Contains("library"))
	{
		m_SceneWindSimInterfacePath = (libraryPath + "/popcornfxcore/templates/core.pkfx").c_str();
	}
	else if (pkLibraryPath.Contains("Library"))
	{
		m_SceneWindSimInterfacePath = (libraryPath + "/PopcornFXCore/Templates/Core.pkfx").c_str();
	}

	if (m_SceneWindSimInterfacePath.Empty())
	{
		//Nothing to bind
		return true;
	}

	if (!_BindSceneSimInterface())
		return false;

	return true;
}

//----------------------------------------------------------------------------

bool	CWindManagerBase::_BindSceneSimInterface() const
{
	// Build the sim interface definition
	SSimulationInterfaceDefinition	def;
	if (!PK_VERIFY(_BuildSimInterfaceDef_SceneSampleWindField(def)))
		return false;

#if	(PK_COMPILER_BUILD_COMPILER != 0)
	// Bind wind simultation interface
	CSimulationInterfaceMapper	*simInterfaceMapper = CSimulationInterfaceMapper::DefaultMapper();
	if (!simInterfaceMapper->Bind(m_SceneWindSimInterfacePath, L"SI_SceneSampleWindField", def))
	{
		AZ_Error("PopcornFX", false, "Failed binding sim interface \"%s\"", def.m_FnNameBase.Data());
		return false;
	}
#endif

	// Bind to CPU linker
	Compiler::SBinding	linkBinding;
	Compiler::Binders::Bind(linkBinding, &_SampleWindField);

	const PopcornFX::CString	mangledCall0 = def.GetCallNameMangledCPU(0);

	PK_ASSERT(CLinkerCPU::GetBinding(CStringView(), CStringView(mangledCall0)).Empty());

	if (!CLinkerCPU::Bind(CStringView(), CStringView(mangledCall0), linkBinding))
	{
		AZ_Error("PopcornFX", false, "Failed linking sim interface \"%s\"", def.m_FnNameBase.Data());
		return false;
	}

	return true;
}

void	CWindManagerBase::_UnbindSceneSimInterface()
{
#if	(PK_COMPILER_BUILD_COMPILER != 0)
	CSimulationInterfaceMapper	*simInterfaceMapper = CSimulationInterfaceMapper::DefaultMapper();
	const CStringUnicode		simInterfaceName = L"SI_SceneSampleWindField";

	// If sim interface is not there, abort
	if (simInterfaceMapper->Map(m_SceneWindSimInterfacePath, simInterfaceName) == null)
		return;

	// Unbind the sim interface.
	// All subsequent effect compilations will not see it as a sim interface
	// and will compile like in the editor: using the default implementation contained
	// in the sim interface template node.
	simInterfaceMapper->Unbind(m_SceneWindSimInterfacePath, simInterfaceName);
#endif

	// Build the sim interface definition
	SSimulationInterfaceDefinition	def;
	if (!PK_VERIFY(_BuildSimInterfaceDef_SceneSampleWindField(def)))
		return;

	// Unbind from CPU linker
	CLinkerCPU::Unbind(def.GetCallNameMangledCPU(0));

	m_SceneWindSimInterfacePath = "";
}

#endif // defined(PK_USE_PHYSX)

//----------------------------------------------------------------------------

#if defined(PK_USE_PHYSX)

void	CWindManager::Activate()
{
	Physics::WindNotificationsBus::Handler::BusConnect();
}

void	CWindManager::Deactivate()
{
	Physics::WindNotificationsBus::Handler::BusDisconnect();
}

void	CWindManager::Update()
{
	PK_SCOPEDPROFILE();
	PK_SCOPEDLOCK_WRITE(m_WindHelper.m_Lock);

	if (m_WindHelper.m_WindChanged)
	{
		m_WindHelper.m_Areas.Clear();

		AZ::EBusAggregateResults<AZ::EntityId> resultSet;
		LmbrCentral::TagGlobalRequestBus::EventResult(resultSet, LmbrCentral::Tag("wind"), &LmbrCentral::TagGlobalRequests::RequestTaggedEntities);

		m_WindHelper.m_Areas.Resize(static_cast<u32>(resultSet.values.size()));

		u32	areaIndex = 0;
		for (const AZ::EntityId &entityId : resultSet.values)
		{
			AZ::Aabb	forceAabb;
			PhysX::ColliderShapeRequestBus::EventResult(forceAabb, entityId, &PhysX::ColliderShapeRequestBus::Events::GetColliderShapeAabb);
			m_WindHelper.m_Areas[areaIndex].m_WorldBB = ToPk(forceAabb);

			m_WindHelper.m_Areas[areaIndex].m_WindAreaInvTransform = CFloat4x4::IDENTITY;
			m_WindHelper.m_Areas[areaIndex].m_WindAreaInvTransform.Scale(CFloat4(m_WindHelper.m_Areas[areaIndex].m_WorldBB.Extent() * 0.5f, 1.0f));
			m_WindHelper.m_Areas[areaIndex].m_WindAreaInvTransform.StrippedTranslations() = m_WindHelper.m_Areas[areaIndex].m_WorldBB.Center();
			m_WindHelper.m_Areas[areaIndex].m_WindAreaInvTransform.Invert();

			AZ::Vector3	direction = AZ::Vector3::CreateZero();
			PhysX::ForceWorldSpaceRequestBus::EventResult(direction, entityId, &PhysX::ForceWorldSpaceRequestBus::Events::GetDirection);

			float	magnitude = 0.0f;
			PhysX::ForceWorldSpaceRequestBus::EventResult(magnitude, entityId, &PhysX::ForceWorldSpaceRequestBus::Events::GetMagnitude);

			if (!direction.IsZero())
				m_WindHelper.m_Areas[areaIndex].m_Wind = ToPk(direction.GetNormalized() * magnitude);
			else
				m_WindHelper.m_Areas[areaIndex].m_Wind = CFloat3::ZERO;
			areaIndex++;
		}

		m_WindHelper.m_WindChanged = false;
	}
}

void	CWindManager::SampleWindField(	const TStridedMemoryView<CFloat3>		&dstWind,
										const TStridedMemoryView<const CFloat3>	&srcLocation)
{
	PK_SCOPEDPROFILE();

	PK_SCOPEDLOCK_READ(m_WindHelper.m_Lock);

	PK_ASSERT(dstWind.Contiguous());
	PK_ASSERT(srcLocation.Contiguous());

	Mem::Fill96(dstWind.Data(), &m_WindHelper.m_GlobalWind, dstWind.Count());

	const CAABB	pageBB = ComputeBounds_Point(srcLocation, TStridedMemoryView<const u8>());

	for (u32 j = 0; j < m_WindHelper.m_Areas.Count(); j++)
	{
		const PopcornFX::CAABB				windWorldBox = m_WindHelper.m_Areas[j].m_WorldBB;

		if (pageBB.Overlaps(windWorldBox))
		{
			const CFloat3					&wind = m_WindHelper.m_Areas[j].m_Wind;
			PK_ALIGN(0x10) const CFloat4x4	areaWindInvMat = m_WindHelper.m_Areas[j].m_WindAreaInvTransform;

			CFloat3							*dstPos = dstWind.Data();
			const CFloat3					*srcPos = srcLocation.Data();
			const CFloat3					*srcEndPos = srcLocation.DataEnd();

			// SIMD data:
			const SIMD::Float4				simdWind = SIMD::Float4(wind);
			const SIMD::Float4				simdWind0 = simdWind.Swizzle<0, 1, 2, 0>();
			const SIMD::Float4				simdWind1 = simdWind.Swizzle<1, 2, 0, 1>();
			const SIMD::Float4				simdWind2 = simdWind.Swizzle<2, 0, 1, 2>();
			const SIMD::Float4x4			simdAreaWindInvMat = SIMD::Float4x4::LoadAligned16(&areaWindInvMat);

			srcEndPos -= 4;
			while (srcPos <= srcEndPos)
			{
				// Get the positions and unpack:
				SIMD::Float4		src_x0x1x2x3 = SIMD::Float4::LoadAligned16(srcPos, 0x00);
				SIMD::Float4		src_y0y1y2y3 = SIMD::Float4::LoadAligned16(srcPos, 0x10);
				SIMD::Float4		src_z0z1z2z3 = SIMD::Float4::LoadAligned16(srcPos, 0x20);
				SIMD::Transpose4x3_Unpack(src_x0x1x2x3, src_y0y1y2y3, src_z0z1z2z3);

				// Transform by the inv wind area matrix:
				simdAreaWindInvMat.TransformVectors_OneExtend(src_x0x1x2x3, src_y0y1y2y3, src_z0z1z2z3);

				// Check if is inside the wind area:
				const SIMD::Float4	isInsideX = src_x0x1x2x3.MaskLower(SIMD::Float4::One()) & src_x0x1x2x3.MaskGreater(SIMD::Float4::NegOne());
				const SIMD::Float4	isInsideY = src_y0y1y2y3.MaskLower(SIMD::Float4::One()) & src_y0y1y2y3.MaskGreater(SIMD::Float4::NegOne());
				const SIMD::Float4	isInsideZ = src_z0z1z2z3.MaskLower(SIMD::Float4::One()) & src_z0z1z2z3.MaskGreater(SIMD::Float4::NegOne());
				const SIMD::Float4	isInside = (isInsideX & isInsideY) & isInsideZ;

				// Add the wind if it's inside:
				const SIMD::Float4	outWind0 = simdWind0 & isInside.Swizzle<0, 0, 0, 1>();
				const SIMD::Float4	outDst0 = outWind0 + SIMD::Float4::LoadAligned16(dstPos, 0x00);
				const SIMD::Float4	outWind1 = simdWind1 & isInside.Swizzle<1, 1, 2, 2>();
				const SIMD::Float4	outDst1 = outWind1 + SIMD::Float4::LoadAligned16(dstPos, 0x10);
				const SIMD::Float4	outWind2 = simdWind2 & isInside.Swizzle<2, 3, 3, 3>();
				const SIMD::Float4	outDst2 = outWind2 + SIMD::Float4::LoadAligned16(dstPos, 0x20);

				// Store data:
				outDst0.StoreAligned16(dstPos, 0x00);
				outDst1.StoreAligned16(dstPos, 0x10);
				outDst2.StoreAligned16(dstPos, 0x20);

				dstPos += 4;
				srcPos += 4;
			}
			srcEndPos += 4;

			while (srcPos < srcEndPos)
			{
				const SIMD::Float4		pos = SIMD::Float4::LoadFloat3(srcPos);
				SIMD::Float4			localPos = simdAreaWindInvMat.TransformVector_OneExtend(pos);

				localPos.SetW(0.0f);
				if (SIMD::All(	localPos.MaskGreater(SIMD::Float4::NegOne()) &
								localPos.MaskLower(SIMD::Float4::One())))
				{
					*dstPos += wind;
				}

				dstPos += 1;
				srcPos += 1;
			}
		}
	}
}

void	CWindManager::OnGlobalWindChanged()
{
	PK_SCOPEDLOCK_WRITE(m_WindHelper.m_Lock);

	const Physics::WindRequests	*windRequests = AZ::Interface<Physics::WindRequests>::Get();
	if (windRequests)
	{
		m_WindHelper.m_GlobalWind = ToPk(windRequests->GetGlobalWind());
	}
}

void	CWindManager::OnWindChanged([[maybe_unused]] const AZ::Aabb &aabb)
{
	PK_SCOPEDLOCK_WRITE(m_WindHelper.m_Lock);

	m_WindHelper.m_WindChanged = true;
}

//----------------------------------------------------------------------------

#endif //defined(PK_USE_PHYSX)

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
