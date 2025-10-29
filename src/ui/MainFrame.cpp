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
#include <fstream>
#include <sstream>

#include "core/version.h"
#include "core/Settings.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(ID_PREFERENCES, MainFrame::OnPreferences)
    EVT_MENU(ID_INFO, MainFrame::OnInfo)
    EVT_MENU(ID_IMPORT_CSV, MainFrame::OnImportCSV)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(ID_ADD_TRANSACTION, MainFrame::OnAddTransaction)
    EVT_TEXT(ID_SOMME_EN_LIGNE, MainFrame::OnSommeEnLigneChanged)
    EVT_LIST_ITEM_ACTIVATED(ID_TRANSACTION_LIST, MainFrame::OnTransactionDoubleClick)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_TRANSACTION_LIST, MainFrame::OnTransactionRightClick)
    EVT_LIST_COL_CLICK(ID_TRANSACTION_LIST, MainFrame::OnColumnClick)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(900, 600)),
        mSommeEnLigne(0.0), mSortColumn(-1), mSortAscending(true) {

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
    menuFile->Append(ID_ADD_TRANSACTION, "&Nouvelle transaction\tCtrl-N",
                     "Ajouter une nouvelle transaction");
    menuFile->AppendSeparator();
    menuFile->Append(ID_IMPORT_CSV, "&Importer depuis CSV\tCtrl-I",
                     "Importer des transactions depuis un fichier CSV");
    menuFile->AppendSeparator();
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

    // Panneau de résumé
    wxStaticBoxSizer* summarySizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Résumé");
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(4, 2, 5, 10);
    gridSizer->AddGrowableCol(1);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Restant:"), 0, wxALIGN_CENTER_VERTICAL);
    mRestantText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    gridSizer->Add(mRestantText, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Somme pointée:"), 0, wxALIGN_CENTER_VERTICAL);
    mPointeeText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
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

    Settings& settings = Settings::GetInstance();
    mCachedTransactions = mDatabase->GetAllTransactions();

    // Appliquer le tri si une colonne est sélectionnée
    if (mSortColumn >= 0) {
        SortTransactions(mSortColumn);
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
    if (mCachedTransactions.empty()) {
        return;
    }

    // Créer un map pour stocker les types (dépense/recette) pour éviter les accès répétés à la DB
    std::unordered_map<std::string, bool> typeCache;
    for (const auto& trans : mCachedTransactions) {
        if (typeCache.find(trans.GetType()) == typeCache.end()) {
            typeCache[trans.GetType()] = mDatabase->IsTypeDepense(trans.GetType());
        }
    }

    bool sortAscending = mSortAscending;

    std::sort(mCachedTransactions.begin(), mCachedTransactions.end(),
        [&typeCache, column, sortAscending](const Transaction& a, const Transaction& b) -> bool {
            bool result = false;

            switch (column) {
                case 0: // Date
                    result = a.GetDate().IsEarlierThan(b.GetDate());
                    break;

                case 1: // Libellé
                    result = a.GetLibelle() < b.GetLibelle();
                    break;

                case 2: { // Somme
                    // Prendre en compte le type (dépense/recette) pour le tri
                    bool aIsDepense = typeCache.at(a.GetType());
                    bool bIsDepense = typeCache.at(b.GetType());
                    double aAmount = aIsDepense ? -a.GetSomme() : a.GetSomme();
                    double bAmount = bIsDepense ? -b.GetSomme() : b.GetSomme();
                    result = aAmount < bAmount;
                    break;
                }

                case 3: // Pointée
                    result = (!a.IsPointee() && b.IsPointee());
                    break;

                case 4: { // Date pointée
                    // Les dates invalides vont en fin
                    bool aValid = a.GetDatePointee().IsValid();
                    bool bValid = b.GetDatePointee().IsValid();

                    if (!aValid && !bValid) {
                        result = false;
                    } else if (!aValid) {
                        result = false;
                    } else if (!bValid) {
                        result = true;
                    } else {
                        result = a.GetDatePointee().IsEarlierThan(b.GetDatePointee());
                    }
                    break;
                }

                case 5: // Type
                    result = a.GetType() < b.GetType();
                    break;

                default:
                    result = false;
            }

            // Inverser si tri décroissant
            return sortAscending ? result : !result;
        });
}