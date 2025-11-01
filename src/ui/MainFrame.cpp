//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include "MainFrame.h"
#include "PreferencesDialog.h"
#include "InfoDialog.h"
#include "CSVImportDialog.h"
#include <wx/stattext.h>
#include <wx/datectrl.h>
#include <wx/progdlg.h>
#include <wx/srchctrl.h>
#include <fstream>
#include <sstream>

#include "RecurringDialog.h"
#include "core/version.h"
#include "core/Settings.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(ID_PREFERENCES, MainFrame::OnPreferences)
    EVT_MENU(ID_INFO, MainFrame::OnInfo)
    EVT_MENU(ID_IMPORT_CSV, MainFrame::OnImportCSV)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(ID_ADD_TRANSACTION, MainFrame::OnAddTransaction)
    EVT_MENU(ID_RAPPROCHEMENT, MainFrame::OnRapprochement)
    EVT_MENU(ID_HIDE_POINTEES, MainFrame::OnToggleHidePointees)
    EVT_MENU(ID_MANAGE_RECURRING, MainFrame::OnManageRecurring)  // NOUVEAU
    EVT_UPDATE_UI(ID_HIDE_POINTEES, MainFrame::OnUpdateToggleHidePointees)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_TRANSACTION_LIST, MainFrame::OnTransactionRightClick)
    EVT_LIST_COL_CLICK(ID_TRANSACTION_LIST, MainFrame::OnColumnClick)
    EVT_LIST_ITEM_CHECKED(ID_TRANSACTION_LIST, MainFrame::OnRapprochementItemChecked)
    EVT_LIST_ITEM_UNCHECKED(ID_TRANSACTION_LIST, MainFrame::OnRapprochementItemChecked)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(900, 600)),
        mSommeEnLigne(0.0), mSortColumn(-1), mSortAscending(true), mSearchText(""),
        mRapprochementMode(false), mHidePointees(false) {

    mDatabase = std::make_unique<Database>("mescomptes.db");
    if (!mDatabase->Open()) {
        wxMessageBox("Erreur lors de l'ouverture de la base de données",
                     "Erreur", wxOK | wxICON_ERROR);
    }
    
    // Exécuter les transactions récurrentes en attente
    int executedCount = mDatabase->ExecutePendingRecurringTransactions();
    if (executedCount > 0) {
        wxMessageBox(wxString::Format("%d transaction(s) récurrente(s) ajoutée(s)", executedCount),
                    "Transactions récurrentes", wxOK | wxICON_INFORMATION);
    }

    // Définir l'icône de l'application
    wxIcon icon("res/icon.png", wxBITMAP_TYPE_PNG);
    if (icon.IsOk()) {
        SetIcon(icon);
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
    menuFile->Append(ID_ADD_TRANSACTION, "&Nouvelle transaction\tCtrl-N",
                     "Ajouter une nouvelle transaction");
    menuFile->AppendSeparator();
    menuFile->Append(ID_IMPORT_CSV, "&Importer depuis CSV\tCtrl-I",
                     "Importer des transactions depuis un fichier CSV");
    menuFile->AppendSeparator();
    menuFile->Append(ID_MANAGE_RECURRING, "Gérer les &récurrences\tCtrl-T",
                     "Gérer les transactions récurrentes");
    menuFile->AppendSeparator();
    menuFile->Append(ID_PREFERENCES, "&Préférences\tCtrl-P",
                     "Gérer les types de transactions");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, "&Quitter\tCtrl-Q", "Quitter l'application");
    menuBar->Append(menuFile, "&Fichier");

    // Menu Affichage
    wxMenu* menuView = new wxMenu;
    menuView->AppendCheckItem(ID_HIDE_POINTEES, "&Masquer les transactions pointées\tCtrl-H",
                              "Masquer ou afficher les transactions pointées");
    menuBar->Append(menuView, "&Affichage");

    // Menu Opérations
    wxMenu* menuOperations = new wxMenu;
    menuOperations->Append(ID_RAPPROCHEMENT, "&Rapprochement bancaire\tCtrl-R",
                          "Effectuer un rapprochement bancaire");
    menuBar->Append(menuOperations, "&Opérations");

    // Menu Informations
    wxMenu* menuInfo = new wxMenu;
    menuInfo->Append(ID_INFO, "&Informations\tCtrl-I",
                     "Afficher les informations de la base");
    menuBar->Append(menuInfo, "I&nformations");

    // Menu Aide
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

    // Barre de recherche
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* searchLabel = new wxStaticText(panel, wxID_ANY, "Recherche:");
    searchSizer->Add(searchLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    mSearchBox = new wxSearchCtrl(panel, ID_SEARCH_BOX, wxEmptyString, 
                                   wxDefaultPosition, wxSize(300, -1));
    mSearchBox->ShowCancelButton(true);
    mSearchBox->SetDescriptiveText("Rechercher dans libellé ou montant...");
    searchSizer->Add(mSearchBox, 0, wxALIGN_CENTER_VERTICAL);
    
    mainSizer->Add(searchSizer, 0, wxALL, 5);

    // Liste des transactions
    mTransactionList = new wxListCtrl(panel, ID_TRANSACTION_LIST, wxDefaultPosition,
                                       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mTransactionList->AppendColumn("Date", wxLIST_FORMAT_LEFT, 120);
    mTransactionList->AppendColumn("Libellé", wxLIST_FORMAT_LEFT, 300);
    mTransactionList->AppendColumn("Somme", wxLIST_FORMAT_RIGHT, 100);
    mTransactionList->AppendColumn("Pointée", wxLIST_FORMAT_CENTER, 80);
    mTransactionList->AppendColumn("Date pointée", wxLIST_FORMAT_LEFT, 120);
    mTransactionList->AppendColumn("Type", wxLIST_FORMAT_LEFT, 120);

    mainSizer->Add(mTransactionList, 1, wxALL | wxEXPAND, 5);

    // Panneau de résumé avec disposition horizontale
    wxStaticBoxSizer* summarySizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Résumé");
    
    // Partie gauche (2x2)
    wxFlexGridSizer* leftGridSizer = new wxFlexGridSizer(2, 2, 5, 10);
    leftGridSizer->AddGrowableCol(1);

    leftGridSizer->Add(new wxStaticText(panel, wxID_ANY, "Restant:"), 0, wxALIGN_CENTER_VERTICAL);
    mRestantText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    leftGridSizer->Add(mRestantText, 1, wxEXPAND);

    leftGridSizer->Add(new wxStaticText(panel, wxID_ANY, "Somme pointée:"), 0, wxALIGN_CENTER_VERTICAL);
    mPointeeText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    leftGridSizer->Add(mPointeeText, 1, wxEXPAND);

    summarySizer->Add(leftGridSizer, 1, wxALL | wxEXPAND, 5);

    // Partie droite (2x2)
    wxFlexGridSizer* rightGridSizer = new wxFlexGridSizer(2, 2, 5, 10);
    rightGridSizer->AddGrowableCol(1);

    rightGridSizer->Add(new wxStaticText(panel, wxID_ANY, "Somme en ligne:"), 0, wxALIGN_CENTER_VERTICAL);
    mSommeEnLigneText = new wxTextCtrl(panel, ID_SOMME_EN_LIGNE, "0.00");
    rightGridSizer->Add(mSommeEnLigneText, 1, wxEXPAND);

    rightGridSizer->Add(new wxStaticText(panel, wxID_ANY, "Diff:"), 0, wxALIGN_CENTER_VERTICAL);
    mDiffText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition,
                                wxDefaultSize, wxTE_READONLY);
    rightGridSizer->Add(mDiffText, 1, wxEXPAND);

    summarySizer->Add(rightGridSizer, 1, wxALL | wxEXPAND, 5);

    mainSizer->Add(summarySizer, 0, wxALL | wxEXPAND, 5);

    panel->SetSizer(mainSizer);
}

void MainFrame::LoadTransactions() {
    mTransactionList->DeleteAllItems();

    Settings& settings = Settings::GetInstance();
    mAllTransactions = mDatabase->GetAllTransactions();
    
    // Appliquer le filtre de recherche
    FilterTransactions();

    // Appliquer le tri si une colonne est sélectionnée
    if (mSortColumn >= 0) {
        SortTransactions(mSortColumn);
    }
    
    long style = mTransactionList->GetWindowStyle();
    if (mRapprochementMode) {
        if (!(style & wxLC_REPORT)) {
            style |= wxLC_REPORT;
        }
        mTransactionList->SetWindowStyle(style);
        mTransactionList->EnableCheckBoxes(true);
    } else {
        mTransactionList->EnableCheckBoxes(false);
    }

    for (size_t i = 0; i < mCachedTransactions.size(); ++i) {
        const auto& trans = mCachedTransactions[i];
        long index = mTransactionList->InsertItem(i, settings.FormatDate(trans.GetDate()));
        mTransactionList->SetItem(index, 1, trans.GetLibelle());

        // Afficher avec signe + ou - selon le type
        bool isDepense = mDatabase->IsTypeDepense(trans.GetType());
        wxString sommeStr;
        if (isDepense) {
            sommeStr = "-" + settings.FormatMoney(trans.GetSomme());
            // Rouge pastel (salmon/coral)
            mTransactionList->SetItemTextColour(index, wxColour(220, 100, 100));
        } else {
            sommeStr = "+" + settings.FormatMoney(trans.GetSomme());
            // Vert pastel
            mTransactionList->SetItemTextColour(index, wxColour(100, 180, 120));
        }

        mTransactionList->SetItem(index, 2, sommeStr);
        mTransactionList->SetItem(index, 3, trans.IsPointee() ? "Oui" : "Non");

        // Afficher la date pointée si elle existe
        if (trans.IsPointee() && trans.GetDatePointee().IsValid()) {
            mTransactionList->SetItem(index, 4, settings.FormatDate(trans.GetDatePointee()));
        } else {
            mTransactionList->SetItem(index, 4, "");
        }

        mTransactionList->SetItem(index, 5, trans.GetType());
        mTransactionList->SetItemData(index, trans.GetId());
    }
}

void MainFrame::UpdateSummary() {
    Settings& settings = Settings::GetInstance();
    double restant = mDatabase->GetTotalRestant();
    double pointee = mDatabase->GetTotalPointee();
    double diff = pointee - (restant - mSommeEnLigne);

    mRestantText->SetValue(settings.FormatMoney(restant) + " €");
    mPointeeText->SetValue(settings.FormatMoney(pointee) + " €");
    mDiffText->SetValue(settings.FormatMoney(diff) + " €");
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

void MainFrame::ShowTransactionDialog(Transaction* existingTransaction) {
    bool isEdit = (existingTransaction != nullptr);
    wxString dialogTitle = isEdit ? "Modifier une transaction" : "Ajouter une transaction";

    wxDialog dialog(this, wxID_ANY, dialogTitle, wxDefaultPosition, wxSize(400, 300));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(5, 2, 5, 10);
    gridSizer->AddGrowableCol(1);

    // Date
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Date:"), 0, wxALIGN_CENTER_VERTICAL);
    wxDatePickerCtrl* datePicker = new wxDatePickerCtrl(&dialog, wxID_ANY);
    if (isEdit) {
        datePicker->SetValue(existingTransaction->GetDate());
    }
    gridSizer->Add(datePicker, 1, wxEXPAND);

    // Libellé
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Libellé:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* libelleText = new wxTextCtrl(&dialog, wxID_ANY);
    if (isEdit) {
        libelleText->SetValue(existingTransaction->GetLibelle());
    }
    gridSizer->Add(libelleText, 1, wxEXPAND);

    // Somme
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Somme:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* sommeText = new wxTextCtrl(&dialog, wxID_ANY);
    if (isEdit) {
        sommeText->SetValue(wxString::Format("%.2f", existingTransaction->GetSomme()));
    }
    gridSizer->Add(sommeText, 1, wxEXPAND);

    // Pointée
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Pointée:"), 0, wxALIGN_CENTER_VERTICAL);
    wxCheckBox* pointeeCheck = new wxCheckBox(&dialog, wxID_ANY, "");
    if (isEdit) {
        pointeeCheck->SetValue(existingTransaction->IsPointee());
    }
    gridSizer->Add(pointeeCheck, 1, wxEXPAND);

    // Type
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Type:"), 0, wxALIGN_CENTER_VERTICAL);
    wxChoice* typeChoice = new wxChoice(&dialog, wxID_ANY);
    auto types = mDatabase->GetAllTypes();
    int selectedIndex = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        wxString displayName = types[i].mNom + (types[i].mIsDepense ? " (Dépense)" : " (Recette)");
        typeChoice->Append(displayName, new wxStringClientData(types[i].mNom));

        if (isEdit && types[i].mNom == existingTransaction->GetType()) {
            selectedIndex = i;
        }
    }
    if (!types.empty()) {
        typeChoice->SetSelection(selectedIndex);
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
        if (isEdit) {
            trans.SetId(existingTransaction->GetId());
        }

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

        bool success = false;
        if (isEdit) {
            success = mDatabase->UpdateTransaction(trans);
        } else {
            success = mDatabase->AddTransaction(trans);
        }

        if (success) {
            LoadTransactions();
            UpdateSummary();
        } else {
            wxMessageBox(isEdit ? "Erreur lors de la modification de la transaction"
                                : "Erreur lors de l'ajout de la transaction",
                         "Erreur", wxOK | wxICON_ERROR);
        }
    }
}

void MainFrame::OnAddTransaction(wxCommandEvent& event) {
    ShowTransactionDialog(nullptr);
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

void MainFrame::OnManageRecurring(wxCommandEvent& event) {
    RecurringDialog dialog(this, mDatabase.get());
    dialog.ShowModal();

    // Recharger les transactions après la fermeture du dialogue
    // (au cas où des transactions récurrentes auraient été exécutées)
    LoadTransactions();
    UpdateSummary();
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
            bool newPointeeStatus = !trans.IsPointee();
            trans.SetPointee(newPointeeStatus);

            // Si on pointe la transaction, enregistrer la date actuelle
            if (newPointeeStatus) {
                trans.SetDatePointee(wxDateTime::Now());
            } else {
                // Si on dépointe, réinitialiser la date pointée
                trans.SetDatePointee(wxDateTime());
            }

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

void MainFrame::OnTransactionDoubleClick(wxListEvent& event) {
    long selectedItem = event.GetIndex();
    if (selectedItem == -1) {
        return;
    }

    int transactionId = mTransactionList->GetItemData(selectedItem);
    auto transactions = mDatabase->GetAllTransactions();

    for (auto& trans : transactions) {
        if (trans.GetId() == transactionId && !trans.IsPointee()) {
            ShowTransactionDialog(&trans);
            break;
        }
    }
}

void MainFrame::OnTransactionRightClick(wxListEvent& event) {
    long selectedItem = event.GetIndex();
    if (selectedItem == -1) {
        return;
    }

    // Sélectionner l'élément si ce n'est pas déjà fait
    mTransactionList->SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

    int transactionId = mTransactionList->GetItemData(selectedItem);

    // Récupérer la transaction pour vérifier son état
    auto transactions = mDatabase->GetAllTransactions();
    Transaction* currentTransaction = nullptr;

    for (auto& trans : transactions) {
        if (trans.GetId() == transactionId) {
            currentTransaction = &trans;
            break;
        }
    }

    if (currentTransaction == nullptr) {
        return;
    }

    // Créer le menu contextuel
    wxMenu contextMenu;

    // Option Éditer (seulement si non pointée)
    if (!currentTransaction->IsPointee()) {
        contextMenu.Append(wxID_EDIT, "Éditer");
        contextMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, [this, currentTransaction](wxCommandEvent&) {
            ShowTransactionDialog(currentTransaction);
        }, wxID_EDIT);
    }

    // Option Pointer/Dépointer
    if (currentTransaction->IsPointee()) {
        contextMenu.Append(ID_TOGGLE_POINTEE, "Dépointer");
    } else {
        contextMenu.Append(ID_TOGGLE_POINTEE, "Pointer");
    }
    contextMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::OnTogglePointee, this, ID_TOGGLE_POINTEE);

    contextMenu.AppendSeparator();

    // Option Supprimer
    contextMenu.Append(ID_DELETE_TRANSACTION, "Supprimer");
    contextMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::OnDeleteTransaction, this, ID_DELETE_TRANSACTION);

    // Afficher le menu à la position du curseur
    PopupMenu(&contextMenu);
}

