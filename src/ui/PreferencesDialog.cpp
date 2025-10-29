//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include <ui/PreferencesDialog.h>
#include <core/Settings.h>

wxBEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
    EVT_BUTTON(ID_ADD_TYPE, PreferencesDialog::OnAdd)
    EVT_BUTTON(ID_DELETE_TYPE, PreferencesDialog::OnDelete)
    EVT_BUTTON(ID_EDIT_TYPE, PreferencesDialog::OnEdit)
    EVT_LIST_ITEM_ACTIVATED(ID_TYPE_LIST, PreferencesDialog::OnItemActivated)
    EVT_CHOICE(ID_DATE_FORMAT, PreferencesDialog::OnDateFormatChanged)
    EVT_CHOICE(ID_DECIMAL_SEPARATOR, PreferencesDialog::OnDecimalSeparatorChanged)
wxEND_EVENT_TABLE()

PreferencesDialog::PreferencesDialog(wxWindow* parent, Database* database)
    : wxDialog(parent, wxID_ANY, "Préférences",
               wxDefaultPosition, wxSize(600, 500)),
      mDatabase(database) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Créer un notebook pour les différentes pages
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
    
    CreateTypesPage(notebook);
    CreateDisplayPage(notebook);
    
    mainSizer->Add(notebook, 1, wxALL | wxEXPAND, 10);

    // Bouton Fermer
    wxButton* closeBtn = new wxButton(this, wxID_CLOSE, "Fermer");
    mainSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(mainSizer);
}

