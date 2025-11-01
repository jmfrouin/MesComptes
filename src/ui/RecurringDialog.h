//
// Created by Jean-Michel Frouin on 01/11/2025.
//

#ifndef RECURRINGDIALOG_H
#define RECURRINGDIALOG_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <core/Database.h>
#include <core/RecurringTransaction.h>

class RecurringDialog : public wxDialog {
public:
    RecurringDialog(wxWindow* parent, Database* db);

private:
    void CreateControls();
    void LoadRecurringTransactions();
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnExecuteNow(wxCommandEvent& event);
    void OnItemSelected(wxListEvent& event);
    void OnItemDeselected(wxListEvent& event);

    void ShowRecurringDialog(RecurringTransaction* existing = nullptr);
    wxString RecurrenceTypeToString(RecurrenceType type);

    Database* mDatabase;
    wxListCtrl* mRecurringList;
    wxButton* mEditBtn;
    wxButton* mDeleteBtn;
    wxButton* mExecuteBtn;

    enum {
        ID_RECURRING_LIST = wxID_HIGHEST + 100,
        ID_ADD_RECURRING,
        ID_EDIT_RECURRING,
        ID_DELETE_RECURRING,
        ID_EXECUTE_NOW
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // RECURRINGDIALOG_H