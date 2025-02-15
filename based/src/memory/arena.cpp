#include "pch.h"
#include "memory/arena.h"

// Arena allocator taken from https://github.com/FaultyPine/tiny_engine by Grayson Clark

namespace based::memory
{
	Arena CreateArena(void* backingMemory, size_t arenaSize)
	{
		Arena a;
		a.mBackingMemory = static_cast<unsigned char*>(backingMemory);
		a.mBackingMemorySize = arenaSize;
		a.mOffset = 0;
		a.mPreviousOffset = 0;
		return a;
	}

	Arena CreateArena(void* backingMemory, size_t arenaSize, const char* name)
	{
		Arena a = CreateArena(backingMemory, arenaSize);
		a.name = name;
		return a;
	}

	void* ArenaAlloc(Arena* arena, size_t allocSize)
	{
		for (auto [ptr, size] : arena->mFreeList)
		{
			if (size <= allocSize) continue;

			arena->mFreeList.erase(ptr);
			return ptr;
		}

		size_t& offset = arena->mOffset;
		bool isOutOfMemory = offset + allocSize > arena->mBackingMemorySize;
		BASED_ASSERT(!isOutOfMemory, "Out of memory in arena");
		if (isOutOfMemory)
		{
			return nullptr;
		}

		void* newAlloc = arena->mBackingMemory + offset;
		arena->mPreviousOffset = offset;
		offset += allocSize;
		BASED_TRACE("Arena {} allocated {} bytes, offset is now {}", GetArenaName(arena), allocSize, offset);
		return newAlloc;
	}

	void* ResizeArena(Arena* arena, void* oldMemory, size_t oldSize, size_t newSize)
	{
		uintptr_t oldMemoryAddress = (uintptr_t)oldMemory;
		uintptr_t backingMemoryAddress = (uintptr_t)arena->mBackingMemory;
		bool isOldMemInRange = oldMemoryAddress >= backingMemoryAddress && oldMemoryAddress < backingMemoryAddress + arena->mOffset;
		if (isOldMemInRange)
		{
			bool isMostRecentAlloc = oldMemoryAddress == backingMemoryAddress + arena->mPreviousOffset;
			if (isMostRecentAlloc)
			{
				arena->mOffset = arena->mPreviousOffset + newSize;
				return oldMemory;
			} else
			{
				void* newMemory = ArenaAlloc(arena, newSize);
				size_t copySize = oldSize < newSize ? oldSize : newSize; // Take smaller
				memmove(newMemory, oldMemory, copySize);
				return newMemory;
			}
		} else
		{
			BASED_ASSERT(false, "Out of bounds resize in arena");
			return nullptr;
		}
	}

	void ArenaRelease(Arena* arena, void* memoryToRelease, size_t releaseSize)
	{
		memset(memoryToRelease, 0, releaseSize);
		arena->mFreeList[memoryToRelease] = releaseSize;
		BASED_TRACE("Arena {} released {} bytes", GetArenaName(arena), releaseSize);
	}

	void ArenaPopLatest(Arena* arena, void* data)
	{
		if (arena->mOffset == arena->mPreviousOffset)
		{
			BASED_WARN("Attempted to pop on arena without a valid most recent allocation");
			return;
		}
		arena->mOffset = arena->mPreviousOffset;
		BASED_ASSERT(data == nullptr || arena->mBackingMemory + arena->mOffset == data, "Error popping data!");
	}

	void ClearArena(Arena* arena)
	{
		arena->mOffset = 0;
		arena->mPreviousOffset = 0;
		memset(arena->mBackingMemory, 0, arena->mBackingMemorySize);
		arena->mFreeList.clear();
	}

	void ArenaFreeAll(Arena* arena)
	{
		ClearArena(arena);
		arena->mBackingMemorySize = 0;
		free(arena->mBackingMemory);
		arena->mBackingMemory = nullptr;
		arena->mFreeList.clear();
	}

	const char* GetArenaName(Arena* arena)
	{
		return arena->name;
	}
}
