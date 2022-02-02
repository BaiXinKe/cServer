#include "Setting.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string_view>

namespace SettingProfile {
static std::string_view settingFilePosition { "settings.json" };

void setSettingFilePosition(std::string_view setting_file_path)
{
    settingFilePosition = setting_file_path;
}

Setting::Setting(std::string_view setting_file_path)
    : json_ {}
{
    std::ifstream setting_file(setting_file_path.data());
    if (!setting_file.is_open())
        throw std::runtime_error { "setting file was not be opened " + std::string(setting_file_path) };
    setting_file >> json_;
}

const Setting& Setting::getInstance()
{
    static Setting setting { settingFilePosition };
    return setting;
}

const nlohmann::json* Setting::operator->() const
{
    return &json_;
}
}