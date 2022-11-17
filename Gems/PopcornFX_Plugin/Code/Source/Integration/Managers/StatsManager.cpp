//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"
#include "StatsManager.h"

#if defined(O3DE_USE_PK)

#include <PopcornFX/PopcornFXBus.h>

#if PK_O3DE_MAJOR_VERSION >= 2205
#include <LyShine/IDraw2d.h>
#else
#include <LyShine/Draw2d.h>
#endif

#include <AzCore/Console/IConsole.h>
#include <AzCore/Interface/Interface.h>
#include <Atom/RPI.Public/ViewportContext.h>

#include <pk_particles/include/ps_mediums.h>
#include <pk_particles/include/ps_descriptor.h>
#include <pk_particles/include/ps_effect.h>
#include <pk_kernel/include/kr_profiler_details.h>
#include <pk_kernel/include/kr_sort.h>
#include <pk_kernel/include/kr_units.h>
#include <pk_particles/include/Storage/MainMemory/storage_ram_allocator.h>
#include <pk_particles/include/Storage/MainMemory/storage_ram.h>

namespace PopcornFX {

AZ_CVAR(float, p_PopcornFXHUD_HideNodesBelowPercent, 5.0f,
		null,
		AZ::ConsoleFunctorFlags::DontReplicate,
		"Hide HUD nodes below percent.");

AZ_CVAR(AZ::u32, p_PopcornFXHUD_UpdateTimeFrameCount, 25,
		null,
		AZ::ConsoleFunctorFlags::DontReplicate,
		"Frame count to compute update time average.");

AZ_CVAR(AZ::Vector2, p_PopcornFXHUD_TopRightBorderPadding, AZ::Vector2(40.0f, 22.0f),
		null,
		AZ::ConsoleFunctorFlags::DontReplicate,
		"The top right border padding for the PopcornFX viewport debug display text");

AZ_CVAR(bool, p_PopcornFXHUD_ShowMemory, false,
		null,
		AZ::ConsoleFunctorFlags::DontReplicate,
		"Show/hide PopcornFX memory HUD.");

AZ_CVAR(bool, p_PopcornFXHUD_ShowProfiler, false,
		null,
		AZ::ConsoleFunctorFlags::DontReplicate,
		"Show/hide PopcornFX profiler HUD.");

//----------------------------------------------------------------------------

#if	(PK_PARTICLES_HAS_STATS != 0)
bool	operator==(const CStatsManager::SPopcornFXEffectTimings &other, const PopcornFX::CParticleEffect *effect)
{
	return effect == other.m_Effect;
}

namespace
{
	class	CEffectTimingsSorter
	{
	public:
		typedef	PopcornFX::TArray<CStatsManager::SPopcornFXEffectTimings>::Iterator	_TypeIt;
		PK_FORCEINLINE static bool	Less(const _TypeIt &it0, const _TypeIt &it1) { return it0->TotalTime() > it1->TotalTime(); }
		PK_FORCEINLINE static bool	Equal(const _TypeIt &it0, const _TypeIt &it1) { return it0->TotalTime() == it1->TotalTime(); }
	};
}
#endif // (PK_PARTICLES_HAS_STATS != 0)

#if	(KR_MEM_DEFAULT_ALLOCATOR_DEBUG != 0)
class	CMemStatNodeSorter
{
public:
	typedef	PopcornFX::TArray<PopcornFX::CMemStatNode>::Iterator	_TypeIt;
	PK_FORCEINLINE static bool	Less(const _TypeIt &it0, const _TypeIt &it1) { return it0->Footprint() > it1->Footprint(); }
	PK_FORCEINLINE static bool	Equal(const _TypeIt &it0, const _TypeIt &it1) { return it0->Footprint() == it1->Footprint(); }
};
#endif

//----------------------------------------------------------------------------

//template <typename _Pool>
//void	CStatsManager::SPoolStat::AccumPool(_Pool &pool)
//{
//	SBufferPool_Stats		stats;
//	pool.AccumStats(&stats);
//	m_Mem += stats.m_TotalBytes;
//	m_MemUsed += stats.m_SoftExactUsedBytes;
//	m_Alloc += stats.m_AllocatedBytes;
//	m_Release += stats.m_ReleasedBytes;
//}

#if	(KR_PROFILER_ENABLED != 0)

void	CStatsManager::SPoolStat::Accum(const SPoolStat &other)
{
	m_Mem += other.m_Mem;
	m_MemUsed += other.m_MemUsed;
	m_Alloc += other.m_Alloc;
	m_Release += other.m_Release;
}

void	CStatsManager::SPoolStat::ComputePerFrame(float invFrameCount)
{
	m_Mem *= invFrameCount;
	m_MemUsed *= invFrameCount;
	//m_Alloc += other.m_Alloc;
	//m_Release += other.m_Release;
}

#endif

//----------------------------------------------------------------------------

void CStatsManager::EnsureHaveTextures() const
{
	if (!m_whiteTex)
	{
		m_whiteTex = AZ::RPI::ImageSystemInterface::Get()->GetSystemImage(AZ::RPI::SystemImage::White);
		PK_ASSERT(m_whiteTex);
	}
}

void	CStatsManager::Activate()
{
	AZ::RPI::ViewportContextNotificationBus::Handler::BusConnect(AZ::RPI::ViewportContextRequests::Get()->GetDefaultViewportContextName());
	PopcornFXProfilerRequestBus::Handler::BusConnect();
#if	(KR_PROFILER_ENABLED != 0)
	const u32	memoryFrameCount = 16;
	PK_VERIFY(m_MemoryFrames.Resize(memoryFrameCount));
#endif
}

void	CStatsManager::Deactivate()
{
	PopcornFXProfilerRequestBus::Handler::BusDisconnect();
	AZ::RPI::ViewportContextNotificationBus::Handler::BusDisconnect();
	m_whiteTex = null;
}

void	CStatsManager::Reset(CParticleMediumCollection *mediumCollection)
{
#if	(PK_PARTICLES_HAS_STATS != 0)
	mediumCollection->Stats().Reset();
#endif //	(PK_PARTICLES_HAS_STATS != 0)

	// Add the OnUpdateComplete callback IFN:
	if (!mediumCollection->m_OnUpdateComplete.Contains(FastDelegate<void(CParticleMediumCollection*)>(this, &CStatsManager::_OnUpdateComplete)))
		mediumCollection->m_OnUpdateComplete += FastDelegate<void(CParticleMediumCollection*)>(this, &CStatsManager::_OnUpdateComplete);
	m_MainThreadTimer.Reset();
	m_UpdateSpanTimer.Reset();
	m_UpdateSpanMainThreadTimer.Reset();
	m_BillboardingSpanTimer.Reset();
}

void	CStatsManager::Update(CParticleMediumCollection *mediumCollection)
{
	if (p_PopcornFXHUD_ShowProfiler)
		UpdateProfiler(mediumCollection);
	if (p_PopcornFXHUD_ShowMemory)
		UpdateMemory(mediumCollection);
}

void CStatsManager::OnRenderTick()
{
	if (!m_fontDrawInterface)
	{
		auto fontQueryInterface = AZ::Interface<AzFramework::FontQueryInterface>::Get();
		if (!fontQueryInterface)
		{
			return;
		}
		m_fontDrawInterface = fontQueryInterface->GetDefaultFontDrawInterface();
	}
	AZ::RPI::ViewportContextPtr viewportContext = AZ::RPI::ViewportContextRequests::Get()->GetDefaultViewportContext();

	if (!m_fontDrawInterface || !viewportContext || !viewportContext->GetRenderScene())
	{
		return;
	}

	m_drawParams.m_drawViewportId = viewportContext->GetId();
	m_drawParams.m_position = AZ::Vector3(p_PopcornFXHUD_TopRightBorderPadding) * viewportContext->GetDpiScalingFactor();
	m_drawParams.m_color = AZ::Colors::White;
	m_drawParams.m_scale = AZ::Vector2(BaseFontSize * viewportContext->GetDpiScalingFactor());
	m_drawParams.m_hAlign = AzFramework::TextHorizontalAlignment::Left;
	m_drawParams.m_monospace = false;
	m_drawParams.m_depthTest = false;
	m_drawParams.m_virtual800x600ScreenSize = false;
	m_drawParams.m_scaleWithWindow = false;
	m_drawParams.m_multiline = true;
	m_drawParams.m_lineSpacing = 0.5f;

	// Calculate line spacing based on the font's actual line height
	const float	lineHeight = m_fontDrawInterface->GetTextSize(m_drawParams, " ").GetY();
	m_lineSpacing = lineHeight * m_drawParams.m_lineSpacing;

	if (p_PopcornFXHUD_ShowProfiler)
		DrawProfiler();
	if (p_PopcornFXHUD_ShowMemory)
		DrawMemory();
}

void	CStatsManager::StartMainThreadTimer()
{
	m_MainThreadTimer.Start();
}

void	CStatsManager::StopMainThreadTimer()
{
	m_MainThreadOverhead += static_cast<float>(m_MainThreadTimer.Stop());
}

void	CStatsManager::StartUpdateSpanTimer()
{
	m_UpdateSpanTimer.Start();
	m_UpdateSpanMainThreadTimer.Start();
	m_IsUpdateSpanMainThreadTimerRunning = true;
}

void	CStatsManager::StopUpdateSpanTimer()
{
	m_UpdateSpan += static_cast<float>(m_UpdateSpanTimer.Stop());
	PK_SCOPEDLOCK(m_LockUpdateSpanTimer)
	{
		if (m_IsUpdateSpanMainThreadTimerRunning)
		{
			m_UpdateSpanMainThread += static_cast<float>(m_UpdateSpanMainThreadTimer.Stop());
			m_IsUpdateSpanMainThreadTimerRunning = false;
		}
	}
}

void	CStatsManager::StopMainThreadUpdateSpanTimer()
{
	PK_SCOPEDLOCK(m_LockUpdateSpanTimer)
	{
		if (m_IsUpdateSpanMainThreadTimerRunning)
		{
			m_UpdateSpanMainThread += static_cast<float>(m_UpdateSpanMainThreadTimer.Stop());
			m_IsUpdateSpanMainThreadTimerRunning = false;
		}
	}
}

void	CStatsManager::StartBillboardingSpanTimer()
{
	m_BillboardingSpanTimer.Start();
}

void	CStatsManager::StopBillboardingSpanTimer()
{
	m_BillboardingSpan += static_cast<float>(m_BillboardingSpanTimer.Stop());
}

void	CStatsManager::RegisterBatch()
{
}

void	CStatsManager::UnregisterBatch()
{
}

////////////////////////////////////////////////////////////////////////
// PopcornFXProfilerRequestBus interface implementation
////////////////////////////////////////////////////////////////////////

bool	CStatsManager::WriteProfileReport(const AZStd::string &path) const
{
#if	(KR_PROFILER_ENABLED != 0)
	Profiler::CProfiler	*profiler = Profiler::MainEngineProfiler();
	if (profiler == null)
		return false;
	profiler->Reset();
	profiler->BuildReport();
	return Profiler::WriteProfileReport(profiler->LatestReport(), File::DefaultFileSystem(), CString(path.c_str()));
#else
	return false;
#endif	// (KR_PROFILER_ENABLED != 0)
}

bool	CStatsManager::ProfilerSetEnable(bool enable) const
{
#if	(KR_PROFILER_ENABLED != 0)
	Profiler::CProfiler	*profiler = Profiler::MainEngineProfiler();
	if (profiler == null)
		return false;
	profiler->GrabCallstacks(false);
	profiler->Activate(enable);
	profiler->Reset();
	return true;
#else
	return false;
#endif	// (KR_PROFILER_ENABLED != 0)
}

#if	(PK_PARTICLES_HAS_STATS != 0)
float	CStatsManager::SPopcornFXEffectTimings::SimTime() const
{
	return m_TotalStatsReport.m_PipelineStages[PopcornFX::SEvolveStatsReport::PipelineStage_Total].m_Time;
}

float	CStatsManager::SPopcornFXEffectTimings::RenderTime() const
{
	return m_TotalStatsReport.m_PipelineStages[PopcornFX::SEvolveStatsReport::PipelineStage_TotalRenderer].m_Time;
}

float	CStatsManager::SPopcornFXEffectTimings::TotalTime() const
{
	return SimTime() + RenderTime();
}
#endif	// (PK_PARTICLES_HAS_STATS != 0)

//----------------------------------------------------------------------------

void	CStatsManager::UpdateMemory(CParticleMediumCollection *mediumCollection)
{
#if	(KR_PROFILER_ENABLED != 0)
	PK_NAMEDSCOPEDPROFILE("PopcornFXMemoryHUD::Update");

	m_CurrentFrame = (m_CurrentFrame + 1) % m_MemoryFrames.Count();

	SFrameMemory &frame = m_MemoryFrames[m_CurrentFrame];

	frame.Clear();

	frame.m_TotalParticles = GetTotalParticleCount(mediumCollection);

	//frame.m_VB.AccumPool(scene->VBPool());
	//frame.m_IB.AccumPool(scene->IBPool());

	PopcornFX::CParticlePageAllocator_Stats		pageStats;
	PopcornFX::CParticleStorageManager_MainMemory::GlobalPageAllocator()->GetTotalStats(pageStats);
	PK_ASSERT(pageStats.m_Used >= pageStats.m_Unused);
	PK_ASSERT(pageStats.m_Allocated >= pageStats.m_Freed);
	PK_ASSERT(pageStats.m_AllocCount >= pageStats.m_FreeCount);
	frame.m_PPP_TotalUsed = (float)(pageStats.m_Used - pageStats.m_Unused);
	frame.m_PPP_TotalMem = (float)(pageStats.m_Allocated - pageStats.m_Freed);
	frame.m_PPP_AllocCount = (float)(pageStats.m_AllocCount - pageStats.m_FreeCount);

#if	(KR_MEM_DEFAULT_ALLOCATOR_DEBUG != 0)
	m_MemNodesCache.Clear();
	CMemStats::CaptureStatNodes(m_MemNodesCache);
	PopcornFX::QuickSort<CMemStatNodeSorter>(m_MemNodesCache.Begin(), m_MemNodesCache.End());
#endif // (KR_MEM_DEFAULT_ALLOCATOR_DEBUG != 0)

	m_MaxFrame = PopcornFX::PKMax(m_MaxFrame, m_CurrentFrame + 1);

	m_MergedFrame.Clear();

	for (u32 framei = 0; framei < m_MaxFrame; ++framei)
	{
		const SFrameMemory	&srcFrame = m_MemoryFrames[framei];

		m_MergedFrame.m_TotalParticles += srcFrame.m_TotalParticles;

		m_MergedFrame.m_VB.Accum(srcFrame.m_VB);
		m_MergedFrame.m_IB.Accum(srcFrame.m_IB);

		m_MergedFrame.m_PPP_TotalMem += srcFrame.m_PPP_TotalMem;
		m_MergedFrame.m_PPP_TotalUsed += srcFrame.m_PPP_TotalUsed;
		m_MergedFrame.m_PPP_AllocCount += srcFrame.m_PPP_AllocCount;
	}

	const float		invMaxFrame = 1.0f / m_MaxFrame;

	m_MergedFrame.m_TotalParticles = (u32)((float)m_MergedFrame.m_TotalParticles * invMaxFrame);

	m_MergedFrame.m_VB.ComputePerFrame(invMaxFrame);
	m_MergedFrame.m_IB.ComputePerFrame(invMaxFrame);

	m_MergedFrame.m_PPP_TotalMem *= invMaxFrame;
	m_MergedFrame.m_PPP_TotalUsed *= invMaxFrame;
	m_MergedFrame.m_PPP_AllocCount *= invMaxFrame;

#endif // (KR_PROFILER_ENABLED != 0)
}

void	CStatsManager::UpdateProfiler(CParticleMediumCollection *mediumCollection)
{
#if	(PK_PARTICLES_HAS_STATS != 0)
	const CParticlesStats	&medColStats = mediumCollection->Stats();
	m_TotalParticleCount = medColStats.TotalParticleCount();
	m_TotalNewParticleCount = medColStats.m_TotalNewParticleCount.Load();

	bool		storeTimingsSum = false;
	const u32	frameCount = p_PopcornFXHUD_UpdateTimeFrameCount;

	if (++m_MediumCollectionUpdateTime_FrameCount >= frameCount)
	{
		storeTimingsSum = true;
		m_MediumCollectionUpdateTime_FrameCount = 0;
		m_MainThreadOverhead_Avg = m_MainThreadOverhead / static_cast<float>(frameCount);
		m_MainThreadOverhead = 0.0f;
		m_UpdateSpan_Avg = m_UpdateSpan / static_cast<float>(frameCount);
		m_UpdateSpan = 0.0f;
		m_UpdateSpanMainThread_Avg = m_UpdateSpanMainThread / static_cast<float>(frameCount);
		m_UpdateSpanMainThread = 0.0f;
		m_BillboardingSpan_Avg = m_BillboardingSpan / static_cast<float>(frameCount);
		m_BillboardingSpan = 0.0f;
	}

	for (u32 iMedium = 0; iMedium < mediumCollection->Mediums().Count(); iMedium++)
	{
		const PopcornFX::CParticleMedium	*medium = mediumCollection->Mediums()[iMedium].Get();
		const PopcornFX::CMediumStats		*mediumStats = medium->Stats();
		const PopcornFX::CParticleEffect	*effect = medium->Descriptor()->ParentEffect();

		bool				newRegister = false;
		PopcornFX::CGuid	effectTimingsId = m_EffectTimings.IndexOf(effect);
		if (!effectTimingsId.Valid())
		{
			newRegister = true;
			effectTimingsId = m_EffectTimings.PushBack();
		}
		if (!PK_VERIFY(effectTimingsId.Valid()))
			break;
		SPopcornFXEffectTimings	&effectTimings = m_EffectTimings[effectTimingsId];

		if (newRegister)
		{
			effectTimings.m_Effect = effect;
			effectTimings.m_EffectPath = effect->File()->Path();
		}
		PopcornFX::SEvolveStatsReport	mediumStatsReport;
		mediumStats->ComputeMediumGlobalStats(mediumStatsReport);

		effectTimings.m_TotalStatsReport += mediumStatsReport;
	}
	if (storeTimingsSum)
	{
		m_EffectTimings_Sum = m_EffectTimings;
		m_EffectTimings.Clear();
	}
#endif	// (PK_PARTICLES_HAS_STATS != 0)
}

void	CStatsManager::DrawMemory()
{
	PK_NAMEDSCOPEDPROFILE("PopcornFXMemoryHUD::Draw");

	const float	totalMem = (float)PopcornFX::CMemStats::RealFootprint();

	DrawTextLine("PopcornFX Memory Stats");

	if (totalMem == 0.0f)
	{
		DrawTextLine("CounterTracking not enabled (enabled by default only for Debug Builds)");
		return;
	}

#if	(KR_PROFILER_ENABLED != 0)
	DrawTextLine(AZStd::string::format("(Averaged over the last %d frames)", m_MaxFrame));
#endif
	DrawTextLine(AZStd::string::format("PopcornFX total memory used: %5.1f%s", HumanReadF(totalMem), HumanReadS(totalMem)));

#if	(KR_PROFILER_ENABLED != 0)
	const SFrameMemory &frame = m_MemoryFrames[m_CurrentFrame];

	const float		totalPCount = (float)frame.m_TotalParticles;
	DrawTextLine(AZStd::string::format("Particle count: %6.2f%s", HumanReadF(totalPCount, 1000), HumanReadS(totalPCount, 1000)));

	DrawTextLine(AZStd::string::format("Particle page memory used/total: %5.1f%s/%5.1f%s (%5.1f%%) (%4.0f allocs used)",
		HumanReadF(frame.m_PPP_TotalUsed), HumanReadS(frame.m_PPP_TotalUsed),
		HumanReadF(frame.m_PPP_TotalMem), HumanReadS(frame.m_PPP_TotalMem),
		100.f * frame.m_PPP_TotalUsed * SafeRcp(frame.m_PPP_TotalMem),
		frame.m_PPP_AllocCount));

	/*
	float		poolX1 = 130;
	float		poolX2 = poolX1 + 100;

#define DRAW_POOL(__name, __pool)													\
		DrawTextLeftAligned(x, y, true, __name);									\
		DrawTextLeftAligned(x + poolX1, y, true, "%5.1f%s %5.1f%%",					\
				HumanReadF((__pool).m_Mem), HumanReadS((__pool).m_Mem),				\
				(__pool).m_MemUsed * 100.f * SafeRcp((__pool).m_Mem));				\
		DrawTextLeftAligned(x + poolX2, y, ", %5.1f%s alloc, %5.1f%s release",		\
				HumanReadF((__pool).m_Alloc), HumanReadS((__pool).m_Alloc),			\
				HumanReadF((__pool).m_Release), HumanReadS((__pool).m_Release));	\

	y += yStep;
	DRAW_POOL(TEXT("Render CPU Pool VB:"), frame.m_VB);
	y += yStep;
	DRAW_POOL(TEXT("Render CPU Pool IB:"), frame.m_IB);
	y += yStep;
	*/


#if	(KR_MEM_DEFAULT_ALLOCATOR_DEBUG != 0)
	DrawTextLine("Detailed PopcornFX memory nodes current memory usage (instant, not averaged):");

	AzFramework::TextDrawParameters	headerDrawParams = m_drawParams;
	const float	offperc = 0.f;
	DrawTextLineComposed(headerDrawParams, 60.0f, "%Mem");
	const float	offmem = headerDrawParams.m_position.GetX() - m_drawParams.m_position.GetX();
	DrawTextLineComposed(headerDrawParams, 60.0f, "Mem");
	const float	offname = headerDrawParams.m_position.GetX() - m_drawParams.m_position.GetX();
	DrawTextLineComposed(headerDrawParams, 0.0f, "Memory node name");

	const float lineHeight = 13.0f;
	m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight);

