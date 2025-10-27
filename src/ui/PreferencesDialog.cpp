//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include "PreferencesDialog.h"

wxBEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
    EVT_BUTTON(ID_ADD_TYPE, PreferencesDialog::OnAdd)
    EVT_BUTTON(ID_DELETE_TYPE, PreferencesDialog::OnDelete)
wxEND_EVENT_TABLE()

PreferencesDialog::PreferencesDialog(wxWindow* parent, Database* database)
    : wxDialog(parent, wxID_ANY, "Préférences - Types de transactions",
               wxDefaultPosition, wxSize(400, 400)),
      mDatabase(database) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Liste des types
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "Types de transactions:");
    mainSizer->Add(label, 0, wxALL, 5);

    mTypeList = new wxListBox(this, wxID_ANY);
    mainSizer->Add(mTypeList, 1, wxALL | wxEXPAND, 5);

    // Ajouter un type
    wxBoxSizer* addSizer = new wxBoxSizer(wxHORIZONTAL);
    mNewTypeText = new wxTextCtrl(this, wxID_ANY);
    wxButton* addBtn = new wxButton(this, ID_ADD_TYPE, "Ajouter");
    addSizer->Add(mNewTypeText, 1, wxALL | wxEXPAND, 5);
    addSizer->Add(addBtn, 0, wxALL, 5);
    mainSizer->Add(addSizer, 0, wxEXPAND);

    // Boutons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* deleteBtn = new wxButton(this, ID_DELETE_TYPE, "Supprimer");
    wxButton* closeBtn = new wxButton(this, wxID_CLOSE, "Fermer");
    buttonSizer->Add(deleteBtn, 0, wxALL, 5);
    buttonSizer->Add(closeBtn, 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    SetSizer(mainSizer);
    LoadTypes();
}

void PreferencesDialog::LoadTypes() {
    mTypeList->Clear();
    auto types = mDatabase->GetAllTypes();
    for (const auto& type : types) {
        mTypeList->Append(type);
    }
}

void PreferencesDialog::OnAdd(wxCommandEvent& event) {
    wxString newType = mNewTypeText->GetValue().Trim();
    if (newType.IsEmpty()) {
        wxMessageBox("Veuillez entrer un nom de type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    if (mDatabase->AddType(newType.ToStdString())) {
        LoadTypes();
        mNewTypeText->Clear();
    } else {
        wxMessageBox("Erreur lors de l'ajout du type (peut-être existe-t-il déjà?)",
                     "Erreur", wxOK | wxICON_ERROR);
    }
}

void PreferencesDialog::OnDelete(wxCommandEvent& event) {
    int selection = mTypeList->GetSelection();
    if (selection == wxNOT_FOUND) {
        wxMessageBox("Veuillez sélectionner un type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    wxString type = mTypeList->GetString(selection);

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