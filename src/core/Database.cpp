//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include "Database.h"
#include <iostream>
#include <sstream>

Database::Database(const std::string& dbPath)
    : mDbPath(dbPath), mDb(nullptr) {}

Database::~Database() {
    Close();
}

bool Database::Open() {
    int rc = sqlite3_open(mDbPath.c_str(), &mDb);
    if (rc != SQLITE_OK) {
        std::cerr << "Erreur lors de l'ouverture de la base: " << sqlite3_errmsg(mDb) << std::endl;
        return false;
    }

    if (!CreateTables()) {
        return false;
    }

    InitializeDefaultTypes();
    return true;
}

void Database::Close() {
    if (mDb) {
        sqlite3_close(mDb);
        mDb = nullptr;
    }
}

bool Database::CreateTables() {
    const char* sqlTransactions = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            libelle TEXT NOT NULL,
            somme REAL NOT NULL,
            pointee INTEGER NOT NULL,
            type TEXT NOT NULL
        );
    )";

    const char* sqlTypes = R"(
        CREATE TABLE IF NOT EXISTS types (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT UNIQUE NOT NULL
        );
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(mDb, sqlTransactions, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Erreur SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    rc = sqlite3_exec(mDb, sqlTypes, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Erreur SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool Database::InitializeDefaultTypes() {
    std::vector<std::string> defaultTypes = {"CB", "CHEQUE", "VIREMENT"};

    for (const auto& type : defaultTypes) {
        std::string sql = "INSERT OR IGNORE INTO types (nom) VALUES ('" + type + "');";
        char* errMsg = nullptr;
        int rc = sqlite3_exec(mDb, sql.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            sqlite3_free(errMsg);
        }
    }
    return true;
}

bool Database::AddTransaction(const Transaction& transaction) {
    std::string sql = "INSERT INTO transactions (date, libelle, somme, pointee, type) "
                      "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    std::string dateStr = transaction.GetDate().Format("%Y-%m-%d").ToStdString();
    sqlite3_bind_text(stmt, 1, dateStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction.GetLibelle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, transaction.GetSomme());
    sqlite3_bind_int(stmt, 4, transaction.IsPointee() ? 1 : 0);
    sqlite3_bind_text(stmt, 5, transaction.GetType().c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::UpdateTransaction(const Transaction& transaction) {
    std::string sql = "UPDATE transactions SET date=?, libelle=?, somme=?, pointee=?, type=? "
                      "WHERE id=?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    std::string dateStr = transaction.GetDate().Format("%Y-%m-%d").ToStdString();
    sqlite3_bind_text(stmt, 1, dateStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction.GetLibelle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, transaction.GetSomme());
    sqlite3_bind_int(stmt, 4, transaction.IsPointee() ? 1 : 0);
    sqlite3_bind_text(stmt, 5, transaction.GetType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, transaction.GetId());

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::DeleteTransaction(int id) {
    std::string sql = "DELETE FROM transactions WHERE id=?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<Transaction> Database::GetAllTransactions() {
    std::vector<Transaction> transactions;
    std::string sql = "SELECT id, date, libelle, somme, pointee, type FROM transactions ORDER BY date DESC;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return transactions;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string dateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string libelle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        double somme = sqlite3_column_double(stmt, 3);
        bool pointee = sqlite3_column_int(stmt, 4) != 0;
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        wxDateTime date;
        date.ParseFormat(dateStr, "%Y-%m-%d");

        transactions.emplace_back(id, date, libelle, somme, pointee, type);
    }

    sqlite3_finalize(stmt);
    return transactions;
}

bool Database::AddType(const std::string& type) {
    std::string sql = "INSERT INTO types (nom) VALUES (?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::DeleteType(const std::string& type) {
    std::string sql = "DELETE FROM types WHERE nom=?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<std::string> Database::GetAllTypes() {
    std::vector<std::string> types;
    std::string sql = "SELECT nom FROM types ORDER BY nom;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return types;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        types.push_back(type);
    }

    sqlite3_finalize(stmt);
    return types;
}

double Database::GetTotalRestant() {
    std::string sql = "SELECT SUM(somme) FROM transactions;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0.0;
    }

    double total = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return total;
}

double Database::GetTotalPointee() {
    std::string sql = "SELECT SUM(somme) FROM transactions WHERE pointee=1;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0.0;
    }

    double total = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return total;
}

int Database::GetTransactionCount() {
    std::string sql = "SELECT COUNT(*) FROM transactions;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

std::string Database::GetDatabaseInfo() {
    std::ostringstream info;
    info << "Chemin de la base : " << mDbPath << "\n";
    info << "Nombre de transactions : " << GetTransactionCount() << "\n";
    info << "Total restant : " << GetTotalRestant() << " €\n";
    info << "Total pointé : " << GetTotalPointee() << " €\n";
    return info.str();
}