	float		dispMem = 0.f;
	const float	hideBelowPerc = p_PopcornFXHUD_HideNodesBelowPercent;

	const u32	memNodeCount = m_MemNodesCache.Count();
	for (u32 i = 0; i < memNodeCount; ++i)
	{
		const PopcornFX::CMemStatNode		&node = m_MemNodesCache[i];
		const AZStd::string					&name = node.m_Name;
		const float							mem = (float)node.Footprint();
		const float							perc = 100.f * mem / totalMem;

		if (perc < hideBelowPerc)
			break;

		dispMem += mem;

		DrawVBar(offperc, offmem - 1, perc, 100.f, lineHeight);
		{
			DrawText(offperc, 0.0f, AZStd::string::format("%4.1f", perc));
			DrawText(offmem, 0.0f, AZStd::string::format("%5.1f%s", HumanReadF(mem), HumanReadS(mem)));
			DrawText(offname, 0.0f, name);
		}
		m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight);
	}

	const float			mem = dispMem;
	const float			perc = 100.f * mem / totalMem;

	DrawVBar(offperc, offmem - 1, perc, 100.f, lineHeight);
	{
		DrawText(offperc, 0.0f, AZStd::string::format("%4.1f", perc));
		DrawText(offmem, 0.0f, AZStd::string::format("%5.1f%s", HumanReadF(mem), HumanReadS(mem)));
		DrawText(offname, 0.0f, AZStd::string::format("Memory displayed here"));
	}
