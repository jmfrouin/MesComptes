//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/dataview.h>
#include <wx/choice.h>
#include "core/DataStore.hpp"
#include "core/Account.hpp"
#include "core/Txn.hpp"
#include <vector>

namespace mc::ui {

class AccountsPage : public wxPanel {
public:
    AccountsPage(wxWindow* parent, mc::core::DataStore& store);

private:
    mc::core::DataStore& store_;
    
    wxChoice* account_choice_;
    wxStaticText* balance_text_;
    wxDataViewListCtrl* txn_list_;
    
    std::vector<core::Account> accounts_;
    std::vector<core::Txn> current_transactions_;
    int64_t current_account_id_ = -1;

    void create_ui();
    void load_accounts();
    void load_transactions();
    void update_balance();

    // Event handlers
    void on_account_selected(wxCommandEvent& event);
    void on_add_account(wxCommandEvent& event);
    void on_edit_account(wxCommandEvent& event);
    void on_delete_account(wxCommandEvent& event);
    void on_add_txn(wxCommandEvent& event);
    void on_edit_txn(wxCommandEvent& event);
    void on_delete_txn(wxCommandEvent& event);
    void on_import_csv(wxCommandEvent& event);
    void on_txn_activated(wxDataViewEvent& event);
    
    wxString format_currency(double amount);
    wxString format_date(int64_t timestamp);
};

} // namespace mc::ui