//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "TxnDAO.hpp"
#include <chrono>

namespace mc::db::dao {

    TxnDAO::TxnDAO(SqliteDB& db) : db_(db) {}

    core::Txn TxnDAO::row_to_txn(sqlite3_stmt* stmt) {
        core::Txn txn;
        txn.id = sqlite3_column_int64(stmt, 0);
        txn.account_id = sqlite3_column_int64(stmt, 1);
        txn.op_date = sqlite3_column_int64(stmt, 2);
        txn.label = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        txn.amount_cents = sqlite3_column_int64(stmt, 4);
        txn.pointed = sqlite3_column_int(stmt, 5) != 0;

        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            txn.type_id = sqlite3_column_int64(stmt, 6);
        }

        txn.created_at = sqlite3_column_int64(stmt, 7);

        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
            txn.updated_at = sqlite3_column_int64(stmt, 8);
        }

        return txn;
    }

    std::optional<core::Txn> TxnDAO::find_by_id(int64_t id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT id, account_id, op_date, label, amount_cents, pointed, "
                         "type_id, created_at, updated_at FROM txn WHERE id = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return std::nullopt;
        }

        sqlite3_bind_int64(stmt, 1, id);

        std::optional<core::Txn> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = row_to_txn(stmt);
        }

        sqlite3_finalize(stmt);
        return result;
    }

    std::vector<core::Txn> TxnDAO::find_by_account(int64_t account_id, int limit, int offset) {
        std::vector<core::Txn> txns;
        sqlite3_stmt* stmt = nullptr;

        std::string sql = "SELECT id, account_id, op_date, label, amount_cents, pointed, "
                         "type_id, created_at, updated_at FROM txn WHERE account_id = ? "
                         "ORDER BY op_date DESC, id DESC";

        if (limit > 0) {
            sql += " LIMIT ? OFFSET ?";
        }

        if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return txns;
        }

        sqlite3_bind_int64(stmt, 1, account_id);

        if (limit > 0) {
            sqlite3_bind_int(stmt, 2, limit);
            sqlite3_bind_int(stmt, 3, offset);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            txns.push_back(row_to_txn(stmt));
        }

        sqlite3_finalize(stmt);
        return txns;
    }

    int TxnDAO::count_by_account(int64_t account_id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT COUNT(*) FROM txn WHERE account_id = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }

        sqlite3_bind_int64(stmt, 1, account_id);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count;
    }

    int64_t TxnDAO::insert(const core::Txn& txn) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT INTO txn (account_id, op_date, label, amount_cents, "
                         "pointed, type_id, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }

        sqlite3_bind_int64(stmt, 1, txn.account_id);
        sqlite3_bind_int64(stmt, 2, txn.op_date);
        sqlite3_bind_text(stmt, 3, txn.label.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 4, txn.amount_cents);
        sqlite3_bind_int(stmt, 5, txn.pointed ? 1 : 0);

        if (txn.type_id) {
            sqlite3_bind_int64(stmt, 6, *txn.type_id);
        } else {
            sqlite3_bind_null(stmt, 6);
        }

        int64_t created_at = txn.created_at;
        if (created_at == 0) {
            created_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        sqlite3_bind_int64(stmt, 7, created_at);

        int64_t id = -1;
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            id = sqlite3_last_insert_rowid(db_.handle());
        }

        sqlite3_finalize(stmt);
        return id;
    }

    bool TxnDAO::update(const core::Txn& txn) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "UPDATE txn SET op_date = ?, label = ?, amount_cents = ?, "
                         "pointed = ?, type_id = ?, updated_at = ? WHERE id = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int64(stmt, 1, txn.op_date);
        sqlite3_bind_text(stmt, 2, txn.label.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, txn.amount_cents);
        sqlite3_bind_int(stmt, 4, txn.pointed ? 1 : 0);

        if (txn.type_id) {
            sqlite3_bind_int64(stmt, 5, *txn.type_id);
        } else {
            sqlite3_bind_null(stmt, 5);
        }

        int64_t updated_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        sqlite3_bind_int64(stmt, 6, updated_at);
        sqlite3_bind_int64(stmt, 7, txn.id);

        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }

    bool TxnDAO::remove(int64_t id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "DELETE FROM txn WHERE id = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int64(stmt, 1, id);

        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }

    bool TxnDAO::toggle_pointed(int64_t id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "UPDATE txn SET pointed = NOT pointed, updated_at = ? WHERE id = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        int64_t updated_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        sqlite3_bind_int64(stmt, 1, updated_at);
        sqlite3_bind_int64(stmt, 2, id);

        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }

    core::Totals TxnDAO::calculate_totals(int64_t account_id) {
        core::Totals totals;

        totals.restant = get_sum_all(account_id);
        totals.somme_pointee = get_sum_pointed(account_id);

        // Récupérer somme_en_ligne depuis settings
        sqlite3_stmt* stmt = nullptr;
        std::string key = "online_sum:" + std::to_string(account_id);
        const char* sql = "SELECT value FROM settings WHERE key = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                totals.somme_en_ligne = std::stoll(value);
            }
            sqlite3_finalize(stmt);
        }

        totals.calculate();
        return totals;
    }

    int64_t TxnDAO::get_sum_pointed(int64_t account_id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT COALESCE(SUM(amount_cents), 0) FROM txn "
                         "WHERE account_id = ? AND pointed = 1";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }

        sqlite3_bind_int64(stmt, 1, account_id);

        int64_t sum = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sum = sqlite3_column_int64(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return sum;
    }

    int64_t TxnDAO::get_sum_all(int64_t account_id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT COALESCE(SUM(amount_cents), 0) FROM txn WHERE account_id = ?";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }

        sqlite3_bind_int64(stmt, 1, account_id);

        int64_t sum = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sum = sqlite3_column_int64(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return sum;
    }

} // namespace mc::db::dao#include "TxnDAO.hpp"
