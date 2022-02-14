#ifndef __LOGGER_HPP
#define __LOGGER_HPP

#include <spdlog/spdlog.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <cstdlib>

namespace chauncy {

class Logger final : boost::noncopyable {
public:
    static Logger& Instance();

    bool init(std::string_view file_path = "logs/log.txt", std::string_view loggerName = "Logger",
        spdlog::level::level_enum level = spdlog::level::info);

    void setLevel(spdlog::level::level_enum level = spdlog::level::info);
    static void onlyToConsole() { writeToFile_ = false; }
    static void onlyToFile() { writeToConsole_ = false; }
    static void shutdown() { spdlog::shutdown(); }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    std::atomic<bool> isInited_ { false };
    static std::atomic<bool> writeToConsole_;
    static std::atomic<bool> writeToFile_;
};

// SPDLOG_LOGGER_XXX 内部会自行设置输出文件和函数名
#define TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__);
#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);
#define CRITICAL(...)                                                  \
    SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__); \
    abort();

// 这里真的需要？
#define LOG_LEVEL_INFO spdlog::set_level(spdlog::level::info);
#define LOG_LEVEL_DEBUG spdlog::set_level(spdlog::level::debug);
#define LOG_LEVEL_TRACE spdlog::set_level(spdlog::level::trace);
#define LOG_LEVEL_WRAN spdlog::set_level(spdlog::level::warn);
#define LOG_LEVEL_ERROR spdlog::set_level(spdlog::level::err);
#define LOG_LEVEL_CRITICAL spdlog::set_level(spdlog::level::critical);

#define LOGGER(...) Logger::Instance().init(__VA_ARGS__);
#define LOGGER_WITH_NAME(name) Logger::Instance().init("logs/log.txt", name);
#define ONLY_TO_CONSOLE Logger::onlyToConsole();
#define ONLY_TO_FILE Logger::onlyToFile();

}
#endif