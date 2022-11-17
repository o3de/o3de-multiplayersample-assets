//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFxStartup.h"

#if defined(O3DE_USE_PK)

#if	defined(PK_LINUX) || defined(PK_MACOSX)
#	include <stdio.h>
#	include <unistd.h>
#endif

#if defined(PK_MACOSX)
#	include <signal.h>
#endif

#include "Integration/ResourceHandlers/ImageResourceHandler.h"
#include "Integration/ResourceHandlers/MeshResourceHandler.h"
#include "Integration/Render/BasicEditorRendererFeatures.h"
#include "Integration/File/FileSystemController_O3DE.h"

#include <AzCore/Memory/Memory.h>
#include <AzCore/Jobs/Job.h>

#include <pk_version_base.h>
#include <pk_kernel/include/kr_init.h>
#include <pk_base_object/include/hb_init.h>
#include <pk_engine_utils/include/eu_init.h>
#include <pk_compiler/include/cp_init.h>
#include <pk_imaging/include/im_init.h>
#include <pk_geometrics/include/ge_init.h>
#include <pk_geometrics/include/ge_coordinate_frame.h>
#include <pk_particles/include/ps_init.h>
#include <pk_particles_toolbox/include/pt_init.h>
#include <pk_render_helpers/include/rh_init.h>
#include <pk_kernel/include/kr_assert_internals.h>
#include <pk_kernel/include/kr_static_config_flags.h>
#include <pk_kernel/include/kr_log_listeners.h>
#include <pk_kernel/include/kr_log_listeners_file.h>
#include <pk_kernel/include/kr_thread_pool_default.h>
#include <pk_kernel/include/kr_thread_pool_details.h>

#ifdef PK_DEBUG
#include <pk_kernel/include/kr_mem_stats.h>
#endif


#define PK_ASSERT_DISPLAY_HBO_SCOPE		0 // Disabled for now: Rethink in v2

#if (PK_ASSERT_DISPLAY_HBO_SCOPE != 0)
#	include <pk_base_object/include/hbo_scope_details.h>
#	include <pk_particles/include/ps_debug.h>
#endif

#if defined(PK_NX)
#	define	LOG_FILE "cache:/popcorn.htm"
#else
#	define	LOG_FILE "popcorn.htm"
#endif

#ifndef PK_RETAIL
#	define	CATCH_ASSERTS		1
#else
#	define	CATCH_ASSERTS		0
#endif

PK_LOG_MODULE_DECLARE();

//----------------------------------------------------------------------------

// Use HHAssertImpl
#if	(PK_ASSERTS_IN_DEBUG != 0) || (PK_ASSERTS_IN_RELEASE != 0)
#	if	(defined(PK_WINDOWS) && !defined(PK_DURANGO)) && ((PK_ASSERTS_IN_DEBUG != 0) || (PK_ASSERTS_IN_RELEASE != 0))
#		define USE_DEFAULT_ASSERT_IMPL		1
#	endif
#endif

//----------------------------------------------------------------------------
namespace PopcornFX {
	//----------------------------------------------------------------------------
	//
	// Assert Catcher
	//
	//----------------------------------------------------------------------------

#if (CATCH_ASSERTS == 0)

	PopcornFX::Assert::EResult	AssertCatcher_IgnoreALL(PK_ASSERT_CATCHER_PARAMETERS)
	{
		return Assert::Result_Ignore;
	}

#else // if (CATCH_ASSERTS != 0)

	//----------------------------------------------------------------------------
	//
	// This function can assert !
	// so, requires external recursion protection, ie: ASSERT_CATCHER_SCOPED_PROTECTION();
	//

