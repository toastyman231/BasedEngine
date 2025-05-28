#include "pch.h"
#include "managers/logmanager.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace based::managers
{
	void LogManager::Initialize()
	{
		PROFILE_FUNCTION();
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] %v%$");

		std::vector<spdlog::sink_ptr> sinks{ consoleSink };
#ifdef BASED_CONFIG_DEBUG
		if (std::filesystem::exists("Logs/recent_log.txt"))
		{
			auto dirIter = std::filesystem::directory_iterator("Logs");

			int fileCount = std::count_if(
				begin(dirIter),
				end(dirIter),
				[](auto& entry) { return entry.is_regular_file(); }
			);

			std::string newName = "Logs/log_" + std::to_string(fileCount) + ".txt";
			std::filesystem::rename("Logs/recent_log.txt", newName);
		}

		sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs/recent_log.txt"));
#endif
		auto logger = std::make_shared<spdlog::logger>(BASED_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::trace);
		spdlog::register_logger(logger);

		logging_enabled = true;
	}

	void LogManager::Shutdown()
	{
		PROFILE_FUNCTION();
		spdlog::shutdown();
		logging_enabled = false;
	}
}