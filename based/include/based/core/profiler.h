#pragma once

#define TRACY_ENABLE
#define TRACY_EXPORTS
#include "tracy/Tracy.hpp"

#ifdef BASED_CONFIG_ENABLE_PROFILING
#define PROFILING 1
#else
#define PROFILING 0
#endif

#if PROFILING
#define PROFILE_SCOPE(name) ZoneScopedN(name);
#define PROFILER_FRAME_MARK() FrameMark
#else
#define PROFILE_SCOPE(name)
#define PROFILER_FRAME_MARK()
#endif
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)