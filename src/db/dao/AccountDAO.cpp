//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "AccountDAO.hpp"
#include <chrono>

namespace mc::db::dao {

AccountDAO::AccountDAO(SqliteDB& db) : db_(db) {}

std::optional<core::Account> AccountDAO::find_by_id(int64_t id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name, iban, created_at FROM account WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int64(stmt, 1, id);

    std::optional<core::Account> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        core::Account account;
        account.id = sqlite3_column_int64(stmt, 0);
        account.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            account.iban = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }

        account.created_at = sqlite3_column_int64(stmt, 3);
        result = account;
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<core::Account> AccountDAO::find_all() {
    std::vector<core::Account> accounts;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name, iban, created_at FROM account ORDER BY name";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return accounts;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        core::Account account;
        account.id = sqlite3_column_int64(stmt, 0);
        account.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            account.iban = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }

        account.created_at = sqlite3_column_int64(stmt, 3);
        accounts.push_back(account);
    }

    sqlite3_finalize(stmt);
    return accounts;
}

int64_t AccountDAO::insert(const core::Account& account) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO account (name, iban, created_at) VALUES (?, ?, ?)";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, account.name.c_str(), -1, SQLITE_TRANSIENT);

    if (account.iban) {
        sqlite3_bind_text(stmt, 2, account.iban->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 2);
    }

    int64_t created_at = account.created_at;
    if (created_at == 0) {
        created_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    sqlite3_bind_int64(stmt, 3, created_at);

    int64_t id = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        id = sqlite3_last_insert_rowid(db_.handle());
    }

    sqlite3_finalize(stmt);
    return id;
}

bool AccountDAO::update(const core::Account& account) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE account SET name = ?, iban = ? WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, account.name.c_str(), -1, SQLITE_TRANSIENT);

    if (account.iban) {
        sqlite3_bind_text(stmt, 2, account.iban->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 2);
    }

    sqlite3_bind_int64(stmt, 3, account.id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool AccountDAO::remove(int64_t id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM account WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int64(stmt, 1, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

int64_t AccountDAO::get_balance(int64_t account_id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT COALESCE(SUM(amount_cents), 0) FROM txn WHERE account_id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int64(stmt, 1, account_id);

    int64_t balance = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        balance = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return balance;
}

} // namespace mc::db::dao