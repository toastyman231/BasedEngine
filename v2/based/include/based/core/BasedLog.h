#pragma once

#include <spdlog/spdlog.h>

#include "BasedTypes.h"
#include "LogManager.h"

#define BASED_DEFAULT_LOGGER_NAME "basedlogger"

#if defined(BASED_PLATFORM_WINDOWS)
#define BASED_BREAK __debugbreak();
#elif defined(BASED_PLATFORM_MAC)
#define BASED_BREAK __builtin_debugtrap();
#else
#define BASED_BREAK __builtin_trap();
#endif

#ifndef BASED_CONFIG_RELEASE
#define BASED_TRACE(...) do{if (g_bLoggingEnabled && based::g_pDefaultLogger != nullptr) {based::g_pDefaultLogger->trace(__VA_ARGS__);}} while(0)
#define BASED_DEBUG(...) do{if (g_bLoggingEnabled && based::g_pDefaultLogger != nullptr) {based::g_pDefaultLogger->debug(__VA_ARGS__);}} while(0)
#define BASED_INFO(...) do{if (g_bLoggingEnabled && based::g_pDefaultLogger != nullptr)  {based::g_pDefaultLogger->info(__VA_ARGS__);}} while(0)
#define BASED_WARN(...) do{if (g_bLoggingEnabled && based::g_pDefaultLogger != nullptr)  {based::g_pDefaultLogger->warn(__VA_ARGS__);}} while(0)
#define BASED_ERROR(...) do{if (g_bLoggingEnabled && based::g_pDefaultLogger != nullptr) {based::g_pDefaultLogger->error(__VA_ARGS__);}} while(0)
#define BASED_FATAL(...) do{if (g_bLoggingEnabled && based::g_pDefaultLogger != nullptr) {based::g_pDefaultLogger->critical(__VA_ARGS__);}} while(0)
#define BASED_ASSERT(x, msg) do{if ((x)) {} else {BASED_FATAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__); BASED_BREAK}} while(0)
#define BASED_ASSERT_FMT(x, fmt, ...) do{if ((x)) {} else {BASED_FATAL("ASSERT - {}\n\tin file: {}\n\ton line: {}\n\t" fmt, #x, __FILE__, __LINE__, ##__VA_ARGS__);}} while(0)

// For when we don't have logging (very early setup)
#define BASED_SIMPLE_ASSERT(x, fmt, ...) do {if ((x)) {} else {printf("%s", std::format("ASSERT - {}\n\tin file: {}\n\ton line: {}\n\t" fmt, #x, __FILE__, __LINE__, ##__VA_ARGS__).c_str()); BASED_BREAK}} while(0)

#else
// Disable logging for release builds
#define BASED_TRACE(...) (void)0
#define BASED_DEBUG(...) (void)0
#define BASED_INFO(...) (void)0
#define BASED_WARN(...) (void)0
#define BASED_ERROR(...) (void)0
#define BASED_FATAL(...) (void)0
#define BASED_ASSERT(x, msg) (void)0
#define BASED_ASSERT_FMT(x, fmt, ...) (void)0
#define BASED_SIMPLE_ASSERT(x, fmt, ...) (void)0
#endif

inline bool g_bLoggingEnabled;