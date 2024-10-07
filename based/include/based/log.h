#pragma once

#include "spdlog/spdlog.h"

#define BASED_DEFAULT_LOGGER_NAME "basedlogger"

#if defined(BASED_PLATFORM_WINDOWS)
#define BASED_BREAK __debugbreak();
#elif defined(BASED_PLATFORM_MAC)
#define BASED_BREAK __builtin_debugtrap();
#else
#define BASED_BREAK __builtin_trap();
#endif

#ifndef BASED_CONFIG_RELEASE
#define BASED_TRACE(...) if (logging_enabled && spdlog::get(BASED_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(BASED_DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__);}
#define BASED_DEBUG(...) if (logging_enabled && spdlog::get(BASED_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(BASED_DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__);}
#define BASED_INFO(...) if (logging_enabled && spdlog::get(BASED_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(BASED_DEFAULT_LOGGER_NAME)->info(__VA_ARGS__);}
#define BASED_WARN(...) if (logging_enabled && spdlog::get(BASED_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(BASED_DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__);}
#define BASED_ERROR(...) if (logging_enabled && spdlog::get(BASED_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(BASED_DEFAULT_LOGGER_NAME)->error(__VA_ARGS__);}
#define BASED_FATAL(...) if (logging_enabled && spdlog::get(BASED_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(BASED_DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__);}
#define BASED_ASSERT(x, msg) if ((x)) {} else {BASED_FATAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__); BASED_BREAK}
#else
// Disable logging for release builds
#define BASED_TRACE(...) (void)0
#define BASED_DEBUG(...) (void)0
#define BASED_INFO(...) (void)0
#define BASED_WARN(...) (void)0
#define BASED_ERROR(...) (void)0
#define BASED_FATAL(...) (void)0
#define BASED_ASSERT(x, msg) (void)0
#endif

extern bool logging_enabled;