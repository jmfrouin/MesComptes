//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include <ui/PreferencesDialog.h>
#include <core/Settings.h>
#include <core/LanguageManager.h>

wxBEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
    EVT_BUTTON(ID_ADD_TYPE, PreferencesDialog::OnAdd)
    EVT_BUTTON(ID_DELETE_TYPE, PreferencesDialog::OnDelete)
    EVT_BUTTON(ID_EDIT_TYPE, PreferencesDialog::OnEdit)
    EVT_LIST_ITEM_ACTIVATED(ID_TYPES_LIST, PreferencesDialog::OnItemActivated)
    EVT_CHOICE(ID_DATE_FORMAT, PreferencesDialog::OnDateFormatChanged)
    EVT_CHOICE(ID_DECIMAL_SEPARATOR, PreferencesDialog::OnDecimalSeparatorChanged)
wxEND_EVENT_TABLE()

PreferencesDialog::PreferencesDialog(wxWindow* parent, Database* database)
    : wxDialog(parent, wxID_ANY, _("Preferences"),
               wxDefaultPosition, wxSize(600, 400)),
      mDatabase(database) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Créer un notebook pour les différentes pages
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
    
    CreateTypesPage(notebook);
    CreateDisplayPage(notebook);
    
    mainSizer->Add(notebook, 1, wxALL | wxEXPAND, 10);

    wxButton* closeBtn = new wxButton(this, wxID_CLOSE, _("Close"));
    mainSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(mainSizer);
}

void PreferencesDialog::CreateTypesPage(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Liste des types
    mTypesList = new wxListCtrl(panel, ID_TYPES_LIST, wxDefaultPosition,
                                 wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mTypesList->AppendColumn(_("Name"), wxLIST_FORMAT_LEFT, 300);
    mTypesList->AppendColumn(_("Category"), wxLIST_FORMAT_LEFT, 150);

    sizer->Add(mTypesList, 1, wxALL | wxEXPAND, 5);

    // Boutons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* addBtn = new wxButton(panel, ID_ADD_TYPE, _("Add"));
    wxButton* editBtn = new wxButton(panel, ID_EDIT_TYPE, _("Edit"));
    wxButton* deleteBtn = new wxButton(panel, ID_DELETE_TYPE, _("Delete"));

    buttonSizer->Add(addBtn, 0, wxALL, 5);
    buttonSizer->Add(editBtn, 0, wxALL, 5);
    buttonSizer->Add(deleteBtn, 0, wxALL, 5);

    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, _("Transaction Types"));

    // Charger les types
    LoadTypes();

    // Connecter les événements
    addBtn->Bind(wxEVT_BUTTON, &PreferencesDialog::OnAdd, this);
    editBtn->Bind(wxEVT_BUTTON, &PreferencesDialog::OnEdit, this);
    deleteBtn->Bind(wxEVT_BUTTON, &PreferencesDialog::OnDelete, this);
    mTypesList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &PreferencesDialog::OnItemActivated, this);
}

