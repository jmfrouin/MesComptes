//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include "../../core/DataStore.hpp"
#include "../../core/Txn.hpp"

namespace mc::ui {

class TxnEditDlg : public wxDialog {
public:
    TxnEditDlg(wxWindow* parent, mc::core::DataStore& store, core::Txn& txn, bool is_new);

    core::Txn get_transaction() const { return txn_; }

private:
    mc::core::DataStore& store_;
    core::Txn& txn_;
    bool is_new_;

    wxDatePickerCtrl* date_picker_;
    wxTextCtrl* label_text_;
    wxTextCtrl* amount_text_;
    wxChoice* type_choice_;
    wxCheckBox* pointed_check_;

    void create_ui();
    void load_types();
    void on_ok(wxCommandEvent& event);
    void on_cancel(wxCommandEvent& event);
    
    bool validate_and_save();
};

} // namespace mc::ui