	TAtomic<u32>		g_GlobalAssertId = 0;
	void	PrettyFormatAssert_Unsafe(char *buffer, u32 bufferLen, PK_ASSERT_CATCHER_PARAMETERS)
	{
		const CThreadID		threadId = CCurrentThread::ThreadID();
		const u32			globalAssertId = g_GlobalAssertId.FetchInc();
		const char			*baseObject = null;
#	if (PK_ASSERT_DISPLAY_HBO_SCOPE != 0)
		CString				_baseObjectCache;
		if (HBOScope::MainContext() != null)
		{
			TMemoryView<const HBOScope::SNodeRecord>	stack = HBOScope::MainContext()->CurrentStackNowAndHere();
			if (!stack.Empty())
			{
				BuildParticleObjectTreeViewName(_baseObjectCache, stack.Last().m_BaseObjectPath);
				baseObject = _baseObjectCache.Data();
			}
		}
#	endif
		// pointer compare ok (compiler probably removed duplicated static strings, else we dont care)
		const bool		pexp = (expanded != failed);
		const bool		pmsg = (message != failed && message != expanded);
		const bool		pbo = (baseObject != null);
		SNativeStringUtils::SPrintf(
			buffer, bufferLen,
			"!! PopcornFX Assertion failed !!"
			"\nFile       : %s(%d)"
			"\nFunction   : %s(...)"
			"%s%s"// Message
			"\nCondition  : %s"
			"%s%s" // Expanded
			"%s%s" // BaseObject
			"\nThreadID   : %d"
			"\nAssertNum  : %d"
			"\n"
			, file, line
			, function
			, (pmsg ? "\nMessage    : " : ""), (pmsg ? message : "")
			, failed
			, (pexp ? "\nExpanded   : " : ""), (pexp ? expanded : "")
			, (pbo ? "\nBaseObject : " : ""), (pbo ? baseObject : "")
			, u32(threadId)
			, globalAssertId
		);
	}

	//----------------------------------------------------------------------------

	static void		_LogAssertFunc_CLog(const char *str) { CLog::Log(PK_ERROR, "%s", str); }

#if	defined(PK_LINUX) || defined(PK_MACOSX)
	static void		_LogAssertFunc_CLog_And_RawError(const char *str) { _LogAssertFunc_CLog(str); fprintf(stderr, "%s\n", str); }
#endif

	static void		PrettyLogAssert_Unsafe(void(*cbLogAssertFunc)(const char *), PK_ASSERT_CATCHER_PARAMETERS)
	{
		char			_buffer[2048];
		PrettyFormatAssert_Unsafe(_buffer, sizeof(_buffer), PK_ASSERT_CATCHER_ARGUMENTS);
		cbLogAssertFunc(_buffer);
	}

	//----------------------------------------------------------------------------

	struct SScopedAssertCatcherProtection
	{
		static PopcornFX::Threads::CCriticalSection		s_AssertProtection_Lock;
		static volatile bool							s_AssertProtection_Asserting;

		s32		m_Return;
		SScopedAssertCatcherProtection(PK_ASSERT_CATCHER_PARAMETERS)
			: m_Return(-1)
		{
			AZ_UNUSED(function); AZ_UNUSED(failed); AZ_UNUSED(expanded);

			s_AssertProtection_Lock.Lock();
			if (s_AssertProtection_Asserting)
			{
				CLog::Log(PK_ERROR, "!! ASSERT RECURSION !! %s %s(%d)", message, file, line);
				m_Return = Assert::Result_Skip;
				return;
			}
			s_AssertProtection_Asserting = true;
		}
		~SScopedAssertCatcherProtection()
		{
			s_AssertProtection_Asserting = false;
			s_AssertProtection_Lock.Unlock();
		}
	};
	PopcornFX::Threads::CCriticalSection	SScopedAssertCatcherProtection::s_AssertProtection_Lock;
	volatile bool							SScopedAssertCatcherProtection::s_AssertProtection_Asserting = false;

#define ASSERT_CATCHER_SCOPED_PROTECTION()								\
	SScopedAssertCatcherProtection		_protection(PK_ASSERT_CATCHER_ARGUMENTS); \
	if (_protection.m_Return >= 0)										\
		return static_cast<PopcornFX::Assert::EResult>(_protection.m_Return); \

	//----------------------------------------------------------------------------

	PopcornFX::Assert::EResult	AssertCatcher_LogOnly(PK_ASSERT_CATCHER_PARAMETERS)
	{
		ASSERT_CATCHER_SCOPED_PROTECTION();

		PrettyLogAssert_Unsafe(_LogAssertFunc_CLog, PK_ASSERT_CATCHER_ARGUMENTS);

		PopcornFX::Assert::EResult		result;

#ifdef PK_DEBUG
		// always print
		result = Assert::Result_Skip; // just skip this time, don't break
#else
		CLog::Log(PK_ERROR, "This assert will be ignored next time");
		result = Assert::Result_Ignore; // skip and ignore next time
#endif

										//result = Assert::Result_Break; // break point in the code
		return result;
	}

