//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include "core/Type.hpp"
#include "db/SqliteDB.hpp"
#include <vector>
#include <optional>

namespace mc::db::dao {

class TypeDAO {
public:
    explicit TypeDAO(SqliteDB& db);

    std::optional<core::Type> find_by_id(int64_t id);
    std::vector<core::Type> find_all();

    int64_t insert(const core::Type& type);
    bool update(const core::Type& type);
    bool remove(int64_t id);

    bool is_used(int64_t id);

private:
    SqliteDB& db_;
};

} // namespace mc::db::dao