void PreferencesDialog::CreateDisplayPage(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(3, 2, 10, 10);
    gridSizer->AddGrowableCol(1);

    // Format de date
    gridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Date Format:")),
                   0, wxALIGN_CENTER_VERTICAL);

    mDateFormatChoice = new wxChoice(panel, ID_DATE_FORMAT);
    mDateFormatChoice->Append("DD/MM/YYYY");
    mDateFormatChoice->Append("MM/DD/YYYY");
    mDateFormatChoice->Append("YYYY-MM-DD");

    Settings& settings = Settings::GetInstance();
    Settings::DateFormat currentFormat = settings.GetDateFormat();

    if (currentFormat == Settings::FORMAT_DD_MM_YYYY) {
        mDateFormatChoice->SetSelection(0);
    } else if (currentFormat == Settings::FORMAT_MM_DD_YYYY) {
        mDateFormatChoice->SetSelection(1);
    } else if (currentFormat == Settings::FORMAT_YYYY_MM_DD) {
        mDateFormatChoice->SetSelection(2);
    } else {
        mDateFormatChoice->SetSelection(0); // Default to DD/MM/YYYY
    }

    gridSizer->Add(mDateFormatChoice, 1, wxEXPAND);

    // Séparateur décimal
    gridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Decimal Separator:")),
                   0, wxALIGN_CENTER_VERTICAL);

    mDecimalSeparatorChoice = new wxChoice(panel, ID_DECIMAL_SEPARATOR);
    mDecimalSeparatorChoice->Append(_("Comma (,)"));
    mDecimalSeparatorChoice->Append(_("Point (.)"));

    if (settings.GetDecimalSeparator() == Settings::SEPARATOR_COMMA) {
        mDecimalSeparatorChoice->SetSelection(0);
    } else {
        mDecimalSeparatorChoice->SetSelection(1);
    }

    gridSizer->Add(mDecimalSeparatorChoice, 1, wxEXPAND);

    // Langue
    gridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Language:")),
                   0, wxALIGN_CENTER_VERTICAL);

    mLanguageChoice = new wxChoice(panel, ID_LANGUAGE);
    mLanguageChoice->Append(_("French"));
    mLanguageChoice->Append(_("English"));

    LanguageManager& langMgr = LanguageManager::GetInstance();
    wxLanguage currentLang = langMgr.GetCurrentLanguage();
    
    if (currentLang == wxLANGUAGE_FRENCH || currentLang == wxLANGUAGE_FRENCH_FRANCE) {
        mLanguageChoice->SetSelection(0);
    } else {
        mLanguageChoice->SetSelection(1);
    }

    gridSizer->Add(mLanguageChoice, 1, wxEXPAND);

    sizer->Add(gridSizer, 0, wxALL | wxEXPAND, 10);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, _("Display"));

    // Connecter les événements
    mDateFormatChoice->Bind(wxEVT_CHOICE, &PreferencesDialog::OnDateFormatChanged, this);
    mDecimalSeparatorChoice->Bind(wxEVT_CHOICE, &PreferencesDialog::OnDecimalSeparatorChanged, this);
    mLanguageChoice->Bind(wxEVT_CHOICE, &PreferencesDialog::OnLanguageChanged, this);
}

void PreferencesDialog::LoadTypes() {
    mTypesList->DeleteAllItems();

    auto types = mDatabase->GetAllTypes();

    for (size_t i = 0; i < types.size(); ++i) {
        long index = mTypesList->InsertItem(i, types[i].mNom);
        mTypesList->SetItem(index, 1, types[i].mIsDepense ? _("Expense") : _("Income"));
        mTypesList->SetItemData(index, static_cast<long>(i));
    }
}

void PreferencesDialog::OnAdd(wxCommandEvent& event) {
    wxDialog dialog(this, wxID_ANY, _("Add Transaction Type"),
                    wxDefaultPosition, wxSize(300, 150));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* label = new wxStaticText(&dialog, wxID_ANY, _("Type name:"));
    sizer->Add(label, 0, wxALL, 5);

    wxTextCtrl* nameText = new wxTextCtrl(&dialog, wxID_ANY);
    sizer->Add(nameText, 0, wxALL | wxEXPAND, 5);

    wxStaticText* categoryLabel = new wxStaticText(&dialog, wxID_ANY, _("Category:"));
    sizer->Add(categoryLabel, 0, wxALL, 5);

    wxChoice* categoryChoice = new wxChoice(&dialog, wxID_ANY);
    categoryChoice->Append(_("Expense"));
    categoryChoice->Append(_("Income"));
    categoryChoice->SetSelection(0);
    sizer->Add(categoryChoice, 0, wxALL | wxEXPAND, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(&dialog, wxID_OK, _("OK"));
    wxButton* cancelBtn = new wxButton(&dialog, wxID_CANCEL, _("Cancel"));
    buttonSizer->Add(okBtn, 0, wxALL, 5);
    buttonSizer->Add(cancelBtn, 0, wxALL, 5);

    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        wxString name = nameText->GetValue();
        if (name.IsEmpty()) {
            wxMessageBox(_("Please enter a name"), _("Error"), wxOK | wxICON_ERROR);
            return;
        }

        bool isDepense = (categoryChoice->GetSelection() == 0);
        if (mDatabase->AddType(name.ToStdString(), isDepense)) {
            LoadTypes();
        }
    }
}