	//----------------------------------------------------------------------------

#if defined(PK_MACOSX)
	Assert::EResult		AssertCatcher_LogThenNativeOsxAlert(PK_ASSERT_CATCHER_PARAMETERS)
	{
		ASSERT_CATCHER_SCOPED_PROTECTION();
		char			_buffer[2048];
		PrettyFormatAssert_Unsafe(_buffer, sizeof(_buffer), PK_ASSERT_CATCHER_ARGUMENTS);
		_LogAssertFunc_CLog(_buffer);

		int				pipeFds[2] = { -1, -1 };
		pid_t			pId = -1;
		int				status = 0;
		Assert::EResult	result = Assert::EResult::Result_Skip;

		// Don't mind me, just recoding a minishell
		status = pipe(pipeFds);
		if (status == 0)
		{
			pId = fork();
			if (pId == 0) // child
			{
				char cmd[2048 + 512];

				close(pipeFds[0]);
				if (dup2(pipeFds[1], 1) == -1)
				{
					perror("PKFX Assert dup2 : ");
					close(pipeFds[1]);
					exit(1);
				}

				for (int i = 0; i < 2048; ++i)
				{
					if (_buffer[i] == '\"')
						_buffer[i] = '\'';
				}

				SNativeStringUtils::SPrintf(cmd,
					"set myReply to button returned of "
					"(display dialog \"%s\" default button 2 buttons {\"Break\", \"Skip\", \"Ignore\"})",
					_buffer);
				execl("/usr/bin/osascript", "osascript", "-e", "tell application \"System Events\"",
					"-e", "activate",
					"-e", cmd,
					"-e", "end tell",
					null);

				perror("PKFX Assert exec osascript");
				exit(1);
			}
			else if (pId > 0) // parent
			{
				char ret[2];

				close(pipeFds[1]);
				pipeFds[1] = -1;
				waitpid(pId, &status, WUNTRACED);
				if (status == 0)
				{
					status = read(pipeFds[0], &ret, 2);
					if (status == 2)
					{
						if (ret[0] == 'B' && ret[1] == 'r')
							result = Assert::EResult::Result_Break;
						else if (ret[0] == 'S' && ret[1] == 'k')
							result = Assert::EResult::Result_Skip;
						else if (ret[0] == 'I' && ret[1] == 'g')
							result = Assert::EResult::Result_Ignore;
						else
							perror("PKFX Assert wrong retcode");
					}
					else if (status >= 0)
					{
						perror("PKFX Assert read");
						fprintf(stderr, "read %d bytes\n", status);
					}
					else
						perror("PKFX Assert read failed");
				}
				else
				{
					kill(pId, SIGKILL);
					perror("PKFX Assert waitpid");
				}
			}
			else
				perror("PKFX Assert fork");
		}
		else
			perror("PKFX Assert pipe");

		if (pipeFds[0] != -1)
			close(pipeFds[0]);
		if (pipeFds[1] != -1)
			close(pipeFds[1]);
		return result;
	}
#endif // defined(PK_MACOSX)

	//----------------------------------------------------------------------------

#if	defined(PK_LINUX) || defined(PK_MACOSX)

	//----------------------------------------------------------------------------

	const char			*g_AssertScriptFilePath = 0;

	//----------------------------------------------------------------------------

	Assert::EResult		AssertCatcher_BreakForever(PK_ASSERT_CATCHER_PARAMETERS)
	{
		ASSERT_CATCHER_SCOPED_PROTECTION();
		PrettyLogAssert_Unsafe(_LogAssertFunc_CLog, PK_ASSERT_CATCHER_ARGUMENTS);
		return Assert::Result_Break;
	}

	//----------------------------------------------------------------------------

	Assert::EResult		AssertCatcher_ExecScript(PK_ASSERT_CATCHER_PARAMETERS)
	{
		ASSERT_CATCHER_SCOPED_PROTECTION();

		PopcornFX::Assert::EResult		result = Assert::Result_Skip;

		PrettyLogAssert_Unsafe(_LogAssertFunc_CLog, PK_ASSERT_CATCHER_ARGUMENTS);

		char		cmd[2048];
		SNativeStringUtils::SPrintf(cmd, "%s %d \"%s\" \"%s\" \"%d\" \"%s\" \"%s\" \"%s\"",
			g_AssertScriptFilePath, getpid(), PK_ASSERT_CATCHER_ARGUMENTS);
		int			res = system(cmd);
		if (res >= 0)
		{
			if (WIFSIGNALED(res))
			{
				Assert::SetCatcher(&AssertCatcher_BreakForever);
				result = Assert::Result_Break;
			}
			else
			{
				int		status = WEXITSTATUS(res);
				switch (status)
				{
				case 1: result = Assert::Result_Break; break;
				case 2: result = Assert::Result_Ignore; break;
				case 3:
					Assert::SetCatcher(&AssertCatcher_BreakForever);
					result = Assert::Result_Break;
					break;
				}
			}
		}
		return result;
	}

