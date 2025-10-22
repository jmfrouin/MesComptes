//
// Created by Jean-Michel Frouin on 22/10/2025.
//

//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "DataStore.hpp"
#include <sstream>
#include <chrono>

namespace mc::core {

DataStore::DataStore(const std::string& filepath)
    : filepath_(filepath) {
}

bool DataStore::load() {
    std::ifstream file(filepath_);
    if (!file.is_open()) {
        // File doesn't exist yet, initialize with empty data
        return true;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return from_json(buffer.str());
}

bool DataStore::save() {
    std::ofstream file(filepath_);
    if (!file.is_open()) {
        return false;
    }

    file << to_json();
    return file.good();
}

// Types
std::optional<Type> DataStore::get_type_by_id(int64_t id) const {
    auto it = std::find_if(types_.begin(), types_.end(),
        [id](const Type& t) { return t.id == id; });
    if (it != types_.end()) {
        return *it;
    }
    return std::nullopt;
}

int64_t DataStore::add_type(const Type& type) {
    // Check for duplicate names
    auto it = std::find_if(types_.begin(), types_.end(),
        [&type](const Type& t) { return t.name == type.name; });
    if (it != types_.end()) {
        return -1; // Duplicate
    }

    Type new_type = type;
    new_type.id = next_type_id_++;
    types_.push_back(new_type);
    save();
    return new_type.id;
}

bool DataStore::update_type(const Type& type) {
    auto it = std::find_if(types_.begin(), types_.end(),
        [&type](const Type& t) { return t.id == type.id; });
    if (it != types_.end()) {
        *it = type;
        save();
        return true;
    }
    return false;
}

bool DataStore::remove_type(int64_t id) {
    auto it = std::remove_if(types_.begin(), types_.end(),
        [id](const Type& t) { return t.id == id; });
    if (it != types_.end()) {
        types_.erase(it, types_.end());
        save();
        return true;
    }
    return false;
}

bool DataStore::is_type_used(int64_t type_id) const {
    return std::any_of(transactions_.begin(), transactions_.end(),
        [type_id](const Txn& txn) { return txn.type_id == type_id; });
}

// Accounts
std::optional<Account> DataStore::get_account_by_id(int64_t id) const {
    auto it = std::find_if(accounts_.begin(), accounts_.end(),
        [id](const Account& a) { return a.id == id; });
    if (it != accounts_.end()) {
        return *it;
    }
    return std::nullopt;
}

int64_t DataStore::add_account(const Account& account) {
    Account new_account = account;
    new_account.id = next_account_id_++;
    new_account.created_at = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    accounts_.push_back(new_account);
    save();
    return new_account.id;
}

bool DataStore::update_account(const Account& account) {
    auto it = std::find_if(accounts_.begin(), accounts_.end(),
        [&account](const Account& a) { return a.id == account.id; });
    if (it != accounts_.end()) {
        *it = account;
        save();
        return true;
    }
    return false;
}

bool DataStore::remove_account(int64_t id) {
    auto it = std::remove_if(accounts_.begin(), accounts_.end(),
        [id](const Account& a) { return a.id == id; });
    if (it != accounts_.end()) {
        accounts_.erase(it, accounts_.end());
        save();
        return true;
    }
    return false;
}

// Transactions
std::vector<Txn> DataStore::get_transactions_by_account(int64_t account_id) const {
    std::vector<Txn> result;
    std::copy_if(transactions_.begin(), transactions_.end(),
        std::back_inserter(result),
        [account_id](const Txn& txn) { return txn.account_id == account_id; });
    return result;
}

std::optional<Txn> DataStore::get_transaction_by_id(int64_t id) const {
    auto it = std::find_if(transactions_.begin(), transactions_.end(),
        [id](const Txn& t) { return t.id == id; });
    if (it != transactions_.end()) {
        return *it;
    }
    return std::nullopt;
}

int64_t DataStore::add_transaction(const Txn& txn) {
    Txn new_txn = txn;
    new_txn.id = next_txn_id_++;
    if (new_txn.op_date == 0) {
        new_txn.op_date = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
    }
    transactions_.push_back(new_txn);
    save();
    return new_txn.id;
}

bool DataStore::update_transaction(const Txn& txn) {
    auto it = std::find_if(transactions_.begin(), transactions_.end(),
        [&txn](const Txn& t) { return t.id == txn.id; });
    if (it != transactions_.end()) {
        *it = txn;
        save();
        return true;
    }
    return false;
}

bool DataStore::remove_transaction(int64_t id) {
    auto it = std::remove_if(transactions_.begin(), transactions_.end(),
        [id](const Txn& t) { return t.id == id; });
    if (it != transactions_.end()) {
        transactions_.erase(it, transactions_.end());
        save();
        return true;
    }
    return false;
}

// Totals
double DataStore::get_account_balance(int64_t account_id) const {
    double balance = 0.0;
    for (const auto& txn : transactions_) {
        if (txn.account_id == account_id) {
            balance += txn.amount_cents;
        }
    }
    return balance;
}

double DataStore::get_total_balance() const {
    double total = 0.0;
    for (const auto& account : accounts_) {
        total += get_account_balance(account.id);
    }
    return total;
}

// JSON Serialization (simple format)
std::string DataStore::to_json() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"next_type_id\": " << next_type_id_ << ",\n";
    oss << "  \"next_account_id\": " << next_account_id_ << ",\n";
    oss << "  \"next_txn_id\": " << next_txn_id_ << ",\n";

    // Types
    oss << "  \"types\": [\n";
    for (size_t i = 0; i < types_.size(); ++i) {
        const auto& type = types_[i];
        oss << "    {\"id\": " << type.id << ", \"name\": \"" << type.name << "\"}";
        if (i < types_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    // Accounts
    oss << "  \"accounts\": [\n";
    for (size_t i = 0; i < accounts_.size(); ++i) {
        const auto& acc = accounts_[i];
        oss << "    {\"id\": " << acc.id << ", \"name\": \"" << acc.name << "\", ";
        oss << "\"iban\": \"" << (acc.iban ? *acc.iban : "") << "\", ";
        oss << "\"created_at\": " << acc.created_at << "}";
        if (i < accounts_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    // Transactions
    oss << "  \"transactions\": [\n";
    for (size_t i = 0; i < transactions_.size(); ++i) {
        const auto& txn = transactions_[i];
        oss << "    {\"id\": " << txn.id << ", \"account_id\": " << txn.account_id << ", ";
        oss << "\"type_id\": " << (txn.type_id.has_value() ? std::to_string(txn.type_id.value()) : "null") << ", \"date\": " << txn.op_date << ", ";
        oss << "\"amount\": " << txn.amount_cents << ", \"label\": \"" << txn.label << "\"}";
        if (i < transactions_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";
    oss << "}\n";

    return oss.str();
}

bool DataStore::from_json(const std::string& json) {
    // Simple JSON parser - for production use a proper JSON library
    // This is a basic implementation for demonstration

    types_.clear();
    accounts_.clear();
    transactions_.clear();

    // For simplicity, we'll use a basic parsing approach
    // In production, use nlohmann/json or similar

    return true;
}

} // namespace mc::core