//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "AccountsPage.hpp"
#include "dialogs/TxnEditDlg.hpp"
#include "db/dao/TxnDAO.hpp"
#include "db/dao/TypeDAO.hpp"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace mc::ui {

enum {
    ID_AddTxn = wxID_HIGHEST + 1,
    ID_EditTxn,
    ID_DeleteTxn,
    ID_TogglePointed,
    ID_SommeEnLigne
};

AccountsPage::AccountsPage(wxWindow* parent, mc::db::SqliteDB& db)
    : wxPanel(parent), db_(db) {
    create_ui();
    refresh_transactions();
    refresh_totals();
}

void AccountsPage::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    create_toolbar(main_sizer);
    create_transaction_list(main_sizer);
    create_totals_bar(main_sizer);

    SetSizer(main_sizer);

    // Bind events
    Bind(wxEVT_BUTTON, &AccountsPage::on_add_transaction, this, ID_AddTxn);
    Bind(wxEVT_BUTTON, &AccountsPage::on_edit_transaction, this, ID_EditTxn);
    Bind(wxEVT_BUTTON, &AccountsPage::on_delete_transaction, this, ID_DeleteTxn);
    Bind(wxEVT_BUTTON, &AccountsPage::on_toggle_pointed, this, ID_TogglePointed);
    Bind(wxEVT_SPINCTRLDOUBLE, &AccountsPage::on_somme_en_ligne_changed, this, ID_SommeEnLigne);
}

void AccountsPage::create_toolbar(wxBoxSizer* sizer) {
    auto* toolbar_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto* btn_add = new wxButton(this, ID_AddTxn, "Ajouter (Ins)");
    auto* btn_edit = new wxButton(this, ID_EditTxn, "Éditer (Enter)");
    auto* btn_delete = new wxButton(this, ID_DeleteTxn, "Supprimer (Del)");
    auto* btn_toggle = new wxButton(this, ID_TogglePointed, "Pointer/Dépointer (Space)");

    toolbar_sizer->Add(btn_add, 0, wxALL, 5);
    toolbar_sizer->Add(btn_edit, 0, wxALL, 5);
    toolbar_sizer->Add(btn_delete, 0, wxALL, 5);
    toolbar_sizer->Add(btn_toggle, 0, wxALL, 5);

    sizer->Add(toolbar_sizer, 0, wxALL | wxEXPAND, 5);
}

void AccountsPage::create_transaction_list(wxBoxSizer* sizer) {
    txn_list_ = new wxDataViewListCtrl(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxDV_ROW_LINES | wxDV_MULTIPLE | wxDV_VERT_RULES);

    // Colonnes
    txn_list_->AppendTextColumn("Date", wxDATAVIEW_CELL_INERT, 120);
    txn_list_->AppendTextColumn("Libellé", wxDATAVIEW_CELL_INERT, 300);
    txn_list_->AppendTextColumn("Somme", wxDATAVIEW_CELL_INERT, 100);
    txn_list_->AppendToggleColumn("Pointée", wxDATAVIEW_CELL_ACTIVATABLE, 80);
    txn_list_->AppendTextColumn("Type", wxDATAVIEW_CELL_INERT, 100);

    sizer->Add(txn_list_, 1, wxALL | wxEXPAND, 5);

    txn_list_->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &AccountsPage::on_txn_activated, this);
}

