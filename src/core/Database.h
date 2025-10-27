//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include "Transaction.h"

class Database {
public:
    Database(const std::string& dbPath);
    ~Database();

    bool Open();
    void Close();
    bool IsOpen() const { return mDb != nullptr; }

    // Opérations sur les transactions
    bool AddTransaction(const Transaction& transaction);
    bool UpdateTransaction(const Transaction& transaction);
    bool DeleteTransaction(int id);
    std::vector<Transaction> GetAllTransactions();
    Transaction GetTransaction(int id);

    // Opérations sur les types
    bool AddType(const std::string& type);
    bool DeleteType(const std::string& type);
    std::vector<std::string> GetAllTypes();

    // Statistiques
    double GetTotalRestant();
    double GetTotalPointee();
    int GetTransactionCount();
    std::string GetDatabaseInfo();

private:
    bool CreateTables();
    bool InitializeDefaultTypes();

    std::string mDbPath;
    sqlite3* mDb;
};

#endif // DATABASE_H