	//----------------------------------------------------------------------------

	Assert::EResult	AssertCatcher_AskTTY(PK_ASSERT_CATCHER_PARAMETERS)
	{
		ASSERT_CATCHER_SCOPED_PROTECTION();

		PopcornFX::Assert::EResult		result = Assert::Result_Skip;

		// also printf beacuse CLog could be a log file only
		PrettyLogAssert_Unsafe(_LogAssertFunc_CLog_And_RawError, PK_ASSERT_CATCHER_ARGUMENTS);

		fprintf(stderr, "\nWhat now ? (0:break, 1:skip, 2:ignore, 3:break forever) ");
		u32		d = 0;
		if (fscanf(stdin, "%d", &d) == EOF)
			d = 0;	// avoid a warning on gcc

		switch (d)
		{
		case 1: result = Assert::Result_Skip; break;
		case 2: result = Assert::Result_Ignore; break;
		case 3:
			Assert::SetCatcher(&AssertCatcher_BreakForever);
			result = Assert::Result_Break;
			break;
		default: result = Assert::Result_Break; break;
		}
		return result;
	}
#endif // defined(PK_LINUX) || defined(PK_MACOSX)

	//----------------------------------------------------------------------------

#if defined(USE_DEFAULT_ASSERT_IMPL)
	Assert::EResult	AssertCatcher_LogThenDefault(PK_ASSERT_CATCHER_PARAMETERS)
	{
		ASSERT_CATCHER_SCOPED_PROTECTION();
		PrettyLogAssert_Unsafe(_LogAssertFunc_CLog, PK_ASSERT_CATCHER_ARGUMENTS);
		return PKAssertImpl(PK_ASSERT_CATCHER_ARGUMENTS);
	}
#endif

#endif // (CATCH_ASSERTS != 0)

	//----------------------------------------------------------------------------
	//
	// Loggers
	//
	//----------------------------------------------------------------------------

#if	defined(PK_WINDOWS)
	class	CLogListenerStdout : public ILogListener
	{
	private:
	public:
		virtual void	Notify(CLog::ELogLevel level, CGuid logClass, const char *message) override
		{
			CString	s = CString::Format("[%d][%s]> %s\n", level, CLog::LogClassToString(logClass), message);
			printf("%s", s.Data());
		}
	};
#endif

	//----------------------------------------------------------------------------

	void	AddDefaultLogListenersOverride_DefaultLogger(void *userHandle)
	{
		AZ_UNUSED(userHandle);

#ifndef	PK_RETAIL	// don't log anything to debut output in retail
		CLog::AddGlobalListener(PK_NEW(CLogListenerFile(LOG_FILE, "popcorn-engine logfile")));
		CLog::AddGlobalListener(PK_NEW(CLogListenerDebug()));
#	if	defined(PK_WINDOWS)
		CLog::AddGlobalListener(PK_NEW(CLogListenerStdout));
#	endif
#endif
	}

	//----------------------------------------------------------------------------

	void	AddDefaultLogListenersOverride_NoDefaultLogger(void *userHandle)
	{
		AZ_UNUSED(userHandle);
#ifndef	PK_RETAIL	// don't log anything to debut output in retail
		// only log to the LOG_FILE, not to the debug output
		CLog::AddGlobalListener(PK_NEW(CLogListenerFile(LOG_FILE, "popcorn-engine logfile")));
#endif
	}

	//----------------------------------------------------------------------------
	//
	// Worker Thread Pool
	//
	//----------------------------------------------------------------------------

#if 0
	static Threads::CAbstractPool		*_CreateThreadPool_Numa()
	{
		CWorkerThreadPool				*pool = PK_NEW(CWorkerThreadPool);
		if (!PK_VERIFY(pool != null))
			return null;
		// Uses cpu core caps to spawn 1 worker per logical core with affinity
		pool->AddNUMAWorkers();
		pool->StartWorkers();
		return pool;
	}

