#include "pch.h"
#include "managers/jobmanager.h"

#ifdef BASED_PLATFORM_WINDOWS
#include <Windows.h>
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
#elif BASED_PLATFORM_LINUX
#include <sys/prctl.h>
#endif

// Based on https://github.com/FaultyPine/tiny_engine/blob/master/engine/src/job_system.cpp

namespace based::managers
{
	void JobManager::Initialize()
	{
		PROFILE_FUNCTION();
		uint32_t numCores = std::thread::hardware_concurrency() - 1u;
		mNumThreads = std::min(std::max(1u, numCores), mMaxThreads);
		mInProgressJobs = std::vector<std::vector<uint32_t>>(mNumThreads);
		BASED_TRACE("Creating {} job threads!", mNumThreads);

		for (uint32_t threadID = 0; threadID < mNumThreads; threadID++)
		{
			std::thread worker([this, threadID]
				{
					BasedJob job;
					std::vector<uint32_t>& inProgressJobsForThread = mInProgressJobs.at(threadID);
					while (mNumThreads > 0)
					{
						if (mJobPool.PopFront(job))
						{
							inProgressJobsForThread.push_back(job.id);
							job.func();
							inProgressJobsForThread.erase(
								std::find(
									inProgressJobsForThread.begin(),
									inProgressJobsForThread.end(),
									job.id));
						}

						std::unique_lock<std::mutex> lock(mJobPool.GetLock());
						mJobPool.GetCondition().wait(lock, 
							[this]
								{
									return !mJobPool.Empty();
								});
					}
				});
			SetThreadName(&worker, "Job Thread " + std::to_string(threadID));
			worker.detach();
		}
	}

	void JobManager::Shutdown()
	{
		mNumThreads = 0;
	}

	uint32_t JobManager::Execute(const std::function<void()>& job)
	{
		BasedJob jobWithId;
		jobWithId.func = job;
		jobWithId.id = mCurrentJobID++;
		mJobPool.PushBack(jobWithId);
		return jobWithId.id;
	}

	void JobManager::ExecuteOnMainThread(const std::function<void()>& job)
	{
		BasedJob jobWithID;
		jobWithID.func = job;
		jobWithID.id = mCurrentJobID++;

		if (mMainThreadJobPool.Size() >= mMaxMainThreadJobs)
		{
			mMainThreadWaitingQueue.push(jobWithID);
		} else
		{
			mMainThreadJobPool.PushBack(jobWithID);
		}
	}

	void JobManager::WaitOnJob(uint32_t id)
	{
		bool jobInProgress = true;

		while (jobInProgress)
		{
			for (int threadIndex = 0; threadIndex < mInProgressJobs.size(); threadIndex++)
			{
				const std::vector<uint32_t>& inProgressJobsForThread = mInProgressJobs.at(threadIndex);
				if (!std::count(inProgressJobsForThread.begin(), inProgressJobsForThread.end(), id))
				{
					jobInProgress = false;
				}
			}
		}
	}

	// Thread name operations based on https://stackoverflow.com/a/23899379
#ifdef BASED_PLATFORM_WINDOWS
	void SetThreadNameWindows(uint32_t dwThreadID, const std::string& name)
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = name.c_str();
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;
		__try
		{
			RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
#endif

	void JobManager::SetThreadName(const std::string& name)
	{
		PROFILER_SET_THREAD_NAME(name.c_str());
#ifdef BASED_PLATFORM_WINDOWS
		SetThreadNameWindows(GetCurrentThreadId(), name.c_str());
#elif BASED_PLATFORM_LINUX
		prctl(PR_SET_NAME,threadName,0,0,0);
#endif
	}

	void JobManager::SetThreadName(std::thread* thread, const std::string& name)
	{
		PROFILER_SET_THREAD_NAME(name.c_str());
#ifdef BASED_PLATFORM_WINDOWS
		DWORD threadId = ::GetThreadId( static_cast<HANDLE>( thread->native_handle() ) );
		SetThreadNameWindows(threadId, name);
#elif BASED_PLATFORM_LINUX
		auto handle = thread->native_handle();
		pthread_setname_np(handle,threadName);
#endif
	}

	void JobManager::Flush()
	{
		PROFILE_FUNCTION();
		mMainThreadJobPool.GetLock().lock();
		while (!mMainThreadJobPool.Empty())
		{
			BasedJob job;
			mMainThreadJobPool.PopFrontUnsafe(job);
			job.func();
		}
		mMainThreadJobPool.ClearUnsafe();

		while (!mMainThreadWaitingQueue.empty() && mMainThreadJobPool.Size() < mMaxMainThreadJobs)
		{
			mMainThreadJobPool.PushBackUnsafe(mMainThreadWaitingQueue.front());
			mMainThreadWaitingQueue.pop();
		}
		mMainThreadJobPool.GetLock().unlock();
		mMainThreadJobPool.NotifyAll();
	}
}