void MainFrame::OnImportCSV(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Ouvrir un fichier CSV", "", "",
                                "Fichiers CSV (*.csv)|*.csv|Tous les fichiers (*.*)|*.*",
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString filePath = openFileDialog.GetPath();

    // Lire le fichier CSV
    std::ifstream file(filePath.ToStdString());
    if (!file.is_open()) {
        wxMessageBox("Impossible d'ouvrir le fichier", "Erreur", wxOK | wxICON_ERROR);
        return;
    }

    // Dialogue pour choisir le séparateur initialement
    wxArrayString separators;
    separators.Add("Point-virgule (;)");
    separators.Add("Virgule (,)");
    separators.Add("Tabulation");

    int sepChoice = wxGetSingleChoiceIndex("Sélectionnez le séparateur utilisé dans le CSV:",
                                           "Séparateur CSV", separators, this);
    if (sepChoice == -1) {
        return;
    }

    char separator = ';';
    switch (sepChoice) {
        case 0: separator = ';'; break;
        case 1: separator = ','; break;
        case 2: separator = '\t'; break;
    }

    // Parser le CSV
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> csvData;
    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, separator)) {
            // Supprimer les guillemets si présents
            if (!cell.empty() && cell.front() == '"' && cell.back() == '"') {
                cell = cell.substr(1, cell.length() - 2);
            }
            row.push_back(cell);
        }

        if (firstLine) {
            headers = row;
            firstLine = false;
        } else {
            csvData.push_back(row);
        }
    }
    file.close();

    if (headers.empty() || csvData.empty()) {
        wxMessageBox("Le fichier CSV est vide ou mal formaté", "Erreur", wxOK | wxICON_ERROR);
        return;
    }

    // Afficher le dialogue de mapping
    CSVImportDialog mappingDialog(this, mDatabase.get(), headers, csvData);
    if (mappingDialog.ShowModal() != wxID_OK || !mappingDialog.IsImportConfirmed()) {
        return;
    }

    auto mapping = mappingDialog.GetMapping();

    // Importer les données
    wxProgressDialog progress("Importation en cours",
                             "Importation des transactions...",
                             csvData.size(),
                             this,
                             wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    bool success = mDatabase->ImportTransactionsFromCSV(
        csvData,
        mapping.dateColumn,
        mapping.libelleColumn,
        mapping.sommeColumn,
        mapping.typeColumn,
        mapping.defaultType,
        mapping.pointeeByDefault
    );

    progress.Update(csvData.size());

    if (success) {
        wxMessageBox(wxString::Format("Importation réussie : %zu transactions importées",
                                     csvData.size()),
                    "Succès", wxOK | wxICON_INFORMATION);
        LoadTransactions();
        UpdateSummary();
    } else {
        wxMessageBox("L'importation s'est terminée avec des erreurs.\n"
                    "Certaines transactions n'ont pas pu être importées.",
                    "Attention", wxOK | wxICON_WARNING);
        LoadTransactions();
        UpdateSummary();
    }
}

