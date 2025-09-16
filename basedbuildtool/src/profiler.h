#pragma once

#define TRACY_ENABLE
//#define TRACY_EXPORTS
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#define BASED_CONFIG_ENABLE_PROFILING
#ifdef BASED_CONFIG_ENABLE_PROFILING
#define PROFILING 1
#else
#define PROFILING 0
#endif

#if PROFILING
#define PROFILE_SCOPE(name) ZoneScopedN(name);
#define PROFILE_ZONE(zoneVar, name, active) TracyCZoneN(zoneVar, name, active);
#define PROFILE_ZONE_END(zoneVar) TracyCZoneEnd(zoneVar);
#define PROFILE_SCOPE_TEXT(fmt, ...) ZoneTextF(fmt, __VA_ARGS__);
#define PROFILE_ZONE_TEXT(var, text, size) TracyCZoneText(var, text, size);
#define PROFILER_FRAME_MARK() FrameMark
#define PROFILER_SET_THREAD_NAME(name) tracy::SetThreadName(name);
#else
#define PROFILE_SCOPE(name)
#define PROFILE_ZONE(zoneVar, name, active)
#define PROFILE_ZONE_END(zoneVar)
#define PROFILE_SCOPE_TEXT(fmt, ...)
#define PROFILE_ZONE_TEXT(var, text, size)
#define PROFILER_FRAME_MARK()
#define PROFILER_SET_THREAD_NAME(name)
#endif
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)