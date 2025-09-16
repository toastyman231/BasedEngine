#pragma once
#include "memory/mutexqueue.h"

// Based on https://github.com/FaultyPine/tiny_engine/blob/master/engine/src/job_system.h

namespace based::managers
{
	struct BasedJob
	{
		std::function<void()> func;
		uint32_t id;
	};

	class JobManager
	{
	public:
		void Initialize();
		void Shutdown();

		uint32_t Execute(const std::function<void()>& job);
		void ExecuteOnMainThread(const std::function<void()>& job);
		void WaitOnJob(uint32_t id);

		void SetThreadName(const std::string& name);
		void SetThreadName(std::thread* thread, const std::string& name);

		void Flush();

	private:
		memory::MutexQueue<BasedJob> mJobPool;
		memory::MutexQueue<BasedJob> mMainThreadJobPool;
		std::queue<BasedJob> mMainThreadWaitingQueue;
		uint32_t mMaxMainThreadJobs = 10u;
		uint32_t mCurrentJobID = 0;
		std::vector<std::vector<uint32_t>> mInProgressJobs;
		uint32_t mNumThreads = 1;
		uint32_t mMaxThreads = 100;
	};
}