void MainFrame::OnColumnClick(wxListEvent& event) {
    int column = event.GetColumn();

    // Si on clique sur la même colonne, inverser l'ordre
    if (column == mSortColumn) {
        mSortAscending = !mSortAscending;
    } else {
        mSortColumn = column;
        mSortAscending = true;
    }

    SortTransactions(column);

    // Mettre à jour les titres des colonnes avec l'indicateur de tri
    UpdateColumnHeaders();

    mTransactionList->DeleteAllItems();
    Settings& settings = Settings::GetInstance();

    for (size_t i = 0; i < mCachedTransactions.size(); ++i) {
        const auto& trans = mCachedTransactions[i];
        long index = mTransactionList->InsertItem(i, settings.FormatDate(trans.GetDate()));
        mTransactionList->SetItem(index, 1, trans.GetLibelle());

        // Afficher avec signe + ou - selon le type
        bool isDepense = mDatabase->IsTypeDepense(trans.GetType());
        wxString sommeStr;
        if (isDepense) {
            sommeStr = "-" + settings.FormatMoney(trans.GetSomme());
            mTransactionList->SetItemTextColour(index, wxColour(220, 100, 100));
        } else {
            sommeStr = "+" + settings.FormatMoney(trans.GetSomme());
            mTransactionList->SetItemTextColour(index, wxColour(100, 180, 120));
        }

        mTransactionList->SetItem(index, 2, sommeStr);
        mTransactionList->SetItem(index, 3, trans.IsPointee() ? "Oui" : "Non");

        if (trans.IsPointee() && trans.GetDatePointee().IsValid()) {
            mTransactionList->SetItem(index, 4, settings.FormatDate(trans.GetDatePointee()));
        } else {
            mTransactionList->SetItem(index, 4, "");
        }

        mTransactionList->SetItem(index, 5, trans.GetType());
        mTransactionList->SetItemData(index, trans.GetId());
    }
}