#else
	DrawTextLine("Detailed memory nodes statistics not available (enabled by default only for Debug Builds)");
#endif//(KR_MEM_DEFAULT_ALLOCATOR_DEBUG != 0)
#else
	DrawTextLine("Memory profile disabled");
#endif //(KR_PROFILER_ENABLED != 0)

}

void	CStatsManager::DrawProfiler()
{
#if	(PK_PARTICLES_HAS_STATS != 0)
	PK_NAMEDSCOPEDPROFILE("PopcornFXProfilerHUD::DrawProfilerHUD");

	PopcornFX::TArray<SPopcornFXEffectTimings>	allSceneTimings = m_EffectTimings_Sum;
	float										realSimTime = m_UpdateSpan_Avg;
	float										realRenderTime = m_BillboardingSpan_Avg;

	if (allSceneTimings.Empty())
		return; // Nothing to draw

	const float	hideBelowPerc = p_PopcornFXHUD_HideNodesBelowPercent;
	const u32	frameCount = p_PopcornFXHUD_UpdateTimeFrameCount;

	float	totalSimTime = 0.0f;
	float	totalRenderTime = 0.0f;

	for (u32 iTiming = 0; iTiming < allSceneTimings.Count(); ++iTiming)
	{
		totalSimTime += allSceneTimings[iTiming].SimTime() / frameCount;
		totalRenderTime += allSceneTimings[iTiming].RenderTime() / frameCount;
	}

	const float	totalTime = totalSimTime + totalRenderTime;
	const float	simTimeNormalizer = totalSimTime == 0.0f ? 0.0f : realSimTime / totalSimTime;
	const float	renderTimeNormalizer = totalRenderTime == 0.0f ? 0.0f : realRenderTime / totalRenderTime;

	DrawTextLine(AZStd::string::format("PopcornFX particles (CPU): %u (new=%u) ", m_TotalParticleCount, m_TotalNewParticleCount));
	DrawTextLine(AZStd::string::format("PopcornFX Profiler (timings are averaged over %d frames)", frameCount));

	const PopcornFX::Units::SValueAndNamedUnit	mainThreadTime = PopcornFX::Units::AutoscaleTime(m_MainThreadOverhead_Avg);
	const PopcornFX::Units::SValueAndNamedUnit	mainThreadUpdateTime = PopcornFX::Units::AutoscaleTime(m_UpdateSpan_Avg - m_UpdateSpanMainThread_Avg, 0.5f);
	const PopcornFX::Units::SValueAndNamedUnit	mainThreadRenderTime = PopcornFX::Units::AutoscaleTime(m_BillboardingSpan_Avg, 0.5f);
	const PopcornFX::Units::SValueAndNamedUnit	updateSpanTime = PopcornFX::Units::AutoscaleTime(m_UpdateSpan_Avg, 0.5f);

	DrawTextLine(AZStd::string::format("PopcornFX main thread total time: %.2f %s (update: %.2f %s, render: %.2f %s)",
						mainThreadTime.m_Value, mainThreadTime.m_UnitName,
						mainThreadUpdateTime.m_Value, mainThreadUpdateTime.m_UnitName,
						mainThreadRenderTime.m_Value, mainThreadRenderTime.m_UnitName));
	DrawTextLine(AZStd::string::format("PopcornFX update total time: %.2f %s", updateSpanTime.m_Value, updateSpanTime.m_UnitName));

	const float lineHeight = 13.0f;
	m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight);

	if (totalTime == 0.0f)
		return; // Nothing to draw

	DrawTextLine("/!\\ The following timings are estimated");

	PopcornFX::QuickSort<CEffectTimingsSorter>(allSceneTimings.Begin(), allSceneTimings.End());

	AzFramework::TextDrawParameters	headerDrawParams = m_drawParams;
	const float	relSimTimexPos = 0.0f;
	DrawTextLineComposed(headerDrawParams, 20.0f, "%CPU sim");
	const float	relRenderTimexPos = headerDrawParams.m_position.GetX() - m_drawParams.m_position.GetX();
	DrawTextLineComposed(headerDrawParams, 20.0f, "%CPU render");
	const float	effectSimTimexPos = headerDrawParams.m_position.GetX() - m_drawParams.m_position.GetX();
	DrawTextLineComposed(headerDrawParams, 20.0f, "Sim time");
	const float	effectRenderTimexPos = headerDrawParams.m_position.GetX() - m_drawParams.m_position.GetX();
	DrawTextLineComposed(headerDrawParams, 20.0f, "Render time");
	const float	effectPathxPos = headerDrawParams.m_position.GetX() - m_drawParams.m_position.GetX();
	DrawTextLineComposed(headerDrawParams, 0.0f, "Effect");

	m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight);

	//const float	maxDrawyPos = maxyPos - lineHeight * 2; // 2 last total lines
	float			displayedSimTime = 0.0f;
	float			displayedRenderTime = 0.0f;
	for (u32 iTiming = 0; iTiming < allSceneTimings.Count(); ++iTiming)
	{
		const float	effectTotalTime = allSceneTimings[iTiming].TotalTime() / frameCount;
		const float	effectTotalTimeRelative = (effectTotalTime / totalTime) * 100.0f;

		const float	effectSimTime = allSceneTimings[iTiming].SimTime() / frameCount;
		const float	effectSimTimeRelative = totalTime == 0.0f ? 0.0f : (effectSimTime / totalTime) * 100.0f;

		const float	effectRenderTime = allSceneTimings[iTiming].RenderTime() / frameCount;
		const float	effectRenderTimeRelative = totalTime == 0.0f ? 0.0f : (effectRenderTime / totalTime) * 100.0f;

		if (effectTotalTimeRelative < hideBelowPerc)
			break;

		DrawBar(relSimTimexPos, relRenderTimexPos - 3, effectSimTimeRelative / 100.0f, lineHeight);
		DrawText(relSimTimexPos, 0.0f, AZStd::string::format("%.1f", effectSimTimeRelative));
		DrawBar(relRenderTimexPos, effectSimTimexPos - 3, effectRenderTimeRelative / 100.0f, lineHeight);
		DrawText(relRenderTimexPos, 0.0f, AZStd::string::format("%.1f", effectRenderTimeRelative));

		displayedSimTime += effectSimTime;
		displayedRenderTime += effectRenderTime;

		const PopcornFX::Units::SValueAndNamedUnit	readableSimTime = PopcornFX::Units::AutoscaleTime(effectSimTime * simTimeNormalizer, 0.5f);
		const PopcornFX::Units::SValueAndNamedUnit	readableRenderTime = PopcornFX::Units::AutoscaleTime(effectRenderTime * renderTimeNormalizer, 0.5f);

		DrawText(effectSimTimexPos, 0.0f, AZStd::string::format("%.1f %s", readableSimTime.m_Value, readableSimTime.m_UnitName));
		DrawText(effectRenderTimexPos, 0.0f, AZStd::string::format("%.1f %s", readableRenderTime.m_Value, readableRenderTime.m_UnitName));
		DrawText(effectPathxPos, 0.0f, AZStd::string::format("%s", CFilePath::ExtractFilename(allSceneTimings[iTiming].m_EffectPath)));

		m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight);
		//if (yPos > maxDrawyPos)
		//	break;
	}

	m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight / 2);

	const float		displayedSimEffectsPercentage = totalTime == 0.0f ? 0.0f : (displayedSimTime / totalTime);
	const float		displayedRenderEffectsPercentage = totalTime == 0.0f ? 0.0f : (displayedRenderTime / totalTime);

	DrawBar(relSimTimexPos, relRenderTimexPos - 3, displayedSimEffectsPercentage, lineHeight);
	DrawText(relSimTimexPos, 0.0f, AZStd::string::format("%.1f", displayedSimEffectsPercentage * 100.0f));
	DrawBar(relRenderTimexPos, effectSimTimexPos - 3, displayedRenderEffectsPercentage, lineHeight);
	DrawText(relRenderTimexPos, 0.0f, AZStd::string::format("%.1f", displayedRenderEffectsPercentage * 100.0f));

	const PopcornFX::Units::SValueAndNamedUnit	readableTotalSimDisplayedTime = PopcornFX::Units::AutoscaleTime(displayedSimTime * simTimeNormalizer, 0.5f);
	const PopcornFX::Units::SValueAndNamedUnit	readableTotalRenderDisplayedTime = PopcornFX::Units::AutoscaleTime(displayedRenderTime * renderTimeNormalizer, 0.5f);

	DrawText(effectSimTimexPos, 0.0f, AZStd::string::format("%.1f %s", readableTotalSimDisplayedTime.m_Value, readableTotalSimDisplayedTime.m_UnitName));
	DrawText(effectRenderTimexPos, 0.0f, AZStd::string::format("%.1f %s", readableTotalRenderDisplayedTime.m_Value, readableTotalRenderDisplayedTime.m_UnitName));
	DrawText(effectPathxPos, 0.0f, "All Effects");

	m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + lineHeight * 2);
