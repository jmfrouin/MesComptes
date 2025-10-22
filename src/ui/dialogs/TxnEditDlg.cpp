//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "TxnEditDlg.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <chrono>
#include <ctime>

namespace mc::ui {

TxnEditDlg::TxnEditDlg(wxWindow* parent, mc::core::DataStore& store, core::Txn& txn, bool is_new)
    : wxDialog(parent, wxID_ANY, is_new ? "Nouvelle transaction" : "Modifier la transaction",
               wxDefaultPosition, wxSize(400, 300)),
      store_(store), txn_(txn), is_new_(is_new) {
    create_ui();
}

void TxnEditDlg::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Date
    auto* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(new wxStaticText(this, wxID_ANY, "Date:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    date_picker_ = new wxDatePickerCtrl(this, wxID_ANY);
    
    // Convertir epoch en wxDateTime si la transaction a une date
    if (txn_.op_date > 0) {
        wxDateTime dt;
        dt.Set(static_cast<time_t>(txn_.op_date));
        date_picker_->SetValue(dt);
    }
    
    date_sizer->Add(date_picker_, 1, wxEXPAND);
    main_sizer->Add(date_sizer, 0, wxALL | wxEXPAND, 5);

    // Label
    auto* label_sizer = new wxBoxSizer(wxHORIZONTAL);
    label_sizer->Add(new wxStaticText(this, wxID_ANY, "Libellé:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    label_text_ = new wxTextCtrl(this, wxID_ANY, txn_.label);
    label_sizer->Add(label_text_, 1, wxEXPAND);
    main_sizer->Add(label_sizer, 0, wxALL | wxEXPAND, 5);

    // Montant
    auto* amount_sizer = new wxBoxSizer(wxHORIZONTAL);
    amount_sizer->Add(new wxStaticText(this, wxID_ANY, "Montant (€):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    amount_text_ = new wxTextCtrl(this, wxID_ANY, 
        wxString::Format("%.2f", txn_.amount_cents / 100.0));
    amount_sizer->Add(amount_text_, 1, wxEXPAND);
    main_sizer->Add(amount_sizer, 0, wxALL | wxEXPAND, 5);

    // Type
    auto* type_sizer = new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(new wxStaticText(this, wxID_ANY, "Type:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    type_choice_ = new wxChoice(this, wxID_ANY);
    load_types();
    type_sizer->Add(type_choice_, 1, wxEXPAND);
    main_sizer->Add(type_sizer, 0, wxALL | wxEXPAND, 5);

    // Pointé
    pointed_check_ = new wxCheckBox(this, wxID_ANY, "Transaction pointée");
    pointed_check_->SetValue(txn_.pointed);
    main_sizer->Add(pointed_check_, 0, wxALL, 5);

    // Boutons
    auto* button_sizer = CreateButtonSizer(wxOK | wxCANCEL);
    main_sizer->Add(button_sizer, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(main_sizer);

    // Bind events
    Bind(wxEVT_BUTTON, &TxnEditDlg::on_ok, this, wxID_OK);
    Bind(wxEVT_BUTTON, &TxnEditDlg::on_cancel, this, wxID_CANCEL);
}

void TxnEditDlg::load_types() {
    type_choice_->Clear();
    type_choice_->Append("(Aucun)", reinterpret_cast<void*>(-1));

    auto types = store_.get_all_types();
    for (const auto& type : types) {
        type_choice_->Append(type.name, reinterpret_cast<void*>(type.id));
        
        // Sélectionner le type actuel si défini
        if (txn_.type_id.has_value() && txn_.type_id.value() == type.id) {
            type_choice_->SetSelection(type_choice_->GetCount() - 1);
        }
    }

    // Si aucun type n'est sélectionné, sélectionner "(Aucun)"
    if (type_choice_->GetSelection() == wxNOT_FOUND) {
        type_choice_->SetSelection(0);
    }
}

void TxnEditDlg::on_ok(wxCommandEvent& event) {
    if (validate_and_save()) {
        EndModal(wxID_OK);
    }
}

void TxnEditDlg::on_cancel(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

bool TxnEditDlg::validate_and_save() {
    // Valider le libellé
    wxString label = label_text_->GetValue().Trim();
    if (label.IsEmpty()) {
        wxMessageBox("Le libellé est obligatoire", "Erreur", wxOK | wxICON_ERROR, this);
        return false;
    }

    // Valider le montant
    double amount;
    if (!amount_text_->GetValue().ToDouble(&amount)) {
        wxMessageBox("Le montant est invalide", "Erreur", wxOK | wxICON_ERROR, this);
        return false;
    }

    // Récupérer la date
    wxDateTime date = date_picker_->GetValue();
    
    // Récupérer le type
    int sel = type_choice_->GetSelection();
    std::optional<int64_t> type_id;
    if (sel != wxNOT_FOUND && sel > 0) {
        type_id = reinterpret_cast<int64_t>(type_choice_->GetClientData(sel));
    }

    // Mettre à jour la transaction
    txn_.label = label.ToStdString();
    txn_.amount_cents = static_cast<int64_t>(amount * 100);
    txn_.op_date = date.GetTicks();
    txn_.type_id = type_id;
    txn_.pointed = pointed_check_->GetValue();

    // Sauvegarder dans le DataStore
    if (is_new_) {
        int64_t new_id = store_.add_transaction(txn_);
        if (new_id < 0) {
            wxMessageBox("Erreur lors de l'ajout de la transaction", "Erreur", wxOK | wxICON_ERROR, this);
            return false;
        }
        txn_.id = new_id;
    } else {
        if (!store_.update_transaction(txn_)) {
            wxMessageBox("Erreur lors de la mise à jour de la transaction", "Erreur", wxOK | wxICON_ERROR, this);
            return false;
        }
    }

    return true;
}

} // namespace mc::ui