void MainFrame::SortTransactions(int column) {
    std::stable_sort(mCachedTransactions.begin(), mCachedTransactions.end(),
        [this, column](const Transaction& a, const Transaction& b) -> bool {
            bool result = false;

            switch (column) {
                case 0: // Date
                    result = a.GetDate() < b.GetDate();
                    break;

                case 1: // Libellé
                    result = a.GetLibelle() < b.GetLibelle();
                    break;

                case 2: // Somme
                    result = a.GetSomme() < b.GetSomme();
                    break;

                case 3: // Pointée
                    if (a.IsPointee() != b.IsPointee()) {
                        result = !a.IsPointee(); // Non pointées en premier
                    } else {
                        result = a.GetDate() < b.GetDate(); // Tri secondaire par date
                    }
                    break;

                case 4: // Date pointée
                    if (a.IsPointee() && b.IsPointee()) {
                        if (a.GetDatePointee().IsValid() && b.GetDatePointee().IsValid()) {
                            result = a.GetDatePointee() < b.GetDatePointee();
                        } else if (a.GetDatePointee().IsValid()) {
                            result = true;
                        } else if (b.GetDatePointee().IsValid()) {
                            result = false;
                        } else {
                            result = a.GetDate() < b.GetDate();
                        }
                    } else if (a.IsPointee()) {
                        result = true;
                    } else if (b.IsPointee()) {
                        result = false;
                    } else {
                        result = a.GetDate() < b.GetDate();
                    }
                    break;

                case 5: // Type
                    result = a.GetType() < b.GetType();
                    break;

                default:
                    result = a.GetId() < b.GetId();
                    break;
            }

            // Inverser le résultat si tri descendant
            return mSortAscending ? result : !result;
        });
}

