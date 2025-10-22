//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "HomePage.hpp"
#include <wx/sizer.h>
#include <wx/statbox.h>

namespace mc::ui {

HomePage::HomePage(wxWindow* parent, mc::core::DataStore& store)
    : wxPanel(parent), store_(store) {
    create_ui();
    refresh_stats();
}

void HomePage::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Titre
    auto* title = new wxStaticText(this, wxID_ANY, "Bienvenue dans Mes Comptes");
    wxFont title_font = title->GetFont();
    title_font.SetPointSize(16);
    title_font.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(title_font);
    main_sizer->Add(title, 0, wxALL | wxALIGN_CENTER, 20);

    // Statistiques
    auto* stats_box = new wxStaticBoxSizer(wxVERTICAL, this, "Statistiques");

    account_count_text_ = new wxStaticText(this, wxID_ANY, "Nombre de comptes: 0");
    stats_box->Add(account_count_text_, 0, wxALL, 5);

    total_balance_text_ = new wxStaticText(this, wxID_ANY, "Solde total: 0,00 €");
    stats_box->Add(total_balance_text_, 0, wxALL, 5);

    last_update_text_ = new wxStaticText(this, wxID_ANY, "Dernière mise à jour: -");
    stats_box->Add(last_update_text_, 0, wxALL, 5);

    main_sizer->Add(stats_box, 0, wxALL | wxEXPAND, 20);

    // Informations
    auto* info_text = new wxStaticText(this, wxID_ANY,
        "Utilisez l'onglet 'Mes Comptes' pour gérer vos transactions.\n"
        "Utilisez l'onglet 'Préférences' pour configurer les types de transactions.");
    main_sizer->Add(info_text, 0, wxALL | wxALIGN_CENTER, 20);

    main_sizer->AddStretchSpacer();

    SetSizer(main_sizer);
}

void HomePage::refresh_stats() {
    auto accounts = store_.get_all_accounts();
    account_count_text_->SetLabelText(
        wxString::Format("Nombre de comptes: %zu", accounts.size())
    );

    double total_balance = store_.get_total_balance();
    total_balance_text_->SetLabelText(
        "Solde total: " + format_currency(total_balance)
    );

    // TODO: Récupérer la date de dernière modification
    last_update_text_->SetLabelText("Dernière mise à jour: Aujourd'hui");
}

wxString HomePage::format_currency(double amount) {
    double euros = amount / 100.0;
    return wxString::Format("%.2f €", euros);
}

} // namespace mc::ui