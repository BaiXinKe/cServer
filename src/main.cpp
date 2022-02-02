#include "base/Setting.h"
#include "base/logging.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

int main(int, char**)
{
    SettingProfile::setSettingFilePosition("../src/settings.json");
    for (int i = 0; i < 1000; i++)
        Logger::getInstance()
            .log(spdlog::level::info, "hello msg");
}
