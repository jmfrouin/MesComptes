//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <string>
#include <cstdint>

namespace mc::core {

struct Type {
    int64_t id = 0;
    std::string name;

    Type() = default;
    explicit Type(std::string name_) : name(std::move(name_)) {}
};

} // namespace mc::core