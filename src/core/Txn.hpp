//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <string>
#include <cstdint>
#include <optional>

namespace mc::core {

struct Txn {
    int64_t id = 0;
    int64_t account_id = 0;
    int64_t op_date = 0;        // epoch seconds
    std::string label;
    int64_t amount_cents = 0;   // positive = crédit, negative = débit
    bool pointed = false;
    std::optional<int64_t> type_id;
    int64_t created_at = 0;
    std::optional<int64_t> updated_at;

    Txn() = default;
    Txn(int64_t account_id_, int64_t op_date_, std::string label_, int64_t amount_cents_)
        : account_id(account_id_), op_date(op_date_), label(std::move(label_)), amount_cents(amount_cents_) {}
};

} // namespace mc::core