	static void							_DestroyThreadPool(Threads::CAbstractPool *pool)
	{
		PK_DELETE(pool);
	}
#endif

#define		USE_O3DE_JOBS		0

#if		USE_O3DE_JOBS
	struct	SO3DEJob : public AZ::Job
	{
		AZ_CLASS_ALLOCATOR(SO3DEJob, AZ::ThreadPoolAllocator, 0);

		SO3DEJob(AZ::JobContext *context = null)
			: AZ::Job(true, context)
			, m_PkfxTask(null)
			, m_ThreadContext(null)
		{
		}

		~SO3DEJob() = default;

		Threads::PAbstractTask		m_PkfxTask;
		Threads::SThreadContext 	*m_ThreadContext;

	protected:
		void Process() override
		{
			PK_SCOPEDPROFILE();
			if (!CCurrentThread::IsRegistered())
				CCurrentThread::RegisterUserThread();
			m_PkfxTask->Run(*m_ThreadContext);
		}
	};

	class	CO3DEThreadPool : public Threads::CAbstractPool
	{
	public:
		CO3DEThreadPool()
			: m_Context(this, 1)
		{
		}

		virtual ~CO3DEThreadPool()
		{
		}

		// These can be called by tasks to enqueue other dependent tasks
		void	SubmitTask(Threads::CAbstractTask *task) override
		{
			PK_SCOPEDPROFILE();
			if (!PK_VERIFY(task != null))
				return;

			const CPU::CScoped_FpuDisableExceptions	_de;
			const CPU::CScoped_FpuEnableFastMode	_ftz;

			SO3DEJob	*job = new SO3DEJob;

			job->m_ThreadContext = &m_Context;
			job->m_PkfxTask = task;

			job->Start();
		}

		CStringView	ThreadPoolName() const override { return "O3DE PopcornFX thread pool"; };

	private:
		Threads::SThreadContext				m_Context;
	};
	PK_DECLARE_REFPTRCLASS(O3DEThreadPool);
#endif

	static Threads::PAbstractPool		_CreateCustomThreadPool()
	{
		bool						success = true;

#if		USE_O3DE_JOBS
		// O3DE thread pool:
		CO3DEThreadPool				*threadPool = PK_NEW(CO3DEThreadPool);
		return threadPool;
#else

		bool						hasExplicitAffinities = false;
		u32							affinityCount = 0;
		CThreadManager::EPriority	workersPriority = CThreadManager::Priority_High;
		const u32					kWorkerAffinities[] =
#if		defined(PK_ORBIS)
			// Default worker affinities on Orbis:
		{
			1 << 2,
			1 << 3, // our main thread
			1 << 4,
			1 << 5,
		};
#else
		{ 0 };
#endif

#if		defined(PK_ORBIS)
		affinityCount = PK_ARRAY_COUNT(kWorkerAffinities);
		workersPriority = CThreadManager::Priority_BackgroundLow;
		hasExplicitAffinities = true;
#endif

		PWorkerThreadPool	pool = PK_NEW(CWorkerThreadPool);

		if (PK_VERIFY(pool != null))
		{
			if (hasExplicitAffinities)	// user specified explicit worker affinities: override our own scheduling entirely.
			{
				CLog::Log(PK_INFO, "Found explicit user affinity masks for worker-threads. Overriding default thread-pool creation.");

				CGenericAffinityMask	affinityMask;

				for (u32 i = 0; i < affinityCount; i++)
				{
					if (kWorkerAffinities[i] != 0)
					{
						affinityMask.Clear();
						affinityMask.SetAffinityBlock32(0, kWorkerAffinities[i]);

						success &= (null != pool->AddWorker(workersPriority, &affinityMask));
					}
				}

				if (!success)
					CLog::Log(PK_ERROR, "Could not create some of the explicitely affinitized worker-threads.");

				success = true;	// still keep what has been created. don't fail here.
			}
			else
			{
#if defined(PK_DESKTOP) || defined(PK_MOBILE)
				// Let the OS shedule our workers
				// leave 2 core for main thread and render thread
				u32		processorCount = PKMin(PKMax(CPU::Caps().ProcessAffinity().NumBitsSet(), 4U) - 3U, 12);

				success = pool->AddFullAffinityWorkers(processorCount, CPU::Caps().ProcessAffinity(), workersPriority);
#else
				success = pool->AddNUMAWorkers();
#endif
			}

			if (!success)
				return null;

			pool->StartWorkers();
		}
		return pool;
#endif
	}