#endif	// (PK_PARTICLES_HAS_STATS != 0)

}

u32	CStatsManager::GetTotalParticleCount(CParticleMediumCollection *mediumCollection) const
{
	u32	totalParticleCount = 0;

	for (AZ::u32 i = 0; i < mediumCollection->Mediums().Count(); i++)
	{
		const CParticleMedium	*medium = mediumCollection->Mediums()[i].Get();
		const u32				particleCount = medium->ParticleStorage()->ActiveParticleCount();
		if (particleCount == 0)
			continue;
		totalParticleCount += particleCount;
	}
	return totalParticleCount;
}

static const char	*kHumanStr[] = {
	" ", "k", "m", "g", "t"
};

float	CStatsManager::HumanReadF(float v, u32 base /*= 1024*/) const
{
	u32	i = 0;
	while (v > base && i < PK_ARRAY_COUNT(kHumanStr) - 1)
	{
		++i;
		v /= base;
	}
	return v;
}

const char	*CStatsManager::HumanReadS(float v, u32 base /*= 1024*/) const
{
	u32		i = 0;
	while (v > base && i < PK_ARRAY_COUNT(kHumanStr) - 1)
	{
		++i;
		v /= base;
	}
	return kHumanStr[i];
}

void	CStatsManager::DrawText(float offsetX, float offsetY, const AZStd::string &text)
{
	AzFramework::TextDrawParameters	drawParams = m_drawParams;
	drawParams.m_position.SetX(m_drawParams.m_position.GetX() + offsetX);
	drawParams.m_position.SetY(m_drawParams.m_position.GetY() + offsetY);
	m_fontDrawInterface->DrawScreenAlignedText2d(drawParams, text);
}

