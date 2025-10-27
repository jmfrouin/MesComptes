//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include "MainFrame.h"
#include "PreferencesDialog.h"
#include "InfoDialog.h"
#include <wx/stattext.h>
#include <wx/datectrl.h>

#include "core/version.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(ID_PREFERENCES, MainFrame::OnPreferences)
    EVT_MENU(ID_INFO, MainFrame::OnInfo)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_BUTTON(ID_ADD_TRANSACTION, MainFrame::OnAddTransaction)
    EVT_BUTTON(ID_DELETE_TRANSACTION, MainFrame::OnDeleteTransaction)
    EVT_BUTTON(ID_TOGGLE_POINTEE, MainFrame::OnTogglePointee)
    EVT_TEXT(ID_SOMME_EN_LIGNE, MainFrame::OnSommeEnLigneChanged)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(900, 600)),
      mSommeEnLigne(0.0) {

    mDatabase = std::make_unique<Database>("mescomptes.db");
    if (!mDatabase->Open()) {
        wxMessageBox("Erreur lors de l'ouverture de la base de données",
                     "Erreur", wxOK | wxICON_ERROR);
    }

    CreateMenuBar();
    CreateControls();
    LoadTransactions();
    UpdateSummary();
}

MainFrame::~MainFrame() {
    if (mDatabase) {
        mDatabase->Close();
    }
}

void MainFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar;

    // Menu Fichier
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_PREFERENCES, "&Préférences\tCtrl-P",
                     "Gérer les types de transactions");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, "&Quitter\tCtrl-Q", "Quitter l'application");
    menuBar->Append(menuFile, "&Fichier");

    // Menu Informations
    wxMenu* menuInfo = new wxMenu;
    menuInfo->Append(ID_INFO, "&Informations\tCtrl-I",
                     "Afficher les informations de la base");
    menuBar->Append(menuInfo, "I&nformations");

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "À &propos\tF1",
                     "À propos de Mes Comptes");
    menuBar->Append(menuHelp, "&Aide");

    SetMenuBar(menuBar);
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxString aboutText = wxString::Format(
        "MesComptes\n\n"
        "Version %s\n\n"
        "Built with:\n"
        "• C++20\n"
        "• wxWidgets %s\n"
        "• SQLite3\n\n"
        "© 2025 Development Team - JM Frouin",
        MESCOMPTES::VERSION_STRING,
        wxVERSION_STRING
    );

    wxMessageBox(aboutText, "About MesComptes", wxOK | wxICON_INFORMATION, this);
}

void MainFrame::CreateControls() {
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Boutons d'action
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* addBtn = new wxButton(panel, ID_ADD_TRANSACTION, "Ajouter");
    wxButton* deleteBtn = new wxButton(panel, ID_DELETE_TRANSACTION, "Supprimer");
    wxButton* toggleBtn = new wxButton(panel, ID_TOGGLE_POINTEE, "Pointer/Dépointer");

    buttonSizer->Add(addBtn, 0, wxALL, 5);
    buttonSizer->Add(deleteBtn, 0, wxALL, 5);
    buttonSizer->Add(toggleBtn, 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 5);

    // Liste des transactions
    mTransactionList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mTransactionList->AppendColumn("Date", wxLIST_FORMAT_LEFT, 120);
    mTransactionList->AppendColumn("Libellé", wxLIST_FORMAT_LEFT, 300);
    mTransactionList->AppendColumn("Somme", wxLIST_FORMAT_RIGHT, 100);
    mTransactionList->AppendColumn("Pointée", wxLIST_FORMAT_CENTER, 80);
    mTransactionList->AppendColumn("Type", wxLIST_FORMAT_LEFT, 120);

    mainSizer->Add(mTransactionList, 1, wxALL | wxEXPAND, 5);

    // Panneau de résumé
    wxStaticBoxSizer* summarySizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Résumé");
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(4, 2, 5, 10);
    gridSizer->AddGrowableCol(1);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Restant:"), 0, wxALIGN_CENTER_VERTICAL);
    mRestantText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition,
                                   wxDefaultSize, wxTE_READONLY);
    gridSizer->Add(mRestantText, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Somme pointée:"), 0, wxALIGN_CENTER_VERTICAL);
    mPointeeText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition,
                                   wxDefaultSize, wxTE_READONLY);
    gridSizer->Add(mPointeeText, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Somme en ligne:"), 0, wxALIGN_CENTER_VERTICAL);
    mSommeEnLigneText = new wxTextCtrl(panel, ID_SOMME_EN_LIGNE, "0.00");
    gridSizer->Add(mSommeEnLigneText, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Diff:"), 0, wxALIGN_CENTER_VERTICAL);
    mDiffText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition,
                                wxDefaultSize, wxTE_READONLY);
    gridSizer->Add(mDiffText, 1, wxEXPAND);

    summarySizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(summarySizer, 0, wxALL | wxEXPAND, 5);

    panel->SetSizer(mainSizer);
}

void MainFrame::LoadTransactions() {
    mTransactionList->DeleteAllItems();
    
    auto transactions = mDatabase->GetAllTransactions();
    for (size_t i = 0; i < transactions.size(); ++i) {
        const auto& trans = transactions[i];
        long index = mTransactionList->InsertItem(i, trans.GetDate().Format("%Y-%m-%d"));
        mTransactionList->SetItem(index, 1, trans.GetLibelle());
        
        // Afficher avec signe + ou - selon le type
        bool isDepense = mDatabase->IsTypeDepense(trans.GetType());
        wxString sommeStr;
        if (isDepense) {
            sommeStr = wxString::Format("-%.2f", trans.GetSomme());
            mTransactionList->SetItemTextColour(index, *wxRED);
        } else {
            sommeStr = wxString::Format("+%.2f", trans.GetSomme());
            mTransactionList->SetItemTextColour(index, wxColour(0, 128, 0)); // Vert foncé
        }
        
        mTransactionList->SetItem(index, 2, sommeStr);
        mTransactionList->SetItem(index, 3, trans.IsPointee() ? "Oui" : "Non");
        mTransactionList->SetItem(index, 4, trans.GetType());
        mTransactionList->SetItemData(index, trans.GetId());
    }
}

