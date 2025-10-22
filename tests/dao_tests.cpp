//
// Created by Jean-Michel Frouin on 22/10/2025.
//
#include <gtest/gtest.h>
#include "db/SqliteDB.hpp"
#include "db/Migrations.hpp"
#include "db/dao/AccountDAO.hpp"
#include "db/dao/TxnDAO.hpp"
#include "db/dao/TypeDAO.hpp"
#include <filesystem>

class DAOTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_path_ = ":memory:"; // Base de données en mémoire pour les tests
        db_ = std::make_unique<mc::db::SqliteDB>(db_path_);
        ASSERT_TRUE(db_->open());

        mc::db::Migrations migrations(*db_);
        ASSERT_TRUE(migrations.apply_all());
    }

    void TearDown() override {
        db_.reset();
    }

    std::string db_path_;
    std::unique_ptr<mc::db::SqliteDB> db_;
};

TEST_F(DAOTest, AccountCRUD) {
    mc::db::dao::AccountDAO dao(*db_);

    // Create
    mc::core::Account account("Test Account");
    account.iban = "FR76...";
    account.created_at = std::time(nullptr);

    int64_t id = dao.insert(account);
    ASSERT_GT(id, 0);

    // Read
    auto found = dao.find_by_id(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Test Account");
    EXPECT_EQ(found->iban, "FR76...");

    // Update
    account.id = id;
    account.name = "Updated Account";
    ASSERT_TRUE(dao.update(account));

    found = dao.find_by_id(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Updated Account");

    // Delete
    ASSERT_TRUE(dao.remove(id));
    found = dao.find_by_id(id);
    EXPECT_FALSE(found.has_value());
}

TEST_F(DAOTest, TxnCRUD) {
    mc::db::dao::AccountDAO account_dao(*db_);
    mc::db::dao::TxnDAO txn_dao(*db_);

    // Créer un compte
    mc::core::Account account("Test Account");
    account.created_at = std::time(nullptr);
    int64_t account_id = account_dao.insert(account);
    ASSERT_GT(account_id, 0);

    // Create transaction
    mc::core::Txn txn(account_id, std::time(nullptr), "Test Transaction", 10050); // 100.50€
    txn.pointed = true;
    txn.created_at = std::time(nullptr);

    int64_t txn_id = txn_dao.insert(txn);
    ASSERT_GT(txn_id, 0);

    // Read
    auto found = txn_dao.find_by_id(txn_id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->label, "Test Transaction");
    EXPECT_EQ(found->amount_cents, 10050);
    EXPECT_TRUE(found->pointed);

    // Toggle pointed
    ASSERT_TRUE(txn_dao.toggle_pointed(txn_id));
    found = txn_dao.find_by_id(txn_id);
    EXPECT_FALSE(found->pointed);
}