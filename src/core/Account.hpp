//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <string>
#include <cstdint>
#include <optional>

namespace mc::core {

struct Account {
    int64_t id = 0;
    std::string name;
    std::optional<std::string> iban;
    int64_t created_at = 0;  // epoch seconds

    Account() = default;
    Account(std::string name_) : name(std::move(name_)) {}
};

} // namespace mc::core