//
// Created by Jean-Michel Frouin
//

#include "CSVImportDialog.h"
#include <wx/stattext.h>
#include <sstream>

wxBEGIN_EVENT_TABLE(CSVImportDialog, wxDialog)
    EVT_CHOICE(ID_CSV_DATE_CHOICE, CSVImportDialog::OnColumnChoiceChanged)
    EVT_CHOICE(ID_CSV_LIBELLE_CHOICE, CSVImportDialog::OnColumnChoiceChanged)
    EVT_CHOICE(ID_CSV_SOMME_CHOICE, CSVImportDialog::OnColumnChoiceChanged)
    EVT_CHOICE(ID_CSV_TYPE_CHOICE, CSVImportDialog::OnColumnChoiceChanged)
    EVT_RADIOBOX(ID_CSV_SEPARATOR_CHOICE, CSVImportDialog::OnColumnChoiceChanged)
    EVT_BUTTON(wxID_OK, CSVImportDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, CSVImportDialog::OnCancel)
wxEND_EVENT_TABLE()

CSVImportDialog::CSVImportDialog(wxWindow* parent, Database* database,
                                 const std::vector<std::string>& csvHeaders,
                                 const std::vector<std::vector<std::string>>& csvData)
    : wxDialog(parent, wxID_ANY, "Importation CSV - Mapping des champs",
               wxDefaultPosition, wxSize(700, 600)),
      mDatabase(database),
      mCSVHeaders(csvHeaders),
      mCSVData(csvData),
      mConfirmed(false) {

    // Initialiser le mapping par défaut
    mMapping.dateColumn = -1;
    mMapping.libelleColumn = -1;
    mMapping.sommeColumn = -1;
    mMapping.typeColumn = -1;
    mMapping.pointeeByDefault = false;
    mMapping.separator = ';';

    CreateControls();
}

