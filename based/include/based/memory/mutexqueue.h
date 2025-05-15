#pragma once

#include <queue>
#include <mutex>

namespace based::memory
{
	template <typename T>
	class MutexQueue
	{
	public:
		MutexQueue() = default;

		inline void PushBack(const T& item)
		{
			mLock.lock();
			mBackingQueue.push(item);
			mLock.unlock();
			mCondition.notify_all();
		}

		inline bool PopFront(T& item)
		{
			bool result = false;

			mLock.lock();

			if (!mBackingQueue.empty())
			{
				item = mBackingQueue.front();
				mBackingQueue.pop();
				result = true;
			}

			mLock.unlock();

			return result;
		}

		// ONLY FOR USE WHEN THE THREAD HAS BEEN LOCKED MANUALLY ELSEWHERE
		inline bool PopFrontUnsafe(T& item)
		{
			bool result = false;

			if (!mBackingQueue.empty())
			{
				item = mBackingQueue.front();
				mBackingQueue.pop();
				result = true;
			}

			return result;
		}

		inline uint32_t Size()
		{
			return mBackingQueue.size();
		}

		inline void Clear()
		{
			mLock.lock();
			while (!mBackingQueue.empty())
			{
				mBackingQueue.pop();
			}
			mLock.unlock();
		}

		inline bool Empty()
		{
			return mBackingQueue.empty();
		}

		inline std::mutex& GetLock() { return mLock; }
		inline std::condition_variable& GetCondition() { return mCondition; }

	private:
		std::queue<T> mBackingQueue;
		std::mutex mLock;
		std::condition_variable mCondition;
	};
}
