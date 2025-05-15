#include "pch.h"
#include "managers/jobmanager.h"

// Based on https://github.com/FaultyPine/tiny_engine/blob/master/engine/src/job_system.cpp

namespace based::managers
{
	void JobManager::Initialize()
	{
		uint32_t numCores = std::thread::hardware_concurrency();
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
		mMainThreadJobPool.PushBack(jobWithID);
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
		mMainThreadJobPool.GetLock().unlock();
		mMainThreadJobPool.Clear();
	}
}
