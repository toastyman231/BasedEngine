#include "memory/arena.h"

#include "log.h"

// Arena allocator taken from https://github.com/FaultyPine/tiny_engine by Grayson Clark

namespace based::memory
{
	Arena CreateArena(void* backingMemory, size_t arenaSize)
	{
		Arena a;
		a.mBackingMemory = (unsigned char*)backingMemory;
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
		size_t& offset = arena->mOffset;
		bool isOutOfMemory = offset + allocSize > arena->mBackingMemorySize;
		BASED_ASSERT(!isOutOfMemory, "Out of memory in arena")
		if (isOutOfMemory)
		{
			return nullptr;
		}

		void* newAlloc = arena->mBackingMemory + offset;
		arena->mPreviousOffset = offset;
		offset += allocSize;
		BASED_TRACE("Arena {} offset is now {}", GetArenaName(arena), offset);
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
	}

	void ArenaFreeAll(Arena* arena)
	{
		ClearArena(arena);
		arena->mBackingMemorySize = 0;
		free(arena->mBackingMemory);
		arena->mBackingMemory = nullptr;
	}

	const char* GetArenaName(Arena* arena)
	{
		return arena->name;
	}
}