void CSVImportDialog::CreateControls() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Instructions
    wxStaticText* instructions = new wxStaticText(this, wxID_ANY,
        "Mappez les colonnes du CSV avec les champs de l'application :");
    mainSizer->Add(instructions, 0, wxALL | wxEXPAND, 10);

    // Séparateur CSV
    wxArrayString separators;
    separators.Add("Point-virgule (;)");
    separators.Add("Virgule (,)");
    separators.Add("Tabulation");
    mSeparatorChoice = new wxRadioBox(this, ID_CSV_SEPARATOR_CHOICE, "Séparateur CSV",
                                      wxDefaultPosition, wxDefaultSize,
                                      separators, 1, wxRA_SPECIFY_ROWS);
    mSeparatorChoice->SetSelection(0);
    mainSizer->Add(mSeparatorChoice, 0, wxALL | wxEXPAND, 10);

    // Grille de mapping
    wxStaticBoxSizer* mappingSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Mapping des colonnes");
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(5, 2, 10, 10);
    gridSizer->AddGrowableCol(1);

    // Préparer les choix (colonnes CSV + "Non mappé")
    wxArrayString columnChoices;
    columnChoices.Add("-- Non mappé --");
    for (const auto& header : mCSVHeaders) {
        columnChoices.Add(wxString::FromUTF8(header));
    }

    // Date
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Date:"), 0, wxALIGN_CENTER_VERTICAL);
    mDateChoice = new wxChoice(this, ID_CSV_DATE_CHOICE, wxDefaultPosition, wxDefaultSize, columnChoices);
    mDateChoice->SetSelection(0);
    gridSizer->Add(mDateChoice, 1, wxEXPAND);

    // Libellé
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Libellé:"), 0, wxALIGN_CENTER_VERTICAL);
    mLibelleChoice = new wxChoice(this, ID_CSV_LIBELLE_CHOICE, wxDefaultPosition, wxDefaultSize, columnChoices);
    mLibelleChoice->SetSelection(0);
    gridSizer->Add(mLibelleChoice, 1, wxEXPAND);

    // Somme
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Somme:"), 0, wxALIGN_CENTER_VERTICAL);
    mSommeChoice = new wxChoice(this, ID_CSV_SOMME_CHOICE, wxDefaultPosition, wxDefaultSize, columnChoices);
    mSommeChoice->SetSelection(0);
    gridSizer->Add(mSommeChoice, 1, wxEXPAND);

    // Type (optionnel dans CSV)
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Type (optionnel):"), 0, wxALIGN_CENTER_VERTICAL);
    mTypeChoice = new wxChoice(this, ID_CSV_TYPE_CHOICE, wxDefaultPosition, wxDefaultSize, columnChoices);
    mTypeChoice->SetSelection(0);
    gridSizer->Add(mTypeChoice, 1, wxEXPAND);

    // Type par défaut
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Type par défaut:"), 0, wxALIGN_CENTER_VERTICAL);
    mDefaultTypeChoice = new wxChoice(this, wxID_ANY);
    auto types = mDatabase->GetAllTypes();
    for (const auto& type : types) {
        wxString displayName = type.mNom + (type.mIsDepense ? " (Dépense)" : " (Recette)");
        mDefaultTypeChoice->Append(displayName, new wxStringClientData(type.mNom));
    }
    if (!types.empty()) {
        mDefaultTypeChoice->SetSelection(0);
    }
    gridSizer->Add(mDefaultTypeChoice, 1, wxEXPAND);

    mappingSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);

    // Pointée par défaut
    mPointeeCheck = new wxCheckBox(this, wxID_ANY, "Marquer toutes les transactions comme pointées");
    mappingSizer->Add(mPointeeCheck, 0, wxALL | wxEXPAND, 5);

    mainSizer->Add(mappingSizer, 0, wxALL | wxEXPAND, 10);

    // Prévisualisation
    wxStaticBoxSizer* previewSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Aperçu (premières lignes)");
    mPreviewText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 200),
                                  wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    wxFont font(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    mPreviewText->SetFont(font);
    previewSizer->Add(mPreviewText, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(previewSizer, 1, wxALL | wxEXPAND, 10);

    // Boutons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(this, wxID_OK, "Importer");
    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Annuler");
    buttonSizer->Add(okBtn, 0, wxALL, 5);
    buttonSizer->Add(cancelBtn, 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    SetSizer(mainSizer);
    UpdatePreview();
}

void CSVImportDialog::OnColumnChoiceChanged(wxCommandEvent& event) {
    UpdatePreview();
}

void CSVImportDialog::UpdatePreview() {
    if (mCSVData.empty()) {
        mPreviewText->SetValue("Aucune donnée à prévisualiser");
        return;
    }

    std::ostringstream preview;
    preview << "Format attendu : Date | Libellé | Somme | Type\n";
    preview << "----------------------------------------\n\n";

    // Afficher jusqu'à 5 lignes
    size_t maxLines = std::min(size_t(5), mCSVData.size());

    for (size_t i = 0; i < maxLines; ++i) {
        const auto& row = mCSVData[i];

        int dateIdx = mDateChoice->GetSelection() - 1;
        int libelleIdx = mLibelleChoice->GetSelection() - 1;
        int sommeIdx = mSommeChoice->GetSelection() - 1;
        int typeIdx = mTypeChoice->GetSelection() - 1;

        std::string date = (dateIdx >= 0 && dateIdx < (int)row.size()) ? row[dateIdx] : "[NON MAPPÉ]";
        std::string libelle = (libelleIdx >= 0 && libelleIdx < (int)row.size()) ? row[libelleIdx] : "[NON MAPPÉ]";
        std::string somme = (sommeIdx >= 0 && sommeIdx < (int)row.size()) ? row[sommeIdx] : "[NON MAPPÉ]";
        std::string type = (typeIdx >= 0 && typeIdx < (int)row.size()) ? row[typeIdx] : "[TYPE PAR DÉFAUT]";

        preview << date << " | " << libelle << " | " << somme << " | " << type << "\n";
    }

    if (mCSVData.size() > 5) {
        preview << "\n... et " << (mCSVData.size() - 5) << " autres lignes";
    }

    mPreviewText->SetValue(wxString::FromUTF8(preview.str()));
}

void CSVImportDialog::OnOK(wxCommandEvent& event) {
    // Valider le mapping
    mMapping.dateColumn = mDateChoice->GetSelection() - 1;
    mMapping.libelleColumn = mLibelleChoice->GetSelection() - 1;
    mMapping.sommeColumn = mSommeChoice->GetSelection() - 1;
    mMapping.typeColumn = mTypeChoice->GetSelection() - 1;

    if (mMapping.dateColumn < 0 || mMapping.libelleColumn < 0 || mMapping.sommeColumn < 0) {
        wxMessageBox("Vous devez mapper au minimum les champs Date, Libellé et Somme",
                     "Mapping incomplet", wxOK | wxICON_WARNING);
        return;
    }

    // Récupérer le type par défaut
    if (mDefaultTypeChoice->GetSelection() != wxNOT_FOUND) {
        wxStringClientData* data = static_cast<wxStringClientData*>(
            mDefaultTypeChoice->GetClientObject(mDefaultTypeChoice->GetSelection())
        );
        mMapping.defaultType = data->GetData().ToStdString();
    }

    mMapping.pointeeByDefault = mPointeeCheck->GetValue();

    // Récupérer le séparateur
    switch (mSeparatorChoice->GetSelection()) {
        case 0: mMapping.separator = ';'; break;
        case 1: mMapping.separator = ','; break;
        case 2: mMapping.separator = '\t'; break;
        default: mMapping.separator = ';'; break;
    }

    mConfirmed = true;
    EndModal(wxID_OK);
}

void CSVImportDialog::OnCancel(wxCommandEvent& event) {
    mConfirmed = false;
    EndModal(wxID_CANCEL);
}