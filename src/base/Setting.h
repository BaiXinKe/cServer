#pragma once

#include "../base/noncopyable.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>

namespace SettingProfile {

class Setting : noncopyable {
public:
    static const Setting& getInstance();
    const nlohmann::json* operator->() const;

private:
    explicit Setting(std::string_view setting_file_path);
    ~Setting() = default;

    nlohmann::json json_;
};

void setSettingFilePosition(std::string_view setting_file_path);

}