//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <core/Database.h>

class PreferencesDialog : public wxDialog {
public:
    PreferencesDialog(wxWindow* parent, Database* database);

private:
    void CreateTypesPage(wxNotebook* notebook);
    void CreateDisplayPage(wxNotebook* notebook);
    
    void LoadTypes();
    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnItemActivated(wxListEvent& event);
    void OnDateFormatChanged(wxCommandEvent& event);
    void OnDecimalSeparatorChanged(wxCommandEvent& event);
    void OnLanguageChanged(wxCommandEvent& event);

    wxListCtrl* mTypesList;
    wxTextCtrl* mNewTypeText;
    wxRadioBox* mTypeRadio;
    wxChoice* mDateFormatChoice;
    wxChoice* mDecimalSeparatorChoice;
    wxChoice* mLanguageChoice;
    wxStaticText* mDateExample;
    wxStaticText* mMoneyExample;
    Database* mDatabase;

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_ADD_TYPE = wxID_HIGHEST + 100,
    ID_DELETE_TYPE,
    ID_EDIT_TYPE,
    ID_TYPES_LIST,
    ID_DATE_FORMAT,
    ID_DECIMAL_SEPARATOR,
    ID_LANGUAGE
};

#endif // PREFERENCESDIALOG_H