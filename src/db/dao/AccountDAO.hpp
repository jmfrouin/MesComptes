//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include "core/Account.hpp"
#include "db/SqliteDB.hpp"
#include <vector>
#include <optional>

namespace mc::db::dao {

class AccountDAO {
public:
    explicit AccountDAO(SqliteDB& db);

    std::optional<core::Account> find_by_id(int64_t id);
    std::vector<core::Account> find_all();

    int64_t insert(const core::Account& account);
    bool update(const core::Account& account);
    bool remove(int64_t id);

    int64_t get_balance(int64_t account_id);

private:
    SqliteDB& db_;
};

} // namespace mc::db::dao