#include "pch.h"
#include "core/LogManager.h"

#include "core/BasedLog.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#define NUM_SINKS 2

namespace based
{
    void LogManager::Initialize()
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] %v%$");

        std::array<spdlog::sink_ptr, NUM_SINKS> sinks{ consoleSink };
        const char* pRecentLogPath = "Logs/recent_log.txt";
#ifdef BASED_CONFIG_DEBUG
        if (std::filesystem::exists(pRecentLogPath))
        {
            auto dirIter = std::filesystem::directory_iterator("Logs");

            auto fileCount = std::count_if(
                begin(dirIter),
                end(dirIter),
                [](auto& entry) { return entry.is_regular_file(); }
            );

            std::string newName = "Logs/log_" + std::to_string(fileCount) + ".txt";
            std::filesystem::rename(pRecentLogPath, newName);
        }

        sinks[1] = std::make_shared<spdlog::sinks::basic_file_sink_mt>(pRecentLogPath);
#endif
        auto logger = std::make_shared<spdlog::logger>(BASED_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
        spdlog::register_logger(logger);
        g_pDefaultLogger = spdlog::get(BASED_DEFAULT_LOGGER_NAME).get();

        g_bLoggingEnabled = true;
        BASED_INFO("Finished setting up LogManager!");
    }

    void LogManager::Shutdown()
    {
        spdlog::shutdown();
        g_bLoggingEnabled = false;
    }
}