void PreferencesDialog::OnDelete(wxCommandEvent& event) {
    long selectedItem = mTypesList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox(_("Please select a transaction"), _("Information"),
                     wxOK | wxICON_INFORMATION);
        return;
    }

    wxString typeName = mTypesList->GetItemText(selectedItem);

    if (wxMessageBox(_("Are you sure you want to delete this type?"),
                     _("Confirmation"), wxYES_NO | wxICON_QUESTION) == wxYES) {

        // Vérifier si le type est utilisé
        if (mDatabase->IsTypeUsed(typeName.ToStdString())) {
            wxMessageBox(_("This type is used by existing transactions and cannot be deleted."),
                        _("Warning"), wxOK | wxICON_WARNING);
            return;
        }

        if (mDatabase->DeleteType(typeName.ToStdString())) {
            LoadTypes();
        } else {
            wxMessageBox(_("Error deleting transaction"),
                        _("Error"), wxOK | wxICON_ERROR);
        }
    }
}

void PreferencesDialog::OnEdit(wxCommandEvent& event) {
    long selectedItem = mTypesList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox(_("Please select a transaction"), _("Information"),
                     wxOK | wxICON_INFORMATION);
        return;
    }

    wxString oldName = mTypesList->GetItemText(selectedItem);
    wxString categoryStr = mTypesList->GetItemText(selectedItem, 1);
    bool wasDepense = (categoryStr == _("Expense"));

    wxDialog dialog(this, wxID_ANY, _("Edit Transaction Type"),
                    wxDefaultPosition, wxSize(300, 150));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* label = new wxStaticText(&dialog, wxID_ANY, _("Type name:"));
    sizer->Add(label, 0, wxALL, 5);

    wxTextCtrl* nameText = new wxTextCtrl(&dialog, wxID_ANY, oldName);
    nameText->SetEditable(false);  // Disable renaming since UpdateType doesn't support it
    sizer->Add(nameText, 0, wxALL | wxEXPAND, 5);

    wxStaticText* categoryLabel = new wxStaticText(&dialog, wxID_ANY, _("Category:"));
    sizer->Add(categoryLabel, 0, wxALL, 5);

    wxChoice* categoryChoice = new wxChoice(&dialog, wxID_ANY);
    categoryChoice->Append(_("Expense"));
    categoryChoice->Append(_("Income"));
    categoryChoice->SetSelection(wasDepense ? 0 : 1);
    sizer->Add(categoryChoice, 0, wxALL | wxEXPAND, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(&dialog, wxID_OK, _("OK"));
    wxButton* cancelBtn = new wxButton(&dialog, wxID_CANCEL, _("Cancel"));
    buttonSizer->Add(okBtn, 0, wxALL, 5);
    buttonSizer->Add(cancelBtn, 0, wxALL, 5);

    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        bool isDepense = (categoryChoice->GetSelection() == 0);

        // Only update the category flag, not the name
        if (mDatabase->UpdateType(oldName.ToStdString(), isDepense)) {
            LoadTypes();
        } else {
            wxMessageBox(_("Error updating transaction"),
                        _("Error"), wxOK | wxICON_ERROR);
        }
    }
}

void PreferencesDialog::OnItemActivated(wxListEvent& event) {
    wxCommandEvent evt;
    OnEdit(evt);
}

void PreferencesDialog::OnDateFormatChanged(wxCommandEvent& event) {
    Settings& settings = Settings::GetInstance();
    settings.SetDateFormat(static_cast<Settings::DateFormat>(mDateFormatChoice->GetSelection()));
    mDateExample->SetLabel(settings.FormatDate(wxDateTime::Now()));
}

void PreferencesDialog::OnDecimalSeparatorChanged(wxCommandEvent& event) {
    Settings& settings = Settings::GetInstance();
    settings.SetDecimalSeparator(static_cast<Settings::DecimalSeparator>(mDecimalSeparatorChoice->GetSelection()));
    mMoneyExample->SetLabel(settings.FormatMoney(1234.56) + " €");
}

void PreferencesDialog::OnLanguageChanged(wxCommandEvent& event) {
    LanguageManager& langMgr = LanguageManager::GetInstance();
    wxLanguage newLang = (event.GetSelection() == 0) ? wxLANGUAGE_FRENCH : wxLANGUAGE_ENGLISH;

    if (langMgr.SetLanguage(newLang)) {
        wxMessageBox(_("Please restart the application for the language change to take effect."),
                    _("Restart required"), wxOK | wxICON_INFORMATION);
    }
}