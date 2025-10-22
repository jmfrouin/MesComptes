//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <cstdint>

namespace mc::core {

struct Totals {
    int64_t restant = 0;            // somme de tous les montants
    int64_t somme_pointee = 0;      // somme des montants pointés
    int64_t somme_en_ligne = 0;     // valeur éditable utilisateur
    int64_t diff = 0;               // somme_pointée - (restant - somme_en_ligne)

    void calculate() {
        diff = somme_pointee - (restant - somme_en_ligne);
    }
};

} // namespace mc::core