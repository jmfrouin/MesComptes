//
// Created by Jean-Michel Frouin on 22/10/2025.
//
#include "SqliteDB.hpp"
#include <stdexcept>

namespace mc::db {

SqliteDB::SqliteDB(const std::string& db_path)
    : db_path_(db_path), db_(nullptr) {
}

SqliteDB::~SqliteDB() {
    close();
}

SqliteDB::SqliteDB(SqliteDB&& other) noexcept
    : db_path_(std::move(other.db_path_)), db_(other.db_) {
    other.db_ = nullptr;
}

SqliteDB& SqliteDB::operator=(SqliteDB&& other) noexcept {
    if (this != &other) {
        close();
        db_path_ = std::move(other.db_path_);
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

bool SqliteDB::open() {
    if (db_) return true;

    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        db_ = nullptr;
        return false;
    }

    // Active les clés étrangères
    execute("PRAGMA foreign_keys=ON;");
    return true;
}

void SqliteDB::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool SqliteDB::begin_transaction() {
    return execute("BEGIN TRANSACTION;");
}

bool SqliteDB::commit() {
    return execute("COMMIT;");
}

bool SqliteDB::rollback() {
    return execute("ROLLBACK;");
}

bool SqliteDB::execute(const std::string& sql) {
    if (!db_) return false;

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        if (err_msg) {
            sqlite3_free(err_msg);
        }
        return false;
    }
    return true;
}

int SqliteDB::get_user_version() {
    if (!db_) return -1;

    sqlite3_stmt* stmt = nullptr;
    int version = -1;

    if (sqlite3_prepare_v2(db_, "PRAGMA user_version;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            version = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return version;
}

bool SqliteDB::set_user_version(int version) {
    return execute("PRAGMA user_version=" + std::to_string(version) + ";");
}

std::string SqliteDB::get_last_error() const {
    if (db_) {
        return sqlite3_errmsg(db_);
    }
    return "Database not open";
}

} // namespace mc::db