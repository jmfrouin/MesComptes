//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include "InfoDialog.h"

InfoDialog::InfoDialog(wxWindow* parent, Database* database)
    : wxDialog(parent, wxID_ANY, "Informations de la base de données",
               wxDefaultPosition, wxSize(500, 300)),
      mDatabase(database) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl* infoText = new wxTextCtrl(this, wxID_ANY, "",
                                           wxDefaultPosition, wxDefaultSize,
                                           wxTE_MULTILINE | wxTE_READONLY);

    wxString info = mDatabase->GetDatabaseInfo();
    infoText->SetValue(info);

    mainSizer->Add(infoText, 1, wxALL | wxEXPAND, 10);

    wxButton* closeBtn = new wxButton(this, wxID_CLOSE, "Fermer");
    mainSizer->Add(closeBtn, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(mainSizer);
}