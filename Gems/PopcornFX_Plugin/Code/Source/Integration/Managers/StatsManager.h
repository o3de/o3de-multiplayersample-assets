//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#undef DrawText

#include <PopcornFX/PopcornFXBus.h>

#include <pk_particles/include/ps_stats.h>
#include <pk_kernel/include/kr_mem_stats.h>

#include <AzFramework/Font/FontInterface.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>

namespace PopcornFX
{
	PK_FORWARD_DECLARE(ParticleEffect);
	PK_FORWARD_DECLARE(ParticleMediumCollection);
}

namespace PopcornFX {
//----------------------------------------------------------------------------

class CStatsManager
	: protected PopcornFXProfilerRequestBus::Handler
	, public AZ::RPI::ViewportContextNotificationBus::Handler
{
public:
	void	Activate();
	void	Deactivate();
	void	Reset(CParticleMediumCollection *mediumCollection);
	void	Update(CParticleMediumCollection *mediumCollection);

	// Timers starts and stop:
	void	StartMainThreadTimer();
	void	StopMainThreadTimer();
	void	StartUpdateSpanTimer();
	void	StopUpdateSpanTimer();
	void	StopMainThreadUpdateSpanTimer();
	void	StartBillboardingSpanTimer();
	void	StopBillboardingSpanTimer();

	void	RegisterBatch();
	void	UnregisterBatch();

protected:
	////////////////////////////////////////////////////////////////////////
	// AZ::RPI::ViewportContextNotificationBus::Handler overrides...
	void OnRenderTick() override;
	////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////
	// PopcornFXProfilerRequestBus interface implementation
	bool	WriteProfileReport(const AZStd::string &path) const override;
	bool	ProfilerSetEnable(bool enable) const override;
	////////////////////////////////////////////////////////////////////////

#if	(PK_PARTICLES_HAS_STATS != 0)
	//Timing
public:
	struct	SPopcornFXEffectTimings
	{
		const PopcornFX::CParticleEffect	*m_Effect = null; // Use the effect pointer as a compare value only
		PopcornFX::CString					m_EffectPath;
		PopcornFX::SEvolveStatsReport		m_TotalStatsReport;

		float	SimTime() const;
		float	RenderTime() const;
		float	TotalTime() const;
	};

private:
	PopcornFX::TArray<SPopcornFXEffectTimings>	m_EffectTimings;
	PopcornFX::TArray<SPopcornFXEffectTimings>	m_EffectTimings_Sum;
	u32											m_MediumCollectionUpdateTime_FrameCount = 0;

	u32											m_TotalParticleCount = 0;
	u32											m_TotalNewParticleCount = 0;
#endif //	(PK_PARTICLES_HAS_STATS != 0)

#if	(KR_PROFILER_ENABLED != 0)
private:
	struct SPoolStat
	{
		float			m_Mem = 0;
		float			m_MemUsed = 0;
		float			m_Alloc = 0;
		float			m_Release = 0;

		void			Clear() { PopcornFX::Mem::Clear(*this); }
//		template <typename _Pool>
//		void			AccumPool(_Pool &pool);
		void			Accum(const SPoolStat &other);
		void			ComputePerFrame(float invFrameCount);
	};

	struct SFrameMemory
	{
		u32			m_TotalParticles;

		SPoolStat	m_IB;
		SPoolStat	m_VB;

		float		m_PPP_TotalMem;
		float		m_PPP_TotalUsed;
		float		m_PPP_AllocCount;

		void		Clear()
		{
			m_TotalParticles = 0;
			m_IB.Clear();
			m_VB.Clear();
			m_PPP_TotalMem = 0;
			m_PPP_TotalUsed = 0;
			m_PPP_AllocCount = 0;
		}
	};
	PopcornFX::TArray<SFrameMemory>	m_MemoryFrames;
	u32								m_CurrentFrame = 0;
	u32								m_MaxFrame = 0;
	SFrameMemory					m_MergedFrame;
#endif	// (KR_PROFILER_ENABLED != 0)

#if	(KR_MEM_DEFAULT_ALLOCATOR_DEBUG != 0)
private:
	PopcornFX::TArray<CMemStatNode>	m_MemNodesCache;
#endif

private:
	void	UpdateMemory(CParticleMediumCollection *mediumCollection);
	void	UpdateProfiler(CParticleMediumCollection *mediumCollection);
	void	DrawMemory();
	void	DrawProfiler();

	u32			GetTotalParticleCount(CParticleMediumCollection *mediumCollection) const;
	float		HumanReadF(float v, u32 base = 1024) const;
	const char	*HumanReadS(float v, u32 base = 1024) const;
	void		DrawText(float offsetX, float offsetY, const AZStd::string &text);
	void		DrawTextLine(const AZStd::string &text);
	void		DrawTextLineComposed(AzFramework::TextDrawParameters &drawParams, float offsetX, const AZStd::string &text);
	void		DrawBar(float minX, float maxX, float cursor, float thickness) const;
	void		DrawVBar(float minX, float maxX, float value, float maxValue, float thickness) const;

	static inline float	SafeRcp(float v) { return v == 0.f ? 0.f : 1.f / v; }

	AzFramework::TextDrawParameters				m_drawParams;
	mutable AZ::Data::Instance<AZ::RPI::Image>	m_whiteTex;
	AzFramework::FontDrawInterface				*m_fontDrawInterface = null;
	float										m_lineSpacing;
	static constexpr float						BaseFontSize = 0.7f;

	void EnsureHaveTextures() const;

public:
	void			_OnUpdateComplete(CParticleMediumCollection *medCol);


private:
	// Timers:

	// Main thread:
	PopcornFX::CTimer			m_MainThreadTimer;
	float						m_MainThreadOverhead = 0.0f;
	float						m_MainThreadOverhead_Avg = 0.0f;


	// Update span:
	PopcornFX::CTimer			m_UpdateSpanTimer;
	float						m_UpdateSpan = 0.0f;
	float						m_UpdateSpan_Avg = 0.0f;

	// Update span not counting main thread overhead:
	Threads::CCriticalSection	m_LockUpdateSpanTimer;
	PopcornFX::CTimer			m_UpdateSpanMainThreadTimer;
	bool						m_IsUpdateSpanMainThreadTimerRunning = false;
	float						m_UpdateSpanMainThread = 0.0f;
	float						m_UpdateSpanMainThread_Avg = 0.0f;

	// Update span:
	PopcornFX::CTimer			m_BillboardingSpanTimer;
	float						m_BillboardingSpan = 0.0f;
	float						m_BillboardingSpan_Avg = 0.0f;
};

struct	SScopedMainThreadTimer
{
	CStatsManager	*m_Manager;

	SScopedMainThreadTimer(CStatsManager *manager)
		: m_Manager(manager)
	{
		m_Manager->StartMainThreadTimer();
	}

	~SScopedMainThreadTimer()
	{
		m_Manager->StopMainThreadTimer();
	}
};

#define	STATS_START_MAIN_THREAD_TIMER_SCOPED(__statsManager)	SScopedMainThreadTimer		PK_GLUE(__scopedTimer, __COUNTER__)(&__statsManager)

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