void MainFrame::UpdateColumnHeaders() {
    // Titres de colonnes de base
    wxString columnTitles[] = {
        "Date",
        "Libellé",
        "Somme",
        "Pointée",
        "Date pointée",
        "Type"
    };

    // Mettre à jour chaque colonne
    for (int i = 0; i < 6; ++i) {
        wxListItem col;
        col.SetMask(wxLIST_MASK_TEXT);

        if (i == mSortColumn) {
            // Ajouter l'indicateur de tri à la colonne active
            wxString indicator = mSortAscending ? " ▲" : " ▼";
            col.SetText(columnTitles[i] + indicator);
        } else {
            col.SetText(columnTitles[i]);
        }

        mTransactionList->SetColumn(i, col);
    }
}

void MainFrame::OnSearchChanged(wxCommandEvent& event) {
    mSearchText = mSearchBox->GetValue();
    LoadTransactions();
}

void MainFrame::OnRapprochement(wxCommandEvent& event) {
    if (mRapprochementMode) {
        // Sortir du mode rapprochement
        if (wxMessageBox("Voulez-vous quitter le mode rapprochement?",
                        "Confirmation", wxYES_NO | wxICON_QUESTION) == wxYES) {
            ExitRapprochementMode();
        }
    } else {
        // Entrer en mode rapprochement
        EnterRapprochementMode();
    }
}

