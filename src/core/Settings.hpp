//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <string>
#include <optional>

namespace mc::core {

struct Settings {
    std::string key;
    std::string value;

    Settings() = default;
    Settings(std::string key_, std::string value_)
        : key(std::move(key_)), value(std::move(value_)) {}
};

} // namespace mc::core