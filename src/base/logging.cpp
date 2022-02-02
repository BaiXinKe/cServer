#include "logging.hpp"
#include "Setting.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>

class Logger::LoggerImpl {
public:
    LoggerImpl(const std::string& server_name, int64_t logfileSize, int32_t rotateFileNumber)
        : rotatSinks_ { std::make_shared<spdlog::sinks::rotating_file_sink_st>(server_name, logfileSize, rotateFileNumber) }
        , logger_ { "defaultLogger", rotatSinks_ }
    {
    }

    void log(spdlog::level::level_enum level, std::string_view msg);

private:
    std::shared_ptr<spdlog::sinks::rotating_file_sink_st> rotatSinks_;
    spdlog::logger logger_;
};

void Logger::LoggerImpl::log(spdlog::level::level_enum level, std::string_view msg)
{
    logger_.log(level, msg);
    if (level >= spdlog::level::level_enum::err)
        std::abort();
}

Logger&
Logger::getInstance()
{
    static Logger logger;
    return logger;
}

static int64_t parse_size(std::string logger_size_per_file)
{
    char symbol = logger_size_per_file.back();
    logger_size_per_file.pop_back();
    int64_t fileSize = std::stoll(logger_size_per_file);

    constexpr int32_t Mbit = 1024 * 1024;
    constexpr int32_t Gbit = 1024 * Mbit;

    switch (symbol) {
    case 'G':
        fileSize *= Gbit;
        break;
    case 'M':
    default:
        fileSize *= Mbit;
        break;
    }

    return fileSize;
}

Logger::Logger()
    : impl_ { nullptr }
{
    using namespace SettingProfile;
    auto& setting = Setting::getInstance();
    std::string logger_name = setting->at("logger").at("name");
    std::string logger_size_per_file = setting->at("logger").at("maxSizePerLogFile");
    std::size_t logger_max_file_number = setting->at("logger").at("maxFileNumber");

    int64_t parsed_size_per_file = parse_size(logger_size_per_file);
    if (parsed_size_per_file <= 0)
        throw std::runtime_error { "bad settings fmt of logger_size_per_file" };

    impl_ = std::make_unique<LoggerImpl>(logger_name, parsed_size_per_file, logger_max_file_number);
}

void Logger::log(spdlog::level::level_enum level, std::string_view msg)
{
    impl_->log(level, msg);
}

Logger::~Logger()
{
}