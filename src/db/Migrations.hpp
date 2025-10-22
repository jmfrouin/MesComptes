//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include "SqliteDB.hpp"
#include <string>
#include <vector>

namespace mc::db {

struct Migration {
    int version;
    std::string description;
    std::string sql;
};

class Migrations {
public:
    explicit Migrations(SqliteDB& db);

    bool apply_all();
    std::vector<Migration> get_pending_migrations();

private:
    SqliteDB& db_;
    std::vector<Migration> migrations_;

    void init_migrations();
    bool apply_migration(const Migration& migration);
};

} // namespace mc::db