	//----------------------------------------------------------------------------
	//
	// Startup
	//
	//----------------------------------------------------------------------------

	PopcornFX::IFileSystem	*PopcornFX_CreateNewFileSystem()
	{
		return PK_NEW(FileSystemController_O3DE);
	}

	void	*PopcornFX_Alloc(size_t size, PopcornFX::Mem::EAllocType type)
	{
		(void)type;
		return AZ::AllocatorInstance<AZ::SystemAllocator>::Get().Allocate(size, AZCORE_GLOBAL_NEW_ALIGNMENT); // popcornfx takes care of alignment already
	}

	void	*PopcornFX_Realloc(void *ptr, size_t size, PopcornFX::Mem::EAllocType type)
	{
		(void)type;
		return AZ::AllocatorInstance<AZ::SystemAllocator>::Get().ReAllocate(ptr, size, AZCORE_GLOBAL_NEW_ALIGNMENT);
	}

	void	PopcornFX_Free(void *ptr, PopcornFX::Mem::EAllocType type)
	{
		(void)type;
		return AZ::AllocatorInstance<AZ::SystemAllocator>::Get().DeAllocate(ptr);
	}

	bool	PopcornStartup(const char *assertScriptFilePath /*= null*/, bool installDefaultLogger /*= true*/)
	{
		AZ_UNUSED(assertScriptFilePath);

#ifdef PK_DEBUG
		PopcornFX::CMemStats::EnableCounterTracking(true);
#endif

		SDllVersion	engineVersion;

		PK_ASSERT(engineVersion.Major == PK_VERSION_MAJOR);
		PK_ASSERT(engineVersion.Minor == PK_VERSION_MINOR);
#ifdef	PK_DEBUG
		PK_ASSERT(engineVersion.Debug == true);
#else
		PK_ASSERT(engineVersion.Debug == false);
#endif
		CPKKernel::Config	configKernel;

		// Setup FileSystem
		configKernel.m_NewFileSystem = PopcornFX_CreateNewFileSystem;

#if	defined(PK_ORBIS)
		// Setup Allocator
		configKernel.m_DefaultAllocator_Alloc = &PopcornFX_Alloc;
		configKernel.m_DefaultAllocator_Realloc = &PopcornFX_Realloc;
		configKernel.m_DefaultAllocator_Free = &PopcornFX_Free;
		//configKernel.m_DefaultAllocator_OutOfMemory = &PopcornFX_AllocFailed;
#endif

#if (CATCH_ASSERTS == 0)
		configKernel.m_AssertCatcher = &AssertCatcher_IgnoreALL;
#else
#	if	defined(PK_LINUX) || defined(PK_MACOSX)
		configKernel.m_AssertCatcher = null;
		g_AssertScriptFilePath = assertScriptFilePath;
		if (g_AssertScriptFilePath != 0)
		{
			if (access(g_AssertScriptFilePath, X_OK) == 0)
			{
				int		res = system(g_AssertScriptFilePath); // exec once
				if (res == 0)
				{
					printf("PKFX: Assert script catcher \"%s\" enabled\n", g_AssertScriptFilePath);
					configKernel.m_AssertCatcher = &AssertCatcher_ExecScript;
				}
				else
					fprintf(stderr, "PKFX: Assert script catcher \"%s\" not valid\n", g_AssertScriptFilePath);
			}
			else
				fprintf(stderr, "PKFX: Assert script catcher \"%s\" not found or permission denied\n", g_AssertScriptFilePath);
		}
		if (configKernel.m_AssertCatcher == null)
		{
			if (isatty(fileno(stdin)) && isatty(fileno(stdout)))
			{
				printf("PKFX: Assert Catcher will ask on the TTY !\n");
				configKernel.m_AssertCatcher = &AssertCatcher_AskTTY;
			}
			else
				configKernel.m_AssertCatcher = &AssertCatcher_LogOnly;
		}

#	elif defined(USE_DEFAULT_ASSERT_IMPL)
		// leave it as it ?
		// or use hellheaven_api/hhapi_assert_notify_win32.cpp ?
		configKernel.m_AssertCatcher = &AssertCatcher_LogThenDefault;
#	else
		configKernel.m_AssertCatcher = &AssertCatcher_LogOnly;
#	endif
#endif // (CATCH_ASSERTS != 0)

#if	defined(PK_ORBIS)
		// Our main-thread is scheduled on core 3 (see the CCurrentThread::SetTargetProcessor() call below)
		const u32	kOrbisWorkerAffinities[] =
		{
			0x07,
			0x07,
			0x07,
			//			0x0F,	// our main thread
			0x30,
			0x30,
			//			0xF0,	// reserved by the system
			//			0xF0,	// reserved by the system
		};
		configKernel.m_WorkerAffinityMasks = kOrbisWorkerAffinities;
		configKernel.m_WorkerAffinityMasksCount = PK_ARRAY_COUNT(kOrbisWorkerAffinities);
#endif

		//const bool		forceNumaThreadPool = false;
		//if (forceNumaThreadPool)
		//{
		//	configKernel.m_CreateThreadPool = &_CreateThreadPool_Numa;
		//	configKernel.m_DestroyThreadPool = &_DestroyThreadPool;
		//}

		configKernel.m_CreateThreadPool = &_CreateCustomThreadPool;

		// depending on wether or not we want default loggers, override the callback:
		if (installDefaultLogger)
			configKernel.m_AddDefaultLogListeners = &AddDefaultLogListenersOverride_DefaultLogger;
		else
			configKernel.m_AddDefaultLogListeners = &AddDefaultLogListenersOverride_NoDefaultLogger;

		CPKParticles::Config	particleConfig;

		particleConfig.m_EnableBillboardingStats = true;
		if (CPKKernel::Startup(engineVersion, configKernel) &&
			CPKBaseObject::Startup(engineVersion, CPKBaseObject::Config()) &&
			CPKEngineUtils::Startup(engineVersion, CPKEngineUtils::Config()) &&
			CPKCompiler::Startup(engineVersion, CPKCompiler::Config()) &&
			CPKImaging::Startup(engineVersion, CPKImaging::Config()) &&
			CPKGeometrics::Startup(engineVersion, CPKGeometrics::Config()) &&
			CPKParticles::Startup(engineVersion, particleConfig) &&
			ParticleToolbox::Startup() &&
			CPKRenderHelpers::Startup(engineVersion, CPKRenderHelpers::Config()) &&
			Kernel::CheckStaticConfigFlags(Kernel::g_BaseStaticConfig, SKernelConfigFlags()))
		{
			PK_LOG_MODULE_INIT_START;

			// Set the coordinate frame:
			CCoordinateFrame::SetGlobalFrame(Frame_RightHand_Z_Up);

			BasicFeatures::Startup();

			// Bind resource handlers:
			CImageResourceHandler	*imgHandler = PK_NEW(CImageResourceHandler);
			if (imgHandler != null)
				Resource::DefaultManager()->RegisterHandler<CImage>(imgHandler);

			CMeshResourceHandler		*meshHandler = PK_NEW(CMeshResourceHandler);
			if (meshHandler != null)
				Resource::DefaultManager()->RegisterHandler<CResourceMesh>(meshHandler);

			PK_LOG_MODULE_INIT_END;

			CLog::Log(PK_INFO, "Started");
			return true;
		}

		PopcornShutdown();	// shutdown the modules we were able to load...
		return false;
	}

	//----------------------------------------------------------------------------

	void	PopcornShutdown()
	{
		PK_LOG_MODULE_RELEASE_START;

		IResourceHandler	*imgHandler;
		imgHandler = Resource::DefaultManager()->FindHandler<CImage>();
		Resource::DefaultManager()->UnregisterHandler<CImage>(imgHandler);
		PK_DELETE(imgHandler);

		IResourceHandler	*meshHandler;
		meshHandler = Resource::DefaultManager()->FindHandler<CResourceMesh>();
		Resource::DefaultManager()->UnregisterHandler<CResourceMesh>(meshHandler);
		PK_DELETE(meshHandler);

		PK_LOG_MODULE_RELEASE_END;

		BasicFeatures::Shutdown();

		CPKRenderHelpers::Shutdown();
		ParticleToolbox::Shutdown();
		CPKParticles::Shutdown();
		CPKGeometrics::Shutdown();
		CPKImaging::Shutdown();
		CPKCompiler::Shutdown();
		CPKEngineUtils::Shutdown();
		CPKBaseObject::Shutdown();
		CPKKernel::Shutdown();
	}

}
//----------------------------------------------------------------------------

#endif //O3DE_USE_PK

