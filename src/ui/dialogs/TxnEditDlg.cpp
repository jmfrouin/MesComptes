//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "TxnEditDlg.hpp"
#include "db/dao/TxnDAO.hpp"
#include "db/dao/TypeDAO.hpp"
#include <wx/sizer.h>
#include <chrono>

namespace mc::ui {

TxnEditDlg::TxnEditDlg(wxWindow* parent, mc::db::SqliteDB& db, core::Txn& txn, bool is_new)
    : wxDialog(parent, wxID_ANY, is_new ? "Nouvelle transaction" : "Modifier la transaction",
               wxDefaultPosition, wxSize(400, 300)),
      db_(db), txn_(txn), is_new_(is_new) {

    load_types();
    create_ui();
    load_data();
}

void TxnEditDlg::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Date
    auto* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(new wxStaticText(this, wxID_ANY, "Date:", wxDefaultPosition, wxSize(100, -1)),
                    0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    date_picker_ = new wxDatePickerCtrl(this, wxID_ANY);
    date_sizer->Add(date_picker_, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(date_sizer, 0, wxALL | wxEXPAND, 5);

    // Libellé
    auto* label_sizer = new wxBoxSizer(wxHORIZONTAL);
    label_sizer->Add(new wxStaticText(this, wxID_ANY, "Libellé:", wxDefaultPosition, wxSize(100, -1)),
                     0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    label_ctrl_ = new wxTextCtrl(this, wxID_ANY);
    label_sizer->Add(label_ctrl_, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(label_sizer, 0, wxALL | wxEXPAND, 5);

    // Montant
    auto* amount_sizer = new wxBoxSizer(wxHORIZONTAL);
    amount_sizer->Add(new wxStaticText(this, wxID_ANY, "Montant (€):", wxDefaultPosition, wxSize(100, -1)),
                      0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    amount_ctrl_ = new wxSpinCtrlDouble(this, wxID_ANY, "0.00",
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSP_ARROW_KEYS, -999999.99, 999999.99, 0.0, 0.01);
    amount_ctrl_->SetDigits(2);
    amount_sizer->Add(amount_ctrl_, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(amount_sizer, 0, wxALL | wxEXPAND, 5);

    // Type
    auto* type_sizer = new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(new wxStaticText(this, wxID_ANY, "Type:", wxDefaultPosition, wxSize(100, -1)),
                    0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    type_choice_ = new wxChoice(this, wxID_ANY);
    type_choice_->Append("(Aucun)", reinterpret_cast<void*>(-1));
    for (const auto& type : types_) {
        type_choice_->Append(type.name, reinterpret_cast<void*>(type.id));
    }
    type_sizer->Add(type_choice_, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(type_sizer, 0, wxALL | wxEXPAND, 5);

    // Pointée
    pointed_check_ = new wxCheckBox(this, wxID_ANY, "Transaction pointée");
    main_sizer->Add(pointed_check_, 0, wxALL | wxEXPAND, 5);

    // Boutons
    auto* btn_sizer = CreateButtonSizer(wxOK | wxCANCEL);
    main_sizer->Add(btn_sizer, 0, wxALL | wxEXPAND, 10);

    SetSizer(main_sizer);

    // Events
    Bind(wxEVT_BUTTON, &TxnEditDlg::on_ok, this, wxID_OK);
    Bind(wxEVT_BUTTON, &TxnEditDlg::on_cancel, this, wxID_CANCEL);
}

void TxnEditDlg::load_types() {
    db::dao::TypeDAO type_dao(db_);
    types_ = type_dao.find_all();
}

void TxnEditDlg::load_data() {
    if (!is_new_) {
        // Charger la date
        std::time_t time = static_cast<std::time_t>(txn_.op_date);
        std::tm* tm_info = std::localtime(&time);
        wxDateTime date;
        date.Set(tm_info->tm_mday, static_cast<wxDateTime::Month>(tm_info->tm_mon),
                tm_info->tm_year + 1900);
        date_picker_->SetValue(date);

        // Charger le libellé
        label_ctrl_->SetValue(txn_.label);

        // Charger le montant
        double euros = txn_.amount_cents / 100.0;
        amount_ctrl_->SetValue(euros);

        // Charger le type
        if (txn_.type_id) {
            for (size_t i = 0; i < types_.size(); ++i) {
                if (types_[i].id == *txn_.type_id) {
                    type_choice_->SetSelection(i + 1); // +1 car index 0 = (Aucun)
                    break;
                }
            }
        } else {
            type_choice_->SetSelection(0);
        }

        // Charger l'état pointé
        pointed_check_->SetValue(txn_.pointed);
    } else {
        type_choice_->SetSelection(0);
    }
}

void TxnEditDlg::save_data() {
    // Date
    wxDateTime date = date_picker_->GetValue();
    std::tm tm_info = {};
    tm_info.tm_mday = date.GetDay();
    tm_info.tm_mon = date.GetMonth();
    tm_info.tm_year = date.GetYear() - 1900;
    txn_.op_date = std::mktime(&tm_info);

    // Libellé
    txn_.label = label_ctrl_->GetValue().ToStdString();

    // Montant
    txn_.amount_cents = static_cast<int64_t>(amount_ctrl_->GetValue() * 100);

    // Type
    int type_selection = type_choice_->GetSelection();
    if (type_selection > 0 && type_selection <= static_cast<int>(types_.size())) {
        txn_.type_id = types_[type_selection - 1].id;
    } else {
        txn_.type_id = std::nullopt;
    }

    // Pointée
    txn_.pointed = pointed_check_->GetValue();
}

void TxnEditDlg::on_ok(wxCommandEvent& WXUNUSED(event)) {
    // Validation
    if (label_ctrl_->GetValue().Trim().IsEmpty()) {
        wxMessageBox("Le libellé ne peut pas être vide", "Erreur", wxOK | wxICON_ERROR);
        return;
    }

    save_data();

    db::dao::TxnDAO txn_dao(db_);

    if (is_new_) {
        txn_.created_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        int64_t id = txn_dao.insert(txn_);
        if (id <= 0) {
            wxMessageBox("Impossible d'ajouter la transaction", "Erreur", wxOK | wxICON_ERROR);
            return;
        }
        txn_.id = id;
    } else {
        if (!txn_dao.update(txn_)) {
            wxMessageBox("Impossible de modifier la transaction", "Erreur", wxOK | wxICON_ERROR);
            return;
        }
    }

    EndModal(wxID_OK);
}

void TxnEditDlg::on_cancel(wxCommandEvent& WXUNUSED(event)) {
    EndModal(wxID_CANCEL);
}

} // namespace mc::ui