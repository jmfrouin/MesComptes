//
// Created by Jean-Michel Frouin on 22/10/2025.
//
//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include "Type.hpp"
#include "Account.hpp"
#include "Txn.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <optional>

namespace mc::core {

class DataStore {
public:
    explicit DataStore(const std::string& filepath);

    // Load and save
    bool load();
    bool save();

    // Types
    std::vector<Type> get_all_types() const { return types_; }
    std::optional<Type> get_type_by_id(int64_t id) const;
    int64_t add_type(const Type& type);
    bool update_type(const Type& type);
    bool remove_type(int64_t id);
    bool is_type_used(int64_t type_id) const;

    // Accounts
    std::vector<Account> get_all_accounts() const { return accounts_; }
    std::optional<Account> get_account_by_id(int64_t id) const;
    int64_t add_account(const Account& account);
    bool update_account(const Account& account);
    bool remove_account(int64_t id);

    // Transactions
    std::vector<Txn> get_all_transactions() const { return transactions_; }
    std::vector<Txn> get_transactions_by_account(int64_t account_id) const;
    std::optional<Txn> get_transaction_by_id(int64_t id) const;
    int64_t add_transaction(const Txn& txn);
    bool update_transaction(const Txn& txn);
    bool remove_transaction(int64_t id);

    // Totals
    double get_account_balance(int64_t account_id) const;
    double get_total_balance() const;

private:
    std::string filepath_;
    std::vector<Type> types_;
    std::vector<Account> accounts_;
    std::vector<Txn> transactions_;
    int64_t next_type_id_ = 1;
    int64_t next_account_id_ = 1;
    int64_t next_txn_id_ = 1;

    std::string to_json() const;
    bool from_json(const std::string& json);
};

} // namespace mc::core