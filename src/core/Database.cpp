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
    const char* createTransactionsTable = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            libelle TEXT NOT NULL,
            somme REAL NOT NULL,
            pointee INTEGER DEFAULT 0,
            type TEXT NOT NULL,
            date_pointee TEXT
        );
    )";

    const char* createTypesTable = R"(
        CREATE TABLE IF NOT EXISTS types (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT UNIQUE NOT NULL,
            is_depense INTEGER DEFAULT 1
        );
    )";

    const char* createRecurringTable = R"(
        CREATE TABLE IF NOT EXISTS recurring_transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            libelle TEXT NOT NULL,
            somme REAL NOT NULL,
            type TEXT NOT NULL,
            recurrence_type INTEGER NOT NULL,
            start_date TEXT NOT NULL,
            end_date TEXT,
            last_executed TEXT,
            day_of_month INTEGER DEFAULT 1,
            active INTEGER DEFAULT 1
        );
    )";

    char* errMsg = nullptr;
    
    if (sqlite3_exec(mDb, createTransactionsTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Erreur création table transactions: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(mDb, createTypesTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Erreur création table types: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(mDb, createRecurringTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Erreur création table recurring_transactions: " << errMsg << std::endl;
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

    // Migration pour ajouter la colonne date_pointee si elle n'existe pas
    const char* sqlCheckDatePointee = "PRAGMA table_info(transactions);";
    bool hasDatePointee = false;

    if (sqlite3_prepare_v2(mDb, sqlCheckDatePointee, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string columnName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            if (columnName == "date_pointee") {
                hasDatePointee = true;
                break;
            }
        }
        sqlite3_finalize(stmt);
    }

    if (!hasDatePointee) {
        const char* sqlAlterDatePointee = "ALTER TABLE transactions ADD COLUMN date_pointee TEXT;";
        char* errMsg = nullptr;
        sqlite3_exec(mDb, sqlAlterDatePointee, nullptr, nullptr, &errMsg);
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
    std::string sql = "INSERT INTO transactions (date, libelle, somme, pointee, type, date_pointee) "
                      "VALUES (?, ?, ?, ?, ?, ?);";

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

    if (transaction.GetDatePointee().IsValid()) {
        sqlite3_bind_text(stmt, 6, transaction.GetDatePointee().Format("%Y-%m-%d").mb_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 6);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::UpdateTransaction(const Transaction& transaction) {
    std::string sql = "UPDATE transactions SET date=?, libelle=?, somme=?, pointee=?, type=?, date_pointee=? "
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
    if (transaction.GetDatePointee().IsValid()) {
        sqlite3_bind_text(stmt, 6, transaction.GetDatePointee().Format("%Y-%m-%d").mb_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    sqlite3_bind_int(stmt, 7, transaction.GetId());

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
    std::string sql = "SELECT id, date, libelle, somme, pointee, type, date_pointee FROM transactions ORDER BY date DESC;";

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

        Transaction trans(id, date, libelle, somme, pointee, type);

        // Récupérer la date pointée si elle existe
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            std::string datePointeeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            wxDateTime datePointee;
            datePointee.ParseFormat(datePointeeStr, "%Y-%m-%d");
            trans.SetDatePointee(datePointee);
        }

        transactions.push_back(trans);
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

bool Database::AddRecurringTransaction(const RecurringTransaction& trans) {
    const char* sql = R"(
        INSERT INTO recurring_transactions 
        (libelle, somme, type, recurrence_type, start_date, end_date, day_of_month, active)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(mDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, trans.GetLibelle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, trans.GetSomme());
    sqlite3_bind_text(stmt, 3, trans.GetType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(trans.GetRecurrence()));
    sqlite3_bind_text(stmt, 5, trans.GetStartDate().FormatISODate().mb_str(), -1, SQLITE_TRANSIENT);
    
    if (trans.GetEndDate().IsValid()) {
        sqlite3_bind_text(stmt, 6, trans.GetEndDate().FormatISODate().mb_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    
    sqlite3_bind_int(stmt, 7, trans.GetDayOfMonth());
    sqlite3_bind_int(stmt, 8, trans.IsActive() ? 1 : 0);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::UpdateRecurringTransaction(const RecurringTransaction& trans) {
    const char* sql = R"(
        UPDATE recurring_transactions 
        SET libelle = ?, somme = ?, type = ?, recurrence_type = ?,
            start_date = ?, end_date = ?, last_executed = ?, 
            day_of_month = ?, active = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(mDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, trans.GetLibelle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, trans.GetSomme());
    sqlite3_bind_text(stmt, 3, trans.GetType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(trans.GetRecurrence()));
    sqlite3_bind_text(stmt, 5, trans.GetStartDate().FormatISODate().mb_str(), -1, SQLITE_TRANSIENT);
    
    if (trans.GetEndDate().IsValid()) {
        sqlite3_bind_text(stmt, 6, trans.GetEndDate().FormatISODate().mb_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    
    if (trans.GetLastExecuted().IsValid()) {
        sqlite3_bind_text(stmt, 7, trans.GetLastExecuted().FormatISODate().mb_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 7);
    }
    
    sqlite3_bind_int(stmt, 8, trans.GetDayOfMonth());
    sqlite3_bind_int(stmt, 9, trans.IsActive() ? 1 : 0);
    sqlite3_bind_int(stmt, 10, trans.GetId());

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::DeleteRecurringTransaction(int id) {
    const char* sql = "DELETE FROM recurring_transactions WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(mDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::vector<RecurringTransaction> Database::GetAllRecurringTransactions() {
    std::vector<RecurringTransaction> transactions;
    const char* sql = "SELECT * FROM recurring_transactions ORDER BY start_date DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(mDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return transactions;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string libelle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double somme = sqlite3_column_double(stmt, 2);
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        auto recurrence = static_cast<RecurrenceType>(sqlite3_column_int(stmt, 4));
        
        wxDateTime startDate;
        startDate.ParseISODate(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        
        wxDateTime endDate;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            endDate.ParseISODate(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
        }
        
        int dayOfMonth = sqlite3_column_int(stmt, 8);
        bool active = sqlite3_column_int(stmt, 9) != 0;
        
        RecurringTransaction trans(id, libelle, somme, type, recurrence, 
                                  startDate, endDate, dayOfMonth, active);
        
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            wxDateTime lastExecuted;
            lastExecuted.ParseISODate(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
            trans.SetLastExecuted(lastExecuted);
        }
        
        transactions.push_back(trans);
    }

    sqlite3_finalize(stmt);
    return transactions;
}

int Database::ExecutePendingRecurringTransactions() {
    auto recurringTrans = GetAllRecurringTransactions();
    int count = 0;
    
    for (auto& recurring : recurringTrans) {
        if (recurring.ShouldExecuteToday()) {
            // Créer une transaction normale
            Transaction trans;
            trans.SetDate(wxDateTime::Today());
            trans.SetLibelle(recurring.GetLibelle());
            trans.SetSomme(recurring.GetSomme());
            trans.SetType(recurring.GetType());
            trans.SetPointee(false);
            
            if (AddTransaction(trans)) {
                // Mettre à jour la date de dernière exécution
                recurring.SetLastExecuted(wxDateTime::Today());
                UpdateRecurringTransaction(recurring);
                count++;
            }
        }
    }
    
    return count;
}

bool Database::ImportTransactionsFromCSV(const std::vector<std::vector<std::string>>& csvData,
                                         int dateColumn, int libelleColumn, int sommeColumn,
                                         int typeColumn, const std::string& defaultType,
                                         bool pointeeByDefault) {
    int successCount = 0;
    int errorCount = 0;

    for (const auto& row : csvData) {
        if (dateColumn >= (int)row.size() || libelleColumn >= (int)row.size() || sommeColumn >= (int)row.size()) {
            errorCount++;
            continue;
        }

        try {
            Transaction trans;

            // Parser la date
            wxDateTime date;
            wxString dateStr = wxString::FromUTF8(row[dateColumn]);

            // Essayer plusieurs formats de date
            if (!date.ParseFormat(dateStr, "%Y-%m-%d") &&
                !date.ParseFormat(dateStr, "%d/%m/%Y") &&
                !date.ParseFormat(dateStr, "%d-%m-%Y") &&
                !date.ParseFormat(dateStr, "%Y/%m/%d")) {
                errorCount++;
                continue;
            }
            trans.SetDate(date);

            // Libellé
            trans.SetLibelle(row[libelleColumn]);

            // Somme - gérer virgule et point
            std::string sommeStr = row[sommeColumn];
            std::replace(sommeStr.begin(), sommeStr.end(), ',', '.');

            // Supprimer les espaces
            sommeStr.erase(std::remove_if(sommeStr.begin(), sommeStr.end(), ::isspace), sommeStr.end());

            double somme = 0.0;
            try {
                somme = std::stod(sommeStr);
            } catch (...) {
                errorCount++;
                continue;
            }
            trans.SetSomme(std::abs(somme)); // Prendre la valeur absolue

            // Type
            std::string type = defaultType;
            if (typeColumn >= 0 && typeColumn < (int)row.size() && !row[typeColumn].empty()) {
                type = row[typeColumn];
            }
            trans.SetType(type);

            // Pointée
            trans.SetPointee(pointeeByDefault);

            if (AddTransaction(trans)) {
                successCount++;
            } else {
                errorCount++;
            }
        } catch (...) {
            errorCount++;
        }
    }

    return errorCount == 0;
}