void	CStatsManager::DrawTextLine(const AZStd::string &text)
{
	const AZ::Vector2 textSize = m_fontDrawInterface->GetTextSize(m_drawParams, text);
	m_fontDrawInterface->DrawScreenAlignedText2d(m_drawParams, text);
	m_drawParams.m_position.SetY(m_drawParams.m_position.GetY() + textSize.GetY() + m_lineSpacing);
}

void	CStatsManager::DrawTextLineComposed(AzFramework::TextDrawParameters &drawParams, float offsetX, const AZStd::string &text)
{
	const AZ::Vector2 textSize = m_fontDrawInterface->GetTextSize(m_drawParams, text);
	m_fontDrawInterface->DrawScreenAlignedText2d(drawParams, text);
	drawParams.m_position.SetX(drawParams.m_position.GetX() + textSize.GetX());
	drawParams.m_position.SetX(drawParams.m_position.GetX() + offsetX);
}

void	CStatsManager::DrawBar(float minX, float maxX, float cursor, float thickness) const
{
	EnsureHaveTextures();

	const float		x = m_drawParams.m_position.GetX();
	const float		y = m_drawParams.m_position.GetY();
	const float		sizeX = (maxX - minX) * cursor;
	Draw2dHelper	helper;

	helper.SetImageColor(AZ::Vector3(cursor, 1.f - cursor, 0.f));
	helper.DrawImage(m_whiteTex, AZ::Vector2(x + minX, y), AZ::Vector2(sizeX, thickness), 0.6f);
	helper.DrawImage(m_whiteTex, AZ::Vector2(x + minX + sizeX, y), AZ::Vector2((maxX - minX) - sizeX, thickness), 0.1f);
}

void	CStatsManager::DrawVBar(float minX, float maxX, float value, float maxValue, float thickness) const
{
	EnsureHaveTextures();

	const float		x = m_drawParams.m_position.GetX();
	const float		y = m_drawParams.m_position.GetY();
	float			sizeX = (maxX - minX) * value * SafeRcp(maxValue);
	float			c = PopcornFX::PKMax(0.f, PopcornFX::PKMin(value * SafeRcp(maxValue), 1.f));
	Draw2dHelper	helper;

	helper.SetImageColor(AZ::Vector3(c, 1.f - c, 0.0f));
	helper.DrawImage(m_whiteTex, AZ::Vector2(x + minX, y), AZ::Vector2(sizeX, thickness), 0.6f);
	helper.DrawImage(m_whiteTex, AZ::Vector2(x + minX + sizeX, y), AZ::Vector2((maxX - minX) - sizeX, thickness), 0.1f);
}

void	CStatsManager::_OnUpdateComplete(CParticleMediumCollection *medCol)
{
	AZ_UNUSED(medCol);
	StopUpdateSpanTimer();
}

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