void MainFrame::UpdateSummary() {
    double restant = mDatabase->GetTotalRestant();
    double pointee = mDatabase->GetTotalPointee();
    double diff = pointee - (restant - mSommeEnLigne);

    mRestantText->SetValue(wxString::Format("%.2f €", restant));
    mPointeeText->SetValue(wxString::Format("%.2f €", pointee));
    mDiffText->SetValue(wxString::Format("%.2f €", diff));
}

void MainFrame::OnQuit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnPreferences(wxCommandEvent& event) {
    PreferencesDialog dialog(this, mDatabase.get());
    dialog.ShowModal();
}

void MainFrame::OnInfo(wxCommandEvent& event) {
    InfoDialog dialog(this, mDatabase.get());
    dialog.ShowModal();
}

void MainFrame::OnAddTransaction(wxCommandEvent& event) {
    wxDialog dialog(this, wxID_ANY, "Ajouter une transaction", wxDefaultPosition, wxSize(400, 300));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(5, 2, 5, 10);
    gridSizer->AddGrowableCol(1);

    // Date
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Date:"), 0, wxALIGN_CENTER_VERTICAL);
    wxDatePickerCtrl* datePicker = new wxDatePickerCtrl(&dialog, wxID_ANY);
    gridSizer->Add(datePicker, 1, wxEXPAND);

    // Libellé
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Libellé:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* libelleText = new wxTextCtrl(&dialog, wxID_ANY);
    gridSizer->Add(libelleText, 1, wxEXPAND);

    // Somme
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Somme:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* sommeText = new wxTextCtrl(&dialog, wxID_ANY);
    gridSizer->Add(sommeText, 1, wxEXPAND);

    // Pointée
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Pointée:"), 0, wxALIGN_CENTER_VERTICAL);
    wxCheckBox* pointeeCheck = new wxCheckBox(&dialog, wxID_ANY, "");
    gridSizer->Add(pointeeCheck, 1, wxEXPAND);

    // Type
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Type:"), 0, wxALIGN_CENTER_VERTICAL);
    wxChoice* typeChoice = new wxChoice(&dialog, wxID_ANY);
    auto types = mDatabase->GetAllTypes();
    for (const auto& type : types) {
        wxString displayName = type.mNom + (type.mIsDepense ? " (Dépense)" : " (Recette)");
        typeChoice->Append(displayName, new wxStringClientData(type.mNom));
    }
    if (!types.empty()) {
        typeChoice->SetSelection(0);
    }
    gridSizer->Add(typeChoice, 1, wxEXPAND);

    sizer->Add(gridSizer, 1, wxALL | wxEXPAND, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(&dialog, wxID_OK, "OK");
    wxButton* cancelBtn = new wxButton(&dialog, wxID_CANCEL, "Annuler");
    buttonSizer->Add(okBtn, 0, wxALL, 5);
    buttonSizer->Add(cancelBtn, 0, wxALL, 5);
    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        Transaction trans;
        trans.SetDate(datePicker->GetValue());
        trans.SetLibelle(libelleText->GetValue().ToStdString());

        double somme;
        if (sommeText->GetValue().ToDouble(&somme)) {
            trans.SetSomme(somme);
        }

        trans.SetPointee(pointeeCheck->GetValue());
        
        // Récupérer le nom réel du type (sans le suffixe)
        wxStringClientData* data = static_cast<wxStringClientData*>(
            typeChoice->GetClientObject(typeChoice->GetSelection())
        );
        trans.SetType(data->GetData().ToStdString());

        if (mDatabase->AddTransaction(trans)) {
            LoadTransactions();
            UpdateSummary();
        } else {
            wxMessageBox("Erreur lors de l'ajout de la transaction",
                         "Erreur", wxOK | wxICON_ERROR);
        }
    }
}

void MainFrame::OnDeleteTransaction(wxCommandEvent& event) {
    long selectedItem = mTransactionList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox("Veuillez sélectionner une transaction", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    int transactionId = mTransactionList->GetItemData(selectedItem);

    if (wxMessageBox("Êtes-vous sûr de vouloir supprimer cette transaction?",
                     "Confirmation", wxYES_NO | wxICON_QUESTION) == wxYES) {
        if (mDatabase->DeleteTransaction(transactionId)) {
            LoadTransactions();
            UpdateSummary();
        } else {
            wxMessageBox("Erreur lors de la suppression",
                         "Erreur", wxOK | wxICON_ERROR);
        }
    }
}

void MainFrame::OnTogglePointee(wxCommandEvent& event) {
    long selectedItem = mTransactionList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox("Veuillez sélectionner une transaction", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    int transactionId = mTransactionList->GetItemData(selectedItem);
    auto transactions = mDatabase->GetAllTransactions();

    for (auto& trans : transactions) {
        if (trans.GetId() == transactionId) {
            trans.SetPointee(!trans.IsPointee());
            mDatabase->UpdateTransaction(trans);
            LoadTransactions();
            UpdateSummary();
            break;
        }
    }
}

void MainFrame::OnSommeEnLigneChanged(wxCommandEvent& event) {
    double somme;
    if (mSommeEnLigneText->GetValue().ToDouble(&somme)) {
        mSommeEnLigne = somme;
        UpdateSummary();
    }
}