void MainFrame::EnterRapprochementMode() {
    // Dialogue pour saisir le solde en ligne
    wxTextEntryDialog dialog(this,
                            "Entrez le solde indiqué sur votre relevé bancaire:",
                            "Rapprochement bancaire",
                            wxString::Format("%.2f", mSommeEnLigne));

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    double solde;
    if (!dialog.GetValue().ToDouble(&solde)) {
        wxMessageBox("Valeur invalide", "Erreur", wxOK | wxICON_ERROR);
        return;
    }

    // Mettre à jour le solde en ligne
    mSommeEnLigne = solde;
    mSommeEnLigneText->SetValue(wxString::Format("%.2f", mSommeEnLigne));

    // Activer le mode rapprochement
    mRapprochementMode = true;

    // Désactiver l'option "Masquer pointées" pendant le rapprochement
    GetMenuBar()->Enable(ID_HIDE_POINTEES, false);

    // Modifier le titre de la fenêtre
    SetTitle("Mes Comptes - Mode Rapprochement");

    // Changer le label du menu
    GetMenuBar()->SetLabel(ID_RAPPROCHEMENT, "&Quitter le rapprochement\tCtrl-R");

    // Recharger les transactions (affichera uniquement les non pointées avec checkboxes)
    LoadTransactions();
    UpdateSummary();

    wxMessageBox("Mode rapprochement activé.\n\n"
                "Cochez les transactions qui apparaissent sur votre relevé bancaire.\n"
                "Seules les transactions non pointées sont affichées.",
                "Information", wxOK | wxICON_INFORMATION);
}

