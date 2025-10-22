//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "TypeDAO.hpp"

namespace mc::db::dao {

TypeDAO::TypeDAO(SqliteDB& db) : db_(db) {}

std::optional<core::Type> TypeDAO::find_by_id(int64_t id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM type WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int64(stmt, 1, id);

    std::optional<core::Type> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        core::Type type;
        type.id = sqlite3_column_int64(stmt, 0);
        type.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result = type;
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<core::Type> TypeDAO::find_all() {
    std::vector<core::Type> types;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, name FROM type ORDER BY name";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return types;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        core::Type type;
        type.id = sqlite3_column_int64(stmt, 0);
        type.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        types.push_back(type);
    }

    sqlite3_finalize(stmt);
    return types;
}

int64_t TypeDAO::insert(const core::Type& type) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO type (name) VALUES (?)";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, type.name.c_str(), -1, SQLITE_TRANSIENT);

    int64_t id = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        id = sqlite3_last_insert_rowid(db_.handle());
    }

    sqlite3_finalize(stmt);
    return id;
}

bool TypeDAO::update(const core::Type& type) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE type SET name = ? WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, type.id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool TypeDAO::remove(int64_t id) {
    // Vérifier si le type est utilisé
    if (is_used(id)) {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM type WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int64(stmt, 1, id);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool TypeDAO::is_used(int64_t id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT COUNT(*) FROM txn WHERE type_id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return true; // En cas d'erreur, on considère qu'il est utilisé (sécurité)
    }

    sqlite3_bind_int64(stmt, 1, id);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0;
}

} // namespace mc::db::dao