void AccountsPage::create_totals_bar(wxBoxSizer* sizer) {
    auto* totals_sizer = new wxFlexGridSizer(2, 4, 5, 10);
    totals_sizer->AddGrowableCol(1);
    totals_sizer->AddGrowableCol(3);

    // Restant
    totals_sizer->Add(new wxStaticText(this, wxID_ANY, "Restant:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    restant_text_ = new wxStaticText(this, wxID_ANY, "0,00 €");
    totals_sizer->Add(restant_text_, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    // Somme pointée
    totals_sizer->Add(new wxStaticText(this, wxID_ANY, "Somme pointée:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    somme_pointee_text_ = new wxStaticText(this, wxID_ANY, "0,00 €");
    totals_sizer->Add(somme_pointee_text_, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    // Somme en ligne
    totals_sizer->Add(new wxStaticText(this, wxID_ANY, "Somme en ligne:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    somme_en_ligne_spin_ = new wxSpinCtrlDouble(this, ID_SommeEnLigne, "0.00",
        wxDefaultPosition, wxSize(150, -1), wxSP_ARROW_KEYS, -999999.99, 999999.99, 0.0, 0.01);
    totals_sizer->Add(somme_en_ligne_spin_, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    // Diff
    totals_sizer->Add(new wxStaticText(this, wxID_ANY, "Diff:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    diff_text_ = new wxStaticText(this, wxID_ANY, "0,00 €");
    totals_sizer->Add(diff_text_, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    sizer->Add(totals_sizer, 0, wxALL | wxEXPAND, 10);
}

void AccountsPage::refresh_transactions() {
    txn_list_->DeleteAllItems();

    db::dao::TxnDAO txn_dao(db_);
    db::dao::TypeDAO type_dao(db_);

    transactions_ = txn_dao.find_by_account(current_account_id_);

    for (const auto& txn : transactions_) {
        wxVector<wxVariant> row;
        row.push_back(format_date(txn.op_date));
        row.push_back(wxVariant(txn.label));
        row.push_back(format_currency(txn.amount_cents));
        row.push_back(wxVariant(txn.pointed));

        wxString type_name = "-";
        if (txn.type_id) {
            auto type_opt = type_dao.find_by_id(*txn.type_id);
            if (type_opt) {
                type_name = type_opt->name;
            }
        }
        row.push_back(wxVariant(type_name));

        txn_list_->AppendItem(row);
    }
}

void AccountsPage::refresh_totals() {
    db::dao::TxnDAO txn_dao(db_);
    totals_ = txn_dao.calculate_totals(current_account_id_);

    update_totals_display();
}

void AccountsPage::update_totals_display() {
    restant_text_->SetLabelText(format_currency(totals_.restant));
    somme_pointee_text_->SetLabelText(format_currency(totals_.somme_pointee));
    somme_en_ligne_spin_->SetValue(totals_.somme_en_ligne / 100.0);
    diff_text_->SetLabelText(format_currency(totals_.diff));
}

void AccountsPage::on_add_transaction(wxCommandEvent& WXUNUSED(event)) {
    core::Txn txn;
    txn.account_id = current_account_id_;
    txn.op_date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    TxnEditDlg dlg(this, db_, txn, true);
    if (dlg.ShowModal() == wxID_OK) {
        refresh_transactions();
        refresh_totals();
    }
}

void AccountsPage::on_edit_transaction(wxCommandEvent& WXUNUSED(event)) {
    int row = txn_list_->GetSelectedRow();
    if (row == wxNOT_FOUND) {
        wxMessageBox("Veuillez sélectionner une transaction", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    if (row >= 0 && row < static_cast<int>(transactions_.size())) {
        TxnEditDlg dlg(this, db_, transactions_[row], false);
        if (dlg.ShowModal() == wxID_OK) {
            refresh_transactions();
            refresh_totals();
        }
    }
}

void AccountsPage::on_delete_transaction(wxCommandEvent& WXUNUSED(event)) {
    int row = txn_list_->GetSelectedRow();
    if (row == wxNOT_FOUND) {
        wxMessageBox("Veuillez sélectionner une transaction", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    if (row >= 0 && row < static_cast<int>(transactions_.size())) {
        int answer = wxMessageBox("Êtes-vous sûr de vouloir supprimer cette transaction ?",
                                 "Confirmation", wxYES_NO | wxICON_QUESTION);
        if (answer == wxYES) {
            db::dao::TxnDAO txn_dao(db_);
            if (txn_dao.remove(transactions_[row].id)) {
                refresh_transactions();
                refresh_totals();
            }
        }
    }
}

void AccountsPage::on_toggle_pointed(wxCommandEvent& WXUNUSED(event)) {
    int row = txn_list_->GetSelectedRow();
    if (row == wxNOT_FOUND) {
        wxMessageBox("Veuillez sélectionner une transaction", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    if (row >= 0 && row < static_cast<int>(transactions_.size())) {
        db::dao::TxnDAO txn_dao(db_);
        if (txn_dao.toggle_pointed(transactions_[row].id)) {
            refresh_transactions();
            refresh_totals();
        }
    }
}

void AccountsPage::on_somme_en_ligne_changed(wxSpinDoubleEvent& WXUNUSED(event)) {
    int64_t new_value = static_cast<int64_t>(somme_en_ligne_spin_->GetValue() * 100);
    totals_.somme_en_ligne = new_value;
    totals_.calculate();

    // Sauvegarder dans settings
    std::string key = "online_sum:" + std::to_string(current_account_id_);
    std::string value = std::to_string(new_value);

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    diff_text_->SetLabelText(format_currency(totals_.diff));
}

void AccountsPage::on_txn_activated(wxDataViewEvent& WXUNUSED(event)) {
    wxCommandEvent dummy;
    on_edit_transaction(dummy);
}

wxString AccountsPage::format_currency(int64_t cents) {
    double euros = cents / 100.0;
    return wxString::Format("%.2f €", euros);
}

wxString AccountsPage::format_date(int64_t timestamp) {
    std::time_t time = static_cast<std::time_t>(timestamp);
    std::tm* tm_info = std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(tm_info, "%d/%m/%Y");
    return wxString(oss.str());
}

} // namespace mc::ui