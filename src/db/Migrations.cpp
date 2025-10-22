//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "Migrations.hpp"
#include <stdexcept>

namespace mc::db {

Migrations::Migrations(SqliteDB& db) : db_(db) {
    init_migrations();
}

void Migrations::init_migrations() {
    // Migration 1: Schéma initial
    migrations_.push_back({1, "Initial schema", R"(
CREATE TABLE IF NOT EXISTS account (
  id INTEGER PRIMARY KEY,
  name TEXT NOT NULL UNIQUE,
  iban TEXT,
  created_at INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS type (
  id INTEGER PRIMARY KEY,
  name TEXT NOT NULL UNIQUE CHECK (length(name)>0)
);

CREATE TABLE IF NOT EXISTS txn (
  id INTEGER PRIMARY KEY,
  account_id INTEGER NOT NULL REFERENCES account(id) ON DELETE CASCADE,
  op_date INTEGER NOT NULL,
  label TEXT NOT NULL,
  amount_cents INTEGER NOT NULL,
  pointed INTEGER NOT NULL DEFAULT 0,
  type_id INTEGER REFERENCES type(id),
  created_at INTEGER NOT NULL,
  updated_at INTEGER
);

CREATE TABLE IF NOT EXISTS settings (
  key TEXT PRIMARY KEY,
  value TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_txn_account_date ON txn(account_id, op_date DESC);
CREATE INDEX IF NOT EXISTS idx_txn_pointed ON txn(account_id, pointed);
CREATE INDEX IF NOT EXISTS idx_txn_type ON txn(type_id);

INSERT OR IGNORE INTO type(id,name) VALUES
  (1,'CB'),(2,'CHEQUE'),(3,'VIREMENT');
    )"});
}

bool Migrations::apply_all() {
    int current_version = db_.get_user_version();
    if (current_version < 0) {
        return false;
    }

    bool all_success = true;
    for (const auto& migration : migrations_) {
        if (migration.version > current_version) {
            if (!apply_migration(migration)) {
                all_success = false;
                break;
            }
        }
    }

    return all_success;
}

std::vector<Migration> Migrations::get_pending_migrations() {
    std::vector<Migration> pending;
    int current_version = db_.get_user_version();

    for (const auto& migration : migrations_) {
        if (migration.version > current_version) {
            pending.push_back(migration);
        }
    }

    return pending;
}

bool Migrations::apply_migration(const Migration& migration) {
    if (!db_.begin_transaction()) {
        return false;
    }

    if (!db_.execute(migration.sql)) {
        db_.rollback();
        return false;
    }

    if (!db_.set_user_version(migration.version)) {
        db_.rollback();
        return false;
    }

    return db_.commit();
}

} // namespace mc::db