//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <string>
#include <memory>
#include <vector>
#include <sqlite3.h>

namespace mc::db {

class SqliteDB {
public:
    explicit SqliteDB(const std::string& db_path);
    ~SqliteDB();

    // Interdit la copie
    SqliteDB(const SqliteDB&) = delete;
    SqliteDB& operator=(const SqliteDB&) = delete;

    // Autorise le déplacement
    SqliteDB(SqliteDB&&) noexcept;
    SqliteDB& operator=(SqliteDB&&) noexcept;

    bool open();
    void close();
    bool is_open() const { return db_ != nullptr; }

    sqlite3* handle() { return db_; }
    const std::string& path() const { return db_path_; }

    // Transactions
    bool begin_transaction();
    bool commit();
    bool rollback();

    // Exécution simple
    bool execute(const std::string& sql);

    // Lecture de la version du schéma
    int get_user_version();
    bool set_user_version(int version);

    std::string get_last_error() const;

private:
    std::string db_path_;
    sqlite3* db_ = nullptr;
};

} // namespace mc::db