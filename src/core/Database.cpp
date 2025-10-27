//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include <core/Database.h>
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

    MigrateTypesTable();
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
            nom TEXT UNIQUE NOT NULL,
            is_depense INTEGER NOT NULL DEFAULT 1
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

void Database::MigrateTypesTable() {
    // Vérifier si la colonne is_depense existe déjà
    const char* sqlCheck = "PRAGMA table_info(types);";
    sqlite3_stmt* stmt;
    bool hasIsDepense = false;
    
    if (sqlite3_prepare_v2(mDb, sqlCheck, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string columnName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            if (columnName == "is_depense") {
                hasIsDepense = true;
                break;
            }
        }
        sqlite3_finalize(stmt);
    }

    // Si la colonne n'existe pas, l'ajouter
    if (!hasIsDepense) {
        const char* sqlAlter = "ALTER TABLE types ADD COLUMN is_depense INTEGER NOT NULL DEFAULT 1;";
        char* errMsg = nullptr;
        sqlite3_exec(mDb, sqlAlter, nullptr, nullptr, &errMsg);
        if (errMsg) {
            sqlite3_free(errMsg);
        }
    }
}

bool Database::InitializeDefaultTypes() {
    // CB et CHEQUE sont des dépenses (true = 1)
    // VIREMENT peut être une recette (false = 0)
    std::vector<std::pair<std::string, bool>> defaultTypes = {
        {"CB", true},
        {"CHEQUE", true},
        {"VIREMENT", false}
    };
    
    for (const auto& [type, isDepense] : defaultTypes) {
        std::string sql = "INSERT OR IGNORE INTO types (nom, is_depense) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, isDepense ? 1 : 0);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
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

Transaction Database::GetTransaction(int id) {
    std::string sql = "SELECT id, date, libelle, somme, pointee, type FROM transactions WHERE id=?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return Transaction();
    }

    sqlite3_bind_int(stmt, 1, id);

    Transaction trans;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int transId = sqlite3_column_int(stmt, 0);
        std::string dateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string libelle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        double somme = sqlite3_column_double(stmt, 3);
        bool pointee = sqlite3_column_int(stmt, 4) != 0;
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        wxDateTime date;
        date.ParseFormat(dateStr, "%Y-%m-%d");

        trans = Transaction(transId, date, libelle, somme, pointee, type);
    }

    sqlite3_finalize(stmt);
    return trans;
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

bool Database::AddType(const std::string& type, bool isDepense) {
    std::string sql = "INSERT INTO types (nom, is_depense) VALUES (?, ?);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, isDepense ? 1 : 0);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::UpdateType(const std::string& type, bool isDepense) {
    std::string sql = "UPDATE types SET is_depense=? WHERE nom=?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, isDepense ? 1 : 0);
    sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_TRANSIENT);
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

std::vector<TransactionType> Database::GetAllTypes() {
    std::vector<TransactionType> types;
    std::string sql = "SELECT nom, is_depense FROM types ORDER BY nom;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return types;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string nom = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        bool isDepense = sqlite3_column_int(stmt, 1) != 0;
        types.emplace_back(nom, isDepense);
    }

    sqlite3_finalize(stmt);
    return types;
}

bool Database::IsTypeDepense(const std::string& type) {
    std::string sql = "SELECT is_depense FROM types WHERE nom=?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return true;  // Par défaut, considérer comme dépense
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_TRANSIENT);
    
    bool isDepense = true;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        isDepense = sqlite3_column_int(stmt, 0) != 0;
    }

    sqlite3_finalize(stmt);
    return isDepense;
}

double Database::GetTotalRestant() {
    std::string sql = R"(
        SELECT SUM(
            CASE 
                WHEN types.is_depense = 1 THEN -transactions.somme
                ELSE transactions.somme
            END
        )
        FROM transactions
        JOIN types ON transactions.type = types.nom;
    )";
    
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
    std::string sql = R"(
        SELECT SUM(
            CASE 
                WHEN types.is_depense = 1 THEN -transactions.somme
                ELSE transactions.somme
            END
        )
        FROM transactions
        JOIN types ON transactions.type = types.nom
        WHERE transactions.pointee = 1;
    )";
    
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