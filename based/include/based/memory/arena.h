#pragma once

// Arena allocator taken from https://github.com/FaultyPine/tiny_engine by Grayson Clark

namespace based::memory
{
	struct Arena
	{
		unsigned char* mBackingMemory = 0;
		std::unordered_map<void*, size_t> mFreeList;
		size_t mBackingMemorySize = 0;
		size_t mOffset = 0;
		size_t mPreviousOffset = 0;
		const char* name = "New Arena";
	};

	Arena CreateArena(void* backingMemory, size_t arenaSize);
	Arena CreateArena(void* backingMemory, size_t arenaSize, const char* name);

	void* ArenaAlloc(Arena* arena, size_t allocSize);
	void* ResizeArena(Arena* arena, void* oldMemory, size_t oldSize, size_t newSize);
	void ArenaRelease(Arena* arena, void* memoryToRelease, size_t releaseSize);

	void ArenaPopLatest(Arena* arena, void* data);
	void ClearArena(Arena* arena);
	void ArenaFreeAll(Arena* arena);

	const char* GetArenaName(Arena* arena);

	inline size_t GetFreeSpaceInArena(Arena* arena) { return arena->mBackingMemorySize - arena->mOffset; }
}