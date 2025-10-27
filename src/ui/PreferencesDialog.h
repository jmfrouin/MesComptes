//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <wx/wx.h>
#include <wx/listbox.h>
#include <core/Database.h>

class PreferencesDialog : public wxDialog {
public:
    PreferencesDialog(wxWindow* parent, Database* database);

private:
    void LoadTypes();
    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);

    wxListBox* mTypeList;
    wxTextCtrl* mNewTypeText;
    Database* mDatabase;

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_ADD_TYPE = wxID_HIGHEST + 100,
    ID_DELETE_TYPE
};

#endif // PREFERENCESDIALOG_H