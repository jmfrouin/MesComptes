//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include "db/SqliteDB.hpp"
#include "core/Txn.hpp"
#include "core/Type.hpp"
#include <vector>

namespace mc::ui {

class TxnEditDlg : public wxDialog {
public:
    TxnEditDlg(wxWindow* parent, mc::db::SqliteDB& db, core::Txn& txn, bool is_new);

    core::Txn get_transaction() const { return txn_; }

private:
    mc::db::SqliteDB& db_;
    core::Txn& txn_;
    bool is_new_;
    std::vector<core::Type> types_;

    wxDatePickerCtrl* date_picker_;
    wxTextCtrl* label_ctrl_;
    wxSpinCtrlDouble* amount_ctrl_;
    wxCheckBox* pointed_check_;
    wxChoice* type_choice_;

    void create_ui();
    void load_types();
    void load_data();
    void save_data();

    void on_ok(wxCommandEvent& event);
    void on_cancel(wxCommandEvent& event);
};

} // namespace mc::ui