//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/dataview.h>
#include <wx/spinctrl.h>
#include "db/SqliteDB.hpp"
#include "core/Txn.hpp"
#include "core/Totals.hpp"
#include <vector>

namespace mc::ui {

class AccountsPage : public wxPanel {
public:
    AccountsPage(wxWindow* parent, mc::db::SqliteDB& db);

private:
    mc::db::SqliteDB& db_;
    int64_t current_account_id_ = 1; // TODO: Gérer la sélection de compte

    wxDataViewListCtrl* txn_list_;
    wxStaticText* restant_text_;
    wxStaticText* somme_pointee_text_;
    wxSpinCtrlDouble* somme_en_ligne_spin_;
    wxStaticText* diff_text_;

    std::vector<core::Txn> transactions_;
    core::Totals totals_;

    void create_ui();
    void create_toolbar(wxBoxSizer* sizer);
    void create_transaction_list(wxBoxSizer* sizer);
    void create_totals_bar(wxBoxSizer* sizer);

    void refresh_transactions();
    void refresh_totals();
    void update_totals_display();

    // Event handlers
    void on_add_transaction(wxCommandEvent& event);
    void on_edit_transaction(wxCommandEvent& event);
    void on_delete_transaction(wxCommandEvent& event);
    void on_toggle_pointed(wxCommandEvent& event);
    void on_somme_en_ligne_changed(wxSpinDoubleEvent& event);
    void on_txn_activated(wxDataViewEvent& event);

    wxString format_currency(int64_t cents);
    wxString format_date(int64_t timestamp);
};

} // namespace mc::ui