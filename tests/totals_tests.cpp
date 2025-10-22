//
// Created by Jean-Michel Frouin on 22/10/2025.
//
#include <gtest/gtest.h>
#include "core/Totals.hpp"

TEST(TotalsTest, Calculate) {
    mc::core::Totals totals;

    totals.restant = 50000;          // 500€
    totals.somme_pointee = 30000;    // 300€
    totals.somme_en_ligne = 20000;   // 200€

    totals.calculate();

    // diff = somme_pointée - (restant - somme_en_ligne)
    // diff = 300 - (500 - 200) = 300 - 300 = 0
    EXPECT_EQ(totals.diff, 0);
}

TEST(TotalsTest, CalculateNegative) {
    mc::core::Totals totals;

    totals.restant = 50000;          // 500€
    totals.somme_pointee = 20000;    // 200€
    totals.somme_en_ligne = 10000;   // 100€

    totals.calculate();

    // diff = 200 - (500 - 100) = 200 - 400 = -200
    EXPECT_EQ(totals.diff, -20000);
}