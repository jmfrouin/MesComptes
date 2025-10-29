//
// Created by Jean-Michel Frouin
//

#ifndef CSVIMPORTDIALOG_H
#define CSVIMPORTDIALOG_H

#include <wx/wx.h>
#include <wx/choice.h>
#include <vector>
#include <string>
#include <core/Database.h>

class CSVImportDialog : public wxDialog {
public:
    CSVImportDialog(wxWindow* parent, Database* database,
                    const std::vector<std::string>& csvHeaders,
                    const std::vector<std::vector<std::string>>& csvData);

    struct FieldMapping {
        int dateColumn;
        int libelleColumn;
        int sommeColumn;
        int typeColumn;
        std::string defaultType;
        bool pointeeByDefault;
        char separator;
    };

    FieldMapping GetMapping() const { return mMapping; }
    bool IsImportConfirmed() const { return mConfirmed; }

private:
    void CreateControls();
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnColumnChoiceChanged(wxCommandEvent& event);
    void UpdatePreview();

    Database* mDatabase;
    std::vector<std::string> mCSVHeaders;
    std::vector<std::vector<std::string>> mCSVData;
    FieldMapping mMapping;
    bool mConfirmed;

    // Widgets
    wxChoice* mDateChoice;
    wxChoice* mLibelleChoice;
    wxChoice* mSommeChoice;
    wxChoice* mTypeChoice;
    wxChoice* mDefaultTypeChoice;
    wxCheckBox* mPointeeCheck;
    wxTextCtrl* mPreviewText;
    wxRadioBox* mSeparatorChoice;

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_CSV_DATE_CHOICE = wxID_HIGHEST + 100,
    ID_CSV_LIBELLE_CHOICE,
    ID_CSV_SOMME_CHOICE,
    ID_CSV_TYPE_CHOICE,
    ID_CSV_SEPARATOR_CHOICE
};

#endif // CSVIMPORTDIALOG_H