void MainFrame::ExitRapprochementMode() {
    mRapprochementMode = false;

    // Réactiver l'option "Masquer pointées"
    GetMenuBar()->Enable(ID_HIDE_POINTEES, true);

    // Restaurer le titre de la fenêtre
    SetTitle("Mes Comptes");

    // Restaurer le label du menu
    GetMenuBar()->SetLabel(ID_RAPPROCHEMENT, "&Rapprochement bancaire\tCtrl-R");

    // Recharger les transactions (affichera toutes les transactions sans checkboxes)
    LoadTransactions();
    UpdateSummary();
}

void MainFrame::OnRapprochementItemChecked(wxListEvent& event) {
    if (!mRapprochementMode) {
        return;
    }

    long index = event.GetIndex();
    int transactionId = mTransactionList->GetItemData(index);
    bool isChecked = mTransactionList->IsItemChecked(index);

    // Trouver la transaction et mettre à jour son état
    auto transactions = mDatabase->GetAllTransactions();
    for (auto& trans : transactions) {
        if (trans.GetId() == transactionId) {
            trans.SetPointee(isChecked);

            // Si on pointe la transaction, enregistrer la date actuelle
            if (isChecked) {
                trans.SetDatePointee(wxDateTime::Now());
            } else {
                trans.SetDatePointee(wxDateTime());
            }

            mDatabase->UpdateTransaction(trans);
            UpdateSummary();
            break;
        }
    }
}

void MainFrame::OnToggleHidePointees(wxCommandEvent& event) {
    mHidePointees = event.IsChecked();
    
    // Recharger les transactions avec le nouveau filtre
    LoadTransactions();
    
    // Afficher un message informatif
    if (mHidePointees) {
        SetStatusText("Transactions pointées masquées");
    } else {
        SetStatusText("Toutes les transactions affichées");
    }
}

void MainFrame::FilterTransactions() {
    mCachedTransactions.clear();

    if (mSearchText.IsEmpty()) {
        // En mode rapprochement OU si l'option "masquer pointées" est active, 
        // filtrer uniquement les transactions non pointées
        if (mRapprochementMode || mHidePointees) {
            for (const auto& trans : mAllTransactions) {
                if (!trans.IsPointee()) {
                    mCachedTransactions.push_back(trans);
                }
            }
        } else {
            // Pas de filtre, afficher toutes les transactions
            mCachedTransactions = mAllTransactions;
        }
    } else {
        // Filtrer par libellé ou montant
        wxString searchLower = mSearchText.Lower();

        for (const auto& trans : mAllTransactions) {
            // En mode rapprochement OU si l'option "masquer pointées" est active,
            // ignorer les transactions pointées
            if ((mRapprochementMode || mHidePointees) && trans.IsPointee()) {
                continue;
            }

            // Recherche dans le libellé
            wxString libelle = wxString(trans.GetLibelle()).Lower();
            if (libelle.Contains(searchLower)) {
                mCachedTransactions.push_back(trans);
                continue;
            }

            // Recherche dans le montant
            Settings& settings = Settings::GetInstance();
            wxString sommeStr = settings.FormatMoney(trans.GetSomme());
            if (sommeStr.Contains(searchLower)) {
                mCachedTransactions.push_back(trans);
                continue;
            }

            // Recherche avec le signe + ou -
            bool isDepense = mDatabase->IsTypeDepense(trans.GetType());
            wxString sommeWithSign = (isDepense ? "-" : "+") + sommeStr;
            if (sommeWithSign.Contains(searchLower)) {
                mCachedTransactions.push_back(trans);
            }
        }
    }
}

void MainFrame::OnUpdateToggleHidePointees(wxUpdateUIEvent& event) {
    event.Check(mHidePointees);
}