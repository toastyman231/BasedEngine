#pragma once

#if defined(BASED_PLATFORM_WINDOWS)
#pragma comment(linker, "/alternatename:malloc=my_malloc")
#pragma comment(linker, "/alternatename:free=my_free")
#pragma comment(linker, "/alternatename:realloc=my_realloc")
#pragma comment(linker, "/alternatename:calloc=my_calloc")
#endif

// Core C heap
// Overriding these is a huge ass pain on Windows, so we just hope that the libraries we use
// give us a mechanism to supply them ourselves
#ifdef __cplusplus
extern "C" {
#endif
void* my_malloc(size_t size);
void  my_free(void* ptr);
void* my_realloc(void* ptr, size_t size);
void* my_calloc(size_t num, size_t size);

#if defined(BASED_PLATFORM_WINDOWS) && defined(_MSC_VER)
#pragma comment(linker, "/alternatename:_aligned_malloc=my_aligned_malloc")
#pragma comment(linker, "/alternatename:_aligned_free=my_aligned_free")
#pragma comment(linker, "/alternatename:_aligned_realloc=my_aligned_realloc")
    
void* my_aligned_malloc(size_t size, size_t alignment);
void  my_aligned_free(void* ptr);
void* my_aligned_realloc(void* ptr, size_t size, size_t alignment);
#endif

// C11 aligned allocation
void* aligned_alloc(size_t alignment, size_t size);

// POSIX aligned allocation
#ifdef BASED_PLATFORM_LINUX
int posix_memalign(void** ptr, size_t alignment, size_t size);
#endif
#ifdef __cplusplus
}
#endif