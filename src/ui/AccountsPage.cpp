//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "AccountsPage.hpp"
#include "dialogs/TxnEditDlg.hpp"
#include "dialogs/ImportCsvDlg.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/datetime.h>

namespace mc::ui {

AccountsPage::AccountsPage(wxWindow* parent, mc::core::DataStore& store)
    : wxPanel(parent), store_(store) {
    create_ui();
    load_accounts();
}

void AccountsPage::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Barre de sélection de compte
    auto* account_bar = new wxBoxSizer(wxHORIZONTAL);
    account_bar->Add(new wxStaticText(this, wxID_ANY, "Compte:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    account_choice_ = new wxChoice(this, wxID_ANY);
    account_bar->Add(account_choice_, 1, wxEXPAND | wxRIGHT, 10);

    auto* btn_add_account = new wxButton(this, wxID_ANY, "Nouveau compte");
    account_bar->Add(btn_add_account, 0, wxRIGHT, 5);

    auto* btn_edit_account = new wxButton(this, wxID_ANY, "Modifier");
    account_bar->Add(btn_edit_account, 0, wxRIGHT, 5);

    auto* btn_delete_account = new wxButton(this, wxID_ANY, "Supprimer");
    account_bar->Add(btn_delete_account, 0);

    main_sizer->Add(account_bar, 0, wxALL | wxEXPAND, 5);

    // Solde du compte
    balance_text_ = new wxStaticText(this, wxID_ANY, "Solde: 0,00 €");
    wxFont balance_font = balance_text_->GetFont();
    balance_font.SetPointSize(14);
    balance_font.SetWeight(wxFONTWEIGHT_BOLD);
    balance_text_->SetFont(balance_font);
    main_sizer->Add(balance_text_, 0, wxALL | wxALIGN_CENTER, 10);

    // Liste des transactions
    txn_list_ = new wxDataViewListCtrl(this, wxID_ANY,
                                       wxDefaultPosition, wxDefaultSize,
                                       wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES);
    
    txn_list_->AppendTextColumn("Date", wxDATAVIEW_CELL_INERT, 100);
    txn_list_->AppendTextColumn("Libellé", wxDATAVIEW_CELL_INERT, 300);
    txn_list_->AppendTextColumn("Montant", wxDATAVIEW_CELL_INERT, 100);
    txn_list_->AppendTextColumn("Type", wxDATAVIEW_CELL_INERT, 150);
    txn_list_->AppendToggleColumn("Pointé", wxDATAVIEW_CELL_ACTIVATABLE, 60);

    main_sizer->Add(txn_list_, 1, wxALL | wxEXPAND, 5);

    // Boutons de gestion des transactions
    auto* txn_buttons = new wxBoxSizer(wxHORIZONTAL);
    
    auto* btn_add_txn = new wxButton(this, wxID_ANY, "Nouvelle transaction");
    txn_buttons->Add(btn_add_txn, 0, wxRIGHT, 5);

    auto* btn_edit_txn = new wxButton(this, wxID_ANY, "Modifier");
    txn_buttons->Add(btn_edit_txn, 0, wxRIGHT, 5);

    auto* btn_delete_txn = new wxButton(this, wxID_ANY, "Supprimer");
    txn_buttons->Add(btn_delete_txn, 0, wxRIGHT, 5);

    auto* btn_import = new wxButton(this, wxID_ANY, "Importer CSV");
    txn_buttons->Add(btn_import, 0);

    main_sizer->Add(txn_buttons, 0, wxALL | wxALIGN_CENTER, 5);

    SetSizer(main_sizer);

    // Bind events
    account_choice_->Bind(wxEVT_CHOICE, &AccountsPage::on_account_selected, this);
    btn_add_account->Bind(wxEVT_BUTTON, &AccountsPage::on_add_account, this);
    btn_edit_account->Bind(wxEVT_BUTTON, &AccountsPage::on_edit_account, this);
    btn_delete_account->Bind(wxEVT_BUTTON, &AccountsPage::on_delete_account, this);
    btn_add_txn->Bind(wxEVT_BUTTON, &AccountsPage::on_add_txn, this);
    btn_edit_txn->Bind(wxEVT_BUTTON, &AccountsPage::on_edit_txn, this);
    btn_delete_txn->Bind(wxEVT_BUTTON, &AccountsPage::on_delete_txn, this);
    btn_import->Bind(wxEVT_BUTTON, &AccountsPage::on_import_csv, this);
    txn_list_->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &AccountsPage::on_txn_activated, this);
}

void AccountsPage::load_accounts() {
    account_choice_->Clear();
    accounts_ = store_.get_all_accounts();

    for (const auto& account : accounts_) {
        account_choice_->Append(account.name, reinterpret_cast<void*>(account.id));
    }

    if (!accounts_.empty()) {
        account_choice_->SetSelection(0);
        current_account_id_ = accounts_[0].id;
        load_transactions();
        update_balance();
    }
}

void AccountsPage::load_transactions() {
    txn_list_->DeleteAllItems();
    
    if (current_account_id_ < 0) {
        return;
    }

    current_transactions_ = store_.get_transactions_by_account(current_account_id_);

    for (const auto& txn : current_transactions_) {
        wxVector<wxVariant> data;
        data.push_back(format_date(txn.op_date));
        data.push_back(wxString(txn.label));
        data.push_back(format_currency(txn.amount_cents));
        
        // Type
        wxString type_name = "-";
        if (txn.type_id.has_value()) {
            auto type_opt = store_.get_type_by_id(txn.type_id.value());
            if (type_opt.has_value()) {
                type_name = type_opt->name;
            }
        }
        data.push_back(type_name);
        data.push_back(txn.pointed);
        
        txn_list_->AppendItem(data);
    }
}

void AccountsPage::update_balance() {
    if (current_account_id_ < 0) {
        balance_text_->SetLabelText("Solde: 0,00 €");
        return;
    }

    double balance = store_.get_account_balance(current_account_id_);
    balance_text_->SetLabelText("Solde: " + format_currency(balance));
}

void AccountsPage::on_account_selected(wxCommandEvent& event) {
    int sel = account_choice_->GetSelection();
    if (sel != wxNOT_FOUND) {
        current_account_id_ = reinterpret_cast<int64_t>(account_choice_->GetClientData(sel));
        load_transactions();
        update_balance();
    }
}

void AccountsPage::on_add_account(wxCommandEvent& event) {
    wxTextEntryDialog dlg(this, "Nom du compte:", "Nouveau compte");
    if (dlg.ShowModal() == wxID_OK) {
        wxString name = dlg.GetValue().Trim();
        if (!name.IsEmpty()) {
            core::Account account;
            account.name = name.ToStdString();
            
            int64_t id = store_.add_account(account);
            if (id > 0) {
                load_accounts();
                // Sélectionner le nouveau compte
                for (size_t i = 0; i < accounts_.size(); i++) {
                    if (accounts_[i].id == id) {
                        account_choice_->SetSelection(i);
                        current_account_id_ = id;
                        load_transactions();
                        update_balance();
                        break;
                    }
                }
            }
        }
    }
}

void AccountsPage::on_edit_account(wxCommandEvent& event) {
    if (current_account_id_ < 0) {
        wxMessageBox("Aucun compte sélectionné", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    auto account_opt = store_.get_account_by_id(current_account_id_);
    if (!account_opt.has_value()) {
        wxMessageBox("Compte introuvable", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    core::Account account = account_opt.value();
    wxTextEntryDialog dlg(this, "Nom du compte:", "Modifier le compte", account.name);
    if (dlg.ShowModal() == wxID_OK) {
        wxString name = dlg.GetValue().Trim();
        if (!name.IsEmpty()) {
            account.name = name.ToStdString();
            if (store_.update_account(account)) {
                load_accounts();
            }
        }
    }
}

void AccountsPage::on_delete_account(wxCommandEvent& event) {
    if (current_account_id_ < 0) {
        wxMessageBox("Aucun compte sélectionné", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    if (wxMessageBox("Voulez-vous vraiment supprimer ce compte et toutes ses transactions ?",
                     "Confirmation", wxYES_NO | wxICON_QUESTION, this) == wxYES) {
        // Supprimer toutes les transactions du compte
        for (const auto& txn : current_transactions_) {
            store_.remove_transaction(txn.id);
        }
        
        // Supprimer le compte
        if (store_.remove_account(current_account_id_)) {
            current_account_id_ = -1;
            load_accounts();
        }
    }
}

void AccountsPage::on_add_txn(wxCommandEvent& event) {
    if (current_account_id_ < 0) {
        wxMessageBox("Veuillez d'abord sélectionner un compte", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    core::Txn txn;
    txn.account_id = current_account_id_;
    
    TxnEditDlg dlg(this, store_, txn, true);
    if (dlg.ShowModal() == wxID_OK) {
        load_transactions();
        update_balance();
    }
}

void AccountsPage::on_edit_txn(wxCommandEvent& event) {
    int row = txn_list_->GetSelectedRow();
    if (row == wxNOT_FOUND) {
        wxMessageBox("Veuillez sélectionner une transaction", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    if (row >= 0 && row < static_cast<int>(current_transactions_.size())) {
        core::Txn txn = current_transactions_[row];
        TxnEditDlg dlg(this, store_, txn, false);
        if (dlg.ShowModal() == wxID_OK) {
            load_transactions();
            update_balance();
        }
    }
}

void AccountsPage::on_delete_txn(wxCommandEvent& event) {
    int row = txn_list_->GetSelectedRow();
    if (row == wxNOT_FOUND) {
        wxMessageBox("Veuillez sélectionner une transaction", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    if (wxMessageBox("Voulez-vous vraiment supprimer cette transaction ?",
                     "Confirmation", wxYES_NO | wxICON_QUESTION, this) == wxYES) {
        if (row >= 0 && row < static_cast<int>(current_transactions_.size())) {
            int64_t txn_id = current_transactions_[row].id;
            if (store_.remove_transaction(txn_id)) {
                load_transactions();
                update_balance();
            }
        }
    }
}

void AccountsPage::on_import_csv(wxCommandEvent& event) {
    if (current_account_id_ < 0) {
        wxMessageBox("Veuillez d'abord sélectionner un compte", "Erreur", wxOK | wxICON_ERROR, this);
        return;
    }

    ImportCsvDlg dlg(this, store_, current_account_id_);
    if (dlg.ShowModal() == wxID_OK) {
        load_transactions();
        update_balance();
    }
}

void AccountsPage::on_txn_activated(wxDataViewEvent& event) {
    int row = txn_list_->GetSelectedRow();
    if (row >= 0 && row < static_cast<int>(current_transactions_.size())) {
        core::Txn txn = current_transactions_[row];
        TxnEditDlg dlg(this, store_, txn, false);
        if (dlg.ShowModal() == wxID_OK) {
            load_transactions();
            update_balance();
        }
    }
}

wxString AccountsPage::format_currency(double amount) {
    double euros = amount / 100.0;
    return wxString::Format("%.2f €", euros);
}

wxString AccountsPage::format_date(int64_t timestamp) {
    if (timestamp == 0) {
        return "-";
    }
    
    wxDateTime dt;
    dt.Set(static_cast<time_t>(timestamp));
    return dt.FormatDate();
}

} // namespace mc::ui