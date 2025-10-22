//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include "core/Txn.hpp"
#include "core/Totals.hpp"
#include "db/SqliteDB.hpp"
#include <vector>
#include <optional>

namespace mc::db::dao {

class TxnDAO {
public:
    explicit TxnDAO(SqliteDB& db);

    std::optional<core::Txn> find_by_id(int64_t id);
    std::vector<core::Txn> find_by_account(int64_t account_id, int limit = -1, int offset = 0);
    int count_by_account(int64_t account_id);

    int64_t insert(const core::Txn& txn);
    bool update(const core::Txn& txn);
    bool remove(int64_t id);

    bool toggle_pointed(int64_t id);

    // Calculs de totaux
    core::Totals calculate_totals(int64_t account_id);
    int64_t get_sum_pointed(int64_t account_id);
    int64_t get_sum_all(int64_t account_id);

private:
    SqliteDB& db_;

    core::Txn row_to_txn(sqlite3_stmt* stmt);
};

} // namespace mc::db::dao