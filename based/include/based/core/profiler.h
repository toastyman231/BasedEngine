#pragma once

#define TRACY_ENABLE
#define TRACY_EXPORTS
#include "tracy/Tracy.hpp"

#define PROFILING 1

#if PROFILING
#define PROFILE_SCOPE(name) ZoneScopedN(name);
#define PROFILER_FRAME_MARK() FrameMark
#else
#define PROFILE_SCOPE(name)
#define PROFILER_FRAME_MARK()
#endif
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)