void PreferencesDialog::CreateTypesPage(wxNotebook* notebook) {
    wxPanel* typesPanel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Liste des types
    wxStaticText* label = new wxStaticText(typesPanel, wxID_ANY, "Types de transactions:");
    sizer->Add(label, 0, wxALL, 5);

    mTypeList = new wxListCtrl(typesPanel, ID_TYPE_LIST, wxDefaultPosition,
                                wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mTypeList->AppendColumn("Nom", wxLIST_FORMAT_LEFT, 250);
    mTypeList->AppendColumn("Type", wxLIST_FORMAT_LEFT, 200);
    sizer->Add(mTypeList, 1, wxALL | wxEXPAND, 5);

    // Ajouter un type
    wxStaticBoxSizer* addSizer = new wxStaticBoxSizer(wxVERTICAL, typesPanel, "Ajouter un nouveau type");
    
    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    nameSizer->Add(new wxStaticText(typesPanel, wxID_ANY, "Nom:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    mNewTypeText = new wxTextCtrl(typesPanel, wxID_ANY);
    nameSizer->Add(mNewTypeText, 1, wxEXPAND);
    addSizer->Add(nameSizer, 0, wxALL | wxEXPAND, 5);

    wxArrayString choices;
    choices.Add("Dépense");
    choices.Add("Recette");
    mTypeRadio = new wxRadioBox(typesPanel, wxID_ANY, "Type",
                                 wxDefaultPosition, wxDefaultSize,
                                 choices, 1, wxRA_SPECIFY_COLS);
    mTypeRadio->SetSelection(0);
    addSizer->Add(mTypeRadio, 0, wxALL | wxEXPAND, 5);

    wxButton* addBtn = new wxButton(typesPanel, ID_ADD_TYPE, "Ajouter");
    addSizer->Add(addBtn, 0, wxALL | wxALIGN_CENTER, 5);

    sizer->Add(addSizer, 0, wxALL | wxEXPAND, 5);

    // Boutons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* editBtn = new wxButton(typesPanel, ID_EDIT_TYPE, "Modifier");
    wxButton* deleteBtn = new wxButton(typesPanel, ID_DELETE_TYPE, "Supprimer");
    buttonSizer->Add(editBtn, 0, wxALL, 5);
    buttonSizer->Add(deleteBtn, 0, wxALL, 5);
    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    typesPanel->SetSizer(sizer);
    notebook->AddPage(typesPanel, "Types de transactions");
    
    LoadTypes();
}

void PreferencesDialog::CreateDisplayPage(wxNotebook* notebook) {
    wxPanel* displayPanel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    Settings& settings = Settings::GetInstance();

    // Section Format de date
    wxStaticBoxSizer* dateSizer = new wxStaticBoxSizer(wxVERTICAL, displayPanel, "Format de date");
    
    wxBoxSizer* dateChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    dateChoiceSizer->Add(new wxStaticText(displayPanel, wxID_ANY, "Format:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    mDateFormatChoice = new wxChoice(displayPanel, ID_DATE_FORMAT);
    mDateFormatChoice->Append("AAAA-MM-JJ (2025-10-27)");
    mDateFormatChoice->Append("JJ/MM/AAAA (27/10/2025)");
    mDateFormatChoice->Append("JJ/MM/AA (27/10/25)");
    mDateFormatChoice->Append("MM/JJ/AAAA (10/27/2025)");
    mDateFormatChoice->Append("JJ Mois AAAA (27 Oct 2025)");
    mDateFormatChoice->SetSelection(static_cast<int>(settings.GetDateFormat()));
    dateChoiceSizer->Add(mDateFormatChoice, 1, wxEXPAND);
    
    dateSizer->Add(dateChoiceSizer, 0, wxALL | wxEXPAND, 5);
    
    wxBoxSizer* dateExampleSizer = new wxBoxSizer(wxHORIZONTAL);
    dateExampleSizer->Add(new wxStaticText(displayPanel, wxID_ANY, "Exemple:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    mDateExample = new wxStaticText(displayPanel, wxID_ANY, settings.FormatDate(wxDateTime::Now()));
    mDateExample->SetFont(mDateExample->GetFont().Bold());
    dateExampleSizer->Add(mDateExample, 1, wxEXPAND);
    dateSizer->Add(dateExampleSizer, 0, wxALL | wxEXPAND, 5);
    
    sizer->Add(dateSizer, 0, wxALL | wxEXPAND, 10);

    // Section Séparateur décimal
    wxStaticBoxSizer* decimalSizer = new wxStaticBoxSizer(wxVERTICAL, displayPanel, "Format monétaire");
    
    wxBoxSizer* decimalChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    decimalChoiceSizer->Add(new wxStaticText(displayPanel, wxID_ANY, "Séparateur:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    mDecimalSeparatorChoice = new wxChoice(displayPanel, ID_DECIMAL_SEPARATOR);
    mDecimalSeparatorChoice->Append("Virgule (1 234,56 €)");
    mDecimalSeparatorChoice->Append("Point (1,234.56 €)");
    mDecimalSeparatorChoice->SetSelection(static_cast<int>(settings.GetDecimalSeparator()));
    decimalChoiceSizer->Add(mDecimalSeparatorChoice, 1, wxEXPAND);
    
    decimalSizer->Add(decimalChoiceSizer, 0, wxALL | wxEXPAND, 5);
    
    wxBoxSizer* moneyExampleSizer = new wxBoxSizer(wxHORIZONTAL);
    moneyExampleSizer->Add(new wxStaticText(displayPanel, wxID_ANY, "Exemple:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    mMoneyExample = new wxStaticText(displayPanel, wxID_ANY, settings.FormatMoney(1234.56) + " €");
    mMoneyExample->SetFont(mMoneyExample->GetFont().Bold());
    moneyExampleSizer->Add(mMoneyExample, 1, wxEXPAND);
    decimalSizer->Add(moneyExampleSizer, 0, wxALL | wxEXPAND, 5);
    
    sizer->Add(decimalSizer, 0, wxALL | wxEXPAND, 10);

    displayPanel->SetSizer(sizer);
    notebook->AddPage(displayPanel, "Affichage");
}

void PreferencesDialog::LoadTypes() {
    mTypeList->DeleteAllItems();
    auto types = mDatabase->GetAllTypes();
    
    for (size_t i = 0; i < types.size(); ++i) {
        long index = mTypeList->InsertItem(i, types[i].mNom);
        mTypeList->SetItem(index, 1, types[i].mIsDepense ? "Dépense" : "Recette");
    }
}

void PreferencesDialog::OnAdd(wxCommandEvent& event) {
    wxString newType = mNewTypeText->GetValue().Trim();
    if (newType.IsEmpty()) {
        wxMessageBox("Veuillez entrer un nom de type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    bool isDepense = (mTypeRadio->GetSelection() == 0);

    if (mDatabase->AddType(newType.ToStdString(), isDepense)) {
        LoadTypes();
        mNewTypeText->Clear();
        mTypeRadio->SetSelection(0);
    } else {
        wxMessageBox("Erreur lors de l'ajout du type (peut-être existe-t-il déjà?)",
                     "Erreur", wxOK | wxICON_ERROR);
    }
}

void PreferencesDialog::OnDelete(wxCommandEvent& event) {
    long selectedItem = mTypeList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox("Veuillez sélectionner un type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    wxListItem item;
    item.SetId(selectedItem);
    item.SetColumn(0);
    item.SetMask(wxLIST_MASK_TEXT);
    mTypeList->GetItem(item);
    wxString type = item.GetText();
    
    if (wxMessageBox("Êtes-vous sûr de vouloir supprimer ce type?",
                     "Confirmation", wxYES_NO | wxICON_QUESTION) == wxYES) {
        if (mDatabase->DeleteType(type.ToStdString())) {
            LoadTypes();
        } else {
            wxMessageBox("Erreur lors de la suppression du type",
                         "Erreur", wxOK | wxICON_ERROR);
        }
    }
}

void PreferencesDialog::OnEdit(wxCommandEvent& event) {
    long selectedItem = mTypeList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox("Veuillez sélectionner un type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    wxListItem item;
    item.SetId(selectedItem);
    item.SetColumn(0);
    item.SetMask(wxLIST_MASK_TEXT);
    mTypeList->GetItem(item);
    wxString typeName = item.GetText();

    item.SetColumn(1);
    mTypeList->GetItem(item);
    wxString typeStr = item.GetText();
    bool currentIsDepense = (typeStr == "Dépense");

    wxDialog dialog(this, wxID_ANY, "Modifier le type", wxDefaultPosition, wxSize(300, 150));
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* label = new wxStaticText(&dialog, wxID_ANY, "Type: " + typeName);
    sizer->Add(label, 0, wxALL, 10);

    wxArrayString choices;
    choices.Add("Dépense");
    choices.Add("Recette");
    wxRadioBox* radio = new wxRadioBox(&dialog, wxID_ANY, "Modifier en",
                                        wxDefaultPosition, wxDefaultSize,
                                        choices, 1, wxRA_SPECIFY_COLS);
    radio->SetSelection(currentIsDepense ? 0 : 1);
    sizer->Add(radio, 0, wxALL | wxEXPAND, 10);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(&dialog, wxID_OK, "OK");
    wxButton* cancelBtn = new wxButton(&dialog, wxID_CANCEL, "Annuler");
    btnSizer->Add(okBtn, 0, wxALL, 5);
    btnSizer->Add(cancelBtn, 0, wxALL, 5);
    sizer->Add(btnSizer, 0, wxALIGN_CENTER);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        bool isDepense = (radio->GetSelection() == 0);
        if (mDatabase->UpdateType(typeName.ToStdString(), isDepense)) {
            LoadTypes();
        } else {
            wxMessageBox("Erreur lors de la modification du type",
                         "Erreur", wxOK | wxICON_ERROR);
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