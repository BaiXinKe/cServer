#pragma once
#include "noncopyable.hpp"
#include <memory>
#include <spdlog/spdlog.h>

class Logger : noncopyable {
public:
    static Logger& getInstance();

    void log(spdlog::level::level_enum level, std::string_view msg);

private:
    Logger();
    ~Logger();
    class LoggerImpl;

    std::unique_ptr<LoggerImpl> impl_;
};