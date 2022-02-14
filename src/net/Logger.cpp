#include "Logger.hpp"

#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <chrono>
#include <memory>

using namespace chauncy;

std::atomic<bool> Logger::writeToConsole_ { true };
std::atomic<bool> Logger::writeToFile_ { true };

Logger& Logger::Instance()
{
    static Logger log;
    return log;
}

bool Logger::init(std::string_view filePath, std::string_view loggerName, spdlog::level::level_enum level)
{
    if (isInited_)
        return true;

    if (!writeToFile_ && !writeToConsole_) {
        std::cout << "Initialized AN EMPTY Logger!" << std::endl;
        return true;
    }

    try {

        spdlog::flush_every(std::chrono::seconds(1));

        std::vector<spdlog::sink_ptr> sinks;
        if (writeToConsole_) {
            auto console_sinks = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(std::move(console_sinks));
        }

        if (writeToFile_) {
            auto file_sinks = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(std::string(filePath), 1024 * 1024 * 1024, 5, false);
            sinks.push_back(std::move(file_sinks));
        }

        std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(std::string(loggerName), sinks.begin(), sinks.end());
        logger->set_level(level);

        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");

        logger->set_formatter(std::move(formatter));

        logger->flush_on(spdlog::level::warn);
        spdlog::set_default_logger(logger);
    } catch (spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }

    isInited_ = true;
    return true;
}

void Logger::setLevel(spdlog::level::level_enum level)
{
    spdlog::set_level(level);
}
