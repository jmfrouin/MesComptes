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
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <fstream>
#include <sstream>

#include "RecurringDialog.h"
#include "core/version.h"
#include "core/Settings.h"
#include "core/LanguageManager.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(ID_PREFERENCES, MainFrame::OnPreferences)
    EVT_MENU(ID_INFO, MainFrame::OnInfo)
    EVT_MENU(ID_IMPORT_CSV, MainFrame::OnImportCSV)
    EVT_MENU(ID_EXPORT_CSV, MainFrame::OnExportCSV)
    EVT_MENU(ID_BACKUP, MainFrame::OnBackup)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(ID_ADD_TRANSACTION, MainFrame::OnAddTransaction)
    EVT_MENU(ID_RAPPROCHEMENT, MainFrame::OnRapprochement)
    EVT_MENU(ID_HIDE_POINTEES, MainFrame::OnToggleHidePointees)
    EVT_MENU(ID_MANAGE_RECURRING, MainFrame::OnManageRecurring)
    EVT_UPDATE_UI(ID_HIDE_POINTEES, MainFrame::OnUpdateToggleHidePointees)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_TRANSACTION_LIST, MainFrame::OnTransactionRightClick)
    EVT_LIST_COL_CLICK(ID_TRANSACTION_LIST, MainFrame::OnColumnClick)
    EVT_LIST_ITEM_CHECKED(ID_TRANSACTION_LIST, MainFrame::OnRapprochementItemChecked)
    EVT_LIST_ITEM_UNCHECKED(ID_TRANSACTION_LIST, MainFrame::OnRapprochementItemChecked)
    EVT_LIST_ITEM_ACTIVATED(ID_TRANSACTION_LIST, MainFrame::OnTransactionDoubleClick)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(900, 600)),
        mSommeEnLigne(0.0), mSortColumn(-1), mSortAscending(true), mSearchText(""),
        mRapprochementMode(false), mHidePointees(false) {

    // Initialiser le gestionnaire de langues
    LanguageManager::GetInstance().Initialize(this);

    mDatabase = std::make_unique<Database>("mescomptes.db");
    if (!mDatabase->Open()) {
        wxMessageBox(_("Error opening database"),
                     _("Error"), wxOK | wxICON_ERROR);
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
    menuFile->Append(ID_ADD_TRANSACTION, _("&New Transaction\tCtrl-N"),
                     _("Add a new transaction"));
    menuFile->AppendSeparator();
    
    // Sous-menu Import/Export
    wxMenu* menuImportExport = new wxMenu;
    menuImportExport->Append(ID_IMPORT_CSV, _("&Import from CSV\tCtrl-I"),
                            _("Import transactions from a CSV file"));
    menuImportExport->Append(ID_EXPORT_CSV, _("&Export to CSV\tCtrl-E"),
                            _("Export transactions to a CSV file"));
    menuFile->AppendSubMenu(menuImportExport, _("&Import/Export"), 
                           _("Import or export data"));
    
    menuFile->AppendSeparator();
    menuFile->Append(ID_BACKUP, _("&Backup Account\tCtrl-S"),
                     _("Create a compressed backup of the account"));
    menuFile->AppendSeparator();
    menuFile->Append(ID_MANAGE_RECURRING, _("Manage &Recurring\tCtrl-T"),
                     _("Manage recurring transactions"));
    menuFile->AppendSeparator();
    menuFile->Append(ID_PREFERENCES, _("&Preferences\tCtrl-P"),
                     _("Manage transaction types"));
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, _("&Quit\tCtrl-Q"), _("Quit the application"));
    menuBar->Append(menuFile, _("&File"));

    // Menu Affichage
    wxMenu* menuView = new wxMenu;
    menuView->AppendCheckItem(ID_HIDE_POINTEES, _("&Hide Checked Transactions\tCtrl-H"),
                              _("Hide or show checked transactions"));
    menuBar->Append(menuView, _("&View"));

    // Menu Opérations
    wxMenu* menuOperations = new wxMenu;
    menuOperations->Append(ID_RAPPROCHEMENT, _("&Bank Reconciliation\tCtrl-R"),
                          _("Perform a bank reconciliation"));
    menuBar->Append(menuOperations, _("&Operations"));

    // Menu Informations
    wxMenu* menuInfo = new wxMenu;
    menuInfo->Append(ID_INFO, _("&Database Info\tCtrl-I"),
                     _("Show database information"));
    menuBar->Append(menuInfo, _("&Information"));

    // Menu Aide
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, _("&About\tF1"),
                     _("About Mes Comptes"));
    menuBar->Append(menuHelp, _("&Help"));

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
        _("Built with:\n• C++20\n• wxWidgets %s\n• SQLite3\n\n© 2025 Development Team - JM Frouin")
    );

    wxMessageBox(wxString::Format(aboutText, wxVERSION_STRING), 
                 _("About Mes Comptes"), wxOK | wxICON_INFORMATION, this);
}

void MainFrame::CreateControls() {
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Barre de recherche
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* searchLabel = new wxStaticText(panel, wxID_ANY, _("Search:"));
    searchSizer->Add(searchLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    mSearchBox = new wxSearchCtrl(panel, ID_SEARCH_BOX, wxEmptyString, 
                                   wxDefaultPosition, wxSize(300, -1));
    mSearchBox->ShowCancelButton(true);
    mSearchBox->SetDescriptiveText(_("Search in description or amount..."));
    searchSizer->Add(mSearchBox, 0, wxALIGN_CENTER_VERTICAL);
    
    mainSizer->Add(searchSizer, 0, wxALL, 5);

    // Liste des transactions
    mTransactionList = new wxListCtrl(panel, ID_TRANSACTION_LIST, wxDefaultPosition,
                                       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mTransactionList->AppendColumn(_("Date"), wxLIST_FORMAT_LEFT, 120);
    mTransactionList->AppendColumn(_("Description"), wxLIST_FORMAT_LEFT, 300);
    mTransactionList->AppendColumn(_("Amount"), wxLIST_FORMAT_RIGHT, 100);
    mTransactionList->AppendColumn(_("Checked"), wxLIST_FORMAT_CENTER, 80);
    mTransactionList->AppendColumn(_("Check Date"), wxLIST_FORMAT_LEFT, 120);
    mTransactionList->AppendColumn(_("Type"), wxLIST_FORMAT_LEFT, 120);

    mainSizer->Add(mTransactionList, 1, wxALL | wxEXPAND, 5);

    // Panneau de résumé avec disposition horizontale
    wxStaticBoxSizer* summarySizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, _("Summary"));
    
    // Partie gauche (2x2)
    wxFlexGridSizer* leftGridSizer = new wxFlexGridSizer(2, 2, 5, 10);
    leftGridSizer->AddGrowableCol(1);

    leftGridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Remaining:")), 0, wxALIGN_CENTER_VERTICAL);
    mRestantText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    leftGridSizer->Add(mRestantText, 1, wxEXPAND);

    leftGridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Checked Total:")), 0, wxALIGN_CENTER_VERTICAL);
    mPointeeText = new wxTextCtrl(panel, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    leftGridSizer->Add(mPointeeText, 1, wxEXPAND);

    summarySizer->Add(leftGridSizer, 1, wxALL | wxEXPAND, 5);

    // Partie droite (2x2)
    wxFlexGridSizer* rightGridSizer = new wxFlexGridSizer(2, 2, 5, 10);
    rightGridSizer->AddGrowableCol(1);

    rightGridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Online Balance:")), 0, wxALIGN_CENTER_VERTICAL);
    mSommeEnLigneText = new wxTextCtrl(panel, ID_SOMME_EN_LIGNE, "0.00");
    rightGridSizer->Add(mSommeEnLigneText, 1, wxEXPAND);

    rightGridSizer->Add(new wxStaticText(panel, wxID_ANY, _("Difference:")), 0, wxALIGN_CENTER_VERTICAL);
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
        mTransactionList->SetItem(index, 3, trans.IsPointee() ? _("Yes") : _("No"));

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
    bool isReadOnly = isEdit && existingTransaction->IsPointee();
    wxString dialogTitle = isReadOnly ? _("Transaction Details (read-only)") 
                         : isEdit ? _("Edit Transaction") 
                         : _("Add Transaction");

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
    datePicker->Enable(!isReadOnly);
    gridSizer->Add(datePicker, 1, wxEXPAND);

    // Libellé
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, _("Description:")), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* libelleText = new wxTextCtrl(&dialog, wxID_ANY, "", wxDefaultPosition, 
                                              wxDefaultSize, isReadOnly ? wxTE_READONLY : 0);
    if (isEdit) {
        libelleText->SetValue(existingTransaction->GetLibelle());
    }
    gridSizer->Add(libelleText, 1, wxEXPAND);

    // Somme
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Somme:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* sommeText = new wxTextCtrl(&dialog, wxID_ANY, "", wxDefaultPosition,
                                            wxDefaultSize, isReadOnly ? wxTE_READONLY : 0);
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
    pointeeCheck->Enable(!isReadOnly);
    gridSizer->Add(pointeeCheck, 1, wxEXPAND);

    // Type
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, _("Type:")), 0, wxALIGN_CENTER_VERTICAL);
    wxChoice* typeChoice = new wxChoice(&dialog, wxID_ANY);
    auto types = mDatabase->GetAllTypes();
    int selectedIndex = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        wxString displayName = types[i].mNom + (types[i].mIsDepense ? 
            wxString(" (") + _("Expense") + ")" : 
            wxString(" (") + _("Income") + ")");
        typeChoice->Append(displayName, new wxStringClientData(types[i].mNom));

        if (isEdit && types[i].mNom == existingTransaction->GetType()) {
            selectedIndex = i;
        }
    }
    if (!types.empty()) {
        typeChoice->SetSelection(selectedIndex);
    }
    typeChoice->Enable(!isReadOnly);
    gridSizer->Add(typeChoice, 1, wxEXPAND);

    sizer->Add(gridSizer, 1, wxALL | wxEXPAND, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    if (isReadOnly) {
        // Mode lecture seule : seulement un bouton Fermer
        wxButton* closeBtn = new wxButton(&dialog, wxID_CANCEL, _("Close"));
        buttonSizer->Add(closeBtn, 0, wxALL, 5);
    } else {
        // Mode édition : boutons OK et Annuler
        wxButton* okBtn = new wxButton(&dialog, wxID_OK, _("OK"));
        wxButton* cancelBtn = new wxButton(&dialog, wxID_CANCEL, _("Cancel"));
        buttonSizer->Add(okBtn, 0, wxALL, 5);
        buttonSizer->Add(cancelBtn, 0, wxALL, 5);
    }
    
    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK && !isReadOnly) {
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
            wxMessageBox(isEdit ? _("Error updating transaction")
                                : _("Error adding transaction"),
                         _("Error"), wxOK | wxICON_ERROR);
        }
    }
}

void MainFrame::OnAddTransaction(wxCommandEvent& event) {
    ShowTransactionDialog(nullptr);
}

void MainFrame::OnDeleteTransaction(wxCommandEvent& event) {
    long selectedItem = mTransactionList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) {
        wxMessageBox(_("Please select a transaction"), _("Information"), wxOK | wxICON_INFORMATION);
        return;
    }

    int transactionId = mTransactionList->GetItemData(selectedItem);

    if (wxMessageBox(_("Are you sure you want to delete this transaction?"),
                     _("Confirmation"), wxYES_NO | wxICON_QUESTION) == wxYES) {
        if (mDatabase->DeleteTransaction(transactionId)) {
            LoadTransactions();
            UpdateSummary();
        } else {
            wxMessageBox(_("Error deleting transaction"),
                         _("Error"), wxOK | wxICON_ERROR);
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
        wxMessageBox(_("Please select a transaction"), _("Information"), wxOK | wxICON_INFORMATION);
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
        if (trans.GetId() == transactionId) {
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
        contextMenu.Append(wxID_EDIT, _("Edit"));
        contextMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, [this, currentTransaction](wxCommandEvent&) {
            ShowTransactionDialog(currentTransaction);
        }, wxID_EDIT);
    }

    // Option Pointer/Dépointer
    if (currentTransaction->IsPointee()) {
        contextMenu.Append(ID_TOGGLE_POINTEE, _("Uncheck"));
    } else {
        contextMenu.Append(ID_TOGGLE_POINTEE, _("Check"));
    }
    contextMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::OnTogglePointee, this, ID_TOGGLE_POINTEE);

    contextMenu.AppendSeparator();

    // Option Supprimer
    contextMenu.Append(ID_DELETE_TRANSACTION, _("Delete"));
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

void MainFrame::OnExportCSV(wxCommandEvent& event) {
    // Dialogue de configuration de l'export
    wxDialog configDialog(this, wxID_ANY, _("CSV Export Options"), 
                         wxDefaultPosition, wxSize(450, 500));
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Séparateur
    wxStaticBoxSizer* separatorBox = new wxStaticBoxSizer(wxVERTICAL, &configDialog, _("Separator"));
    wxRadioButton* radioSemicolon = new wxRadioButton(&configDialog, wxID_ANY, 
                                                      _("Semicolon (;)"), 
                                                      wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    wxRadioButton* radioComma = new wxRadioButton(&configDialog, wxID_ANY, _("Comma (,)"));
    wxRadioButton* radioTab = new wxRadioButton(&configDialog, wxID_ANY, _("Tab"));
    radioSemicolon->SetValue(true);
    separatorBox->Add(radioSemicolon, 0, wxALL, 5);
    separatorBox->Add(radioComma, 0, wxALL, 5);
    separatorBox->Add(radioTab, 0, wxALL, 5);
    mainSizer->Add(separatorBox, 0, wxALL | wxEXPAND, 10);
    
    // Encodage
    wxStaticBoxSizer* encodingBox = new wxStaticBoxSizer(wxVERTICAL, &configDialog, _("Encoding"));
    wxRadioButton* radioUTF8 = new wxRadioButton(&configDialog, wxID_ANY, 
                                                 _("UTF-8 (recommended)"), 
                                                 wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    wxRadioButton* radioLatin1 = new wxRadioButton(&configDialog, wxID_ANY, _("ISO-8859-1 (Latin1)"));
    radioUTF8->SetValue(true);
    encodingBox->Add(radioUTF8, 0, wxALL, 5);
    encodingBox->Add(radioLatin1, 0, wxALL, 5);
    mainSizer->Add(encodingBox, 0, wxALL | wxEXPAND, 10);
    
    // Format de date
    wxStaticBoxSizer* dateBox = new wxStaticBoxSizer(wxVERTICAL, &configDialog, _("Date Format"));
    wxRadioButton* radioDateDMY = new wxRadioButton(&configDialog, wxID_ANY, 
                                                    _("DD/MM/YYYY"), 
                                                    wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    wxRadioButton* radioDateYMD = new wxRadioButton(&configDialog, wxID_ANY, _("YYYY-MM-DD (ISO)"));
    wxRadioButton* radioDateMDY = new wxRadioButton(&configDialog, wxID_ANY, _("MM/DD/YYYY"));
    radioDateDMY->SetValue(true);
    dateBox->Add(radioDateDMY, 0, wxALL, 5);
    dateBox->Add(radioDateYMD, 0, wxALL, 5);
    dateBox->Add(radioDateMDY, 0, wxALL, 5);
    mainSizer->Add(dateBox, 0, wxALL | wxEXPAND, 10);
    
    // Options supplémentaires
    wxStaticBoxSizer* optionsBox = new wxStaticBoxSizer(wxVERTICAL, &configDialog, _("Options"));
    wxCheckBox* checkHeader = new wxCheckBox(&configDialog, wxID_ANY, _("Include column headers"));
    wxCheckBox* checkQuotes = new wxCheckBox(&configDialog, wxID_ANY, _("Use quotes for text fields"));
    wxCheckBox* checkOnlyVisible = new wxCheckBox(&configDialog, wxID_ANY, _("Export only visible transactions"));
    wxCheckBox* checkSign = new wxCheckBox(&configDialog, wxID_ANY, _("Include +/- sign for amounts"));
    checkHeader->SetValue(true);
    checkQuotes->SetValue(true);
    checkOnlyVisible->SetValue(false);
    checkSign->SetValue(false);
    optionsBox->Add(checkHeader, 0, wxALL, 5);
    optionsBox->Add(checkQuotes, 0, wxALL, 5);
    optionsBox->Add(checkOnlyVisible, 0, wxALL, 5);
    optionsBox->Add(checkSign, 0, wxALL, 5);
    mainSizer->Add(optionsBox, 0, wxALL | wxEXPAND, 10);
    
    // Boutons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(&configDialog, wxID_OK, _("Export"));
    wxButton* cancelBtn = new wxButton(&configDialog, wxID_CANCEL, _("Cancel"));
    buttonSizer->Add(okBtn, 0, wxALL, 5);
    buttonSizer->Add(cancelBtn, 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 10);
    
    configDialog.SetSizer(mainSizer);
    
    if (configDialog.ShowModal() != wxID_OK) {
        return;
    }
    
    // Récupérer les options choisies
    char separator = ';';
    if (radioComma->GetValue()) separator = ',';
    else if (radioTab->GetValue()) separator = '\t';
    
    wxString dateFormat = "%d/%m/%Y";
    if (radioDateYMD->GetValue()) dateFormat = "%Y-%m-%d";
    else if (radioDateMDY->GetValue()) dateFormat = "%m/%d/%Y";
    
    bool includeHeader = checkHeader->GetValue();
    bool useQuotes = checkQuotes->GetValue();
    bool onlyVisible = checkOnlyVisible->GetValue();
    bool includeSign = checkSign->GetValue();
    bool useUTF8 = radioUTF8->GetValue();
    
    // Demander où sauvegarder le fichier
    wxFileDialog saveFileDialog(this, _("Export to CSV"), "", 
                               "export_transactions.csv",
                               _("CSV files (*.csv)|*.csv"),
                               wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    
    wxString filePath = saveFileDialog.GetPath();
    
    try {
        std::ofstream csvFile(filePath.ToStdString());
        if (!csvFile.is_open()) {
            wxMessageBox(_("Unable to create CSV file"), 
                        _("Error"), wxOK | wxICON_ERROR);
            return;
        }
        
        // Helper pour échapper les champs si nécessaire
        auto escapeField = [useQuotes, separator](const wxString& field) -> std::string {
            std::string result = field.ToStdString();
            if (useQuotes || result.find(separator) != std::string::npos || 
                result.find('"') != std::string::npos || result.find('\n') != std::string::npos) {
                // Doubler les guillemets existants
                size_t pos = 0;
                while ((pos = result.find('"', pos)) != std::string::npos) {
                    result.insert(pos, "\"");
                    pos += 2;
                }
                result = "\"" + result + "\"";
            }
            return result;
        };
        
        // Écrire l'en-tête si demandé
        if (includeHeader) {
            csvFile << escapeField(_("Date")) << separator
                   << escapeField(_("Description")) << separator
                   << escapeField(_("Amount")) << separator
                   << escapeField(_("Type")) << separator
                   << escapeField(_("Checked")) << separator
                   << escapeField(_("Check Date")) << "\n";
        }
        
        // Choisir les transactions à exporter
        std::vector<Transaction> transactionsToExport;
        if (onlyVisible) {
            transactionsToExport = mCachedTransactions;
        } else {
            transactionsToExport = mDatabase->GetAllTransactions();
            // Trier par date
            std::sort(transactionsToExport.begin(), transactionsToExport.end(),
                [](const Transaction& a, const Transaction& b) {
                    return a.GetDate() < b.GetDate();
                });
        }
        
        Settings& settings = Settings::GetInstance();
        
        // Écrire les transactions
        for (const auto& trans : transactionsToExport) {
            // Date
            wxString dateStr = trans.GetDate().Format(dateFormat);
            csvFile << escapeField(dateStr) << separator;
            
            // Libellé
            csvFile << escapeField(trans.GetLibelle()) << separator;
            
            // Montant
            wxString montantStr;
            bool isDepense = mDatabase->IsTypeDepense(trans.GetType());
            if (includeSign) {
                montantStr = (isDepense ? "-" : "+") + settings.FormatMoney(trans.GetSomme());
            } else {
                montantStr = settings.FormatMoney(trans.GetSomme());
                if (isDepense) {
                    montantStr = "-" + montantStr;
                }
            }
            csvFile << escapeField(montantStr) << separator;
            
            // Type
            csvFile << escapeField(trans.GetType()) << separator;
            
            // Pointée
            csvFile << escapeField(trans.IsPointee() ? _("Yes") : _("No")) << separator;
            
            // Date pointée
            if (trans.IsPointee() && trans.GetDatePointee().IsValid()) {
                wxString datePointeeStr = trans.GetDatePointee().Format(dateFormat);
                csvFile << escapeField(datePointeeStr);
            }
            
            csvFile << "\n";
        }
        
        csvFile.close();
        
        wxMessageBox(wxString::Format(_("CSV export successful!\n\nFile: %s\nTransactions exported: %zu"),
                                      filePath, transactionsToExport.size()),
                    _("Success"), wxOK | wxICON_INFORMATION);
        
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format(_("Export error: %s"), e.what()),
                    _("Error"), wxOK | wxICON_ERROR);
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
        _("Date"),
        _("Description"),
        _("Amount"),
        _("Checked"),
        _("Check Date"),
        _("Type")
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
        if (wxMessageBox(_("Do you want to exit reconciliation mode?"),
                        _("Confirmation"), wxYES_NO | wxICON_QUESTION) == wxYES) {
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
                            _("Enter the balance shown on your bank statement:"),
                            _("Bank Reconciliation"),
                            wxString::Format("%.2f", mSommeEnLigne));

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    double solde;
    if (!dialog.GetValue().ToDouble(&solde)) {
        wxMessageBox(_("Invalid value"), _("Error"), wxOK | wxICON_ERROR);
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
    SetTitle(_("My Accounts - Reconciliation Mode"));

    // Changer le label du menu
    GetMenuBar()->SetLabel(ID_RAPPROCHEMENT, _("&Exit Reconciliation\tCtrl-R"));

    // Recharger les transactions (affichera uniquement les non pointées avec checkboxes)
    LoadTransactions();
    UpdateSummary();

    wxMessageBox(_("Reconciliation mode enabled.\n\nCheck the transactions that appear on your bank statement.\nOnly unchecked transactions are displayed."),
                _("Information"), wxOK | wxICON_INFORMATION);
}

void MainFrame::ExitRapprochementMode() {
    mRapprochementMode = false;

    // Réactiver l'option "Masquer pointées"
    GetMenuBar()->Enable(ID_HIDE_POINTEES, true);

    // Restaurer le titre de la fenêtre
    SetTitle(_("My Accounts"));

    // Restaurer le label du menu
    GetMenuBar()->SetLabel(ID_RAPPROCHEMENT, _("&Bank Reconciliation\tCtrl-R"));

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
        SetStatusText(_("Checked transactions hidden"));
    } else {
        SetStatusText(_("All transactions displayed"));
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

void MainFrame::OnBackup(wxCommandEvent& event) {
    // Demander à l'utilisateur où sauvegarder le fichier
    wxFileDialog saveFileDialog(this, _("Backup Account"), "", 
                                "sauvegarde_compte.zip",
                                _("ZIP files (*.zip)|*.zip"),
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString zipPath = saveFileDialog.GetPath();
    wxString txtPath = zipPath;
    txtPath.Replace(".zip", ".txt");

    Settings& settings = Settings::GetInstance();

    try {
        // Créer le fichier texte de sauvegarde
        std::ofstream textFile(txtPath.ToStdString());
        if (!textFile.is_open()) {
            wxMessageBox(_("Unable to create backup file"), 
                        _("Error"), wxOK | wxICON_ERROR);
            return;
        }

        // Écrire l'en-tête
        textFile << "=================================================\n";
        textFile << "        SAUVEGARDE DU COMPTE - MesComptes\n";
        textFile << "=================================================\n";
        textFile << "Date de sauvegarde: " << wxDateTime::Now().Format("%d/%m/%Y %H:%M:%S").ToStdString() << "\n";
        textFile << "Version: " << MESCOMPTES::VERSION_STRING << "\n";
        textFile << "=================================================\n\n";

        // Récupérer toutes les transactions
        auto allTransactions = mDatabase->GetAllTransactions();

        // Trier par date
        std::sort(allTransactions.begin(), allTransactions.end(),
            [](const Transaction& a, const Transaction& b) {
                return a.GetDate() < b.GetDate();
            });

        // Écrire les statistiques
        textFile << "STATISTIQUES\n";
        textFile << "------------\n";
        textFile << "Nombre total de transactions: " << allTransactions.size() << "\n";
        textFile << "Restant: " << settings.FormatMoney(mDatabase->GetTotalRestant()).ToStdString() << " €\n";
        textFile << "Somme pointée: " << settings.FormatMoney(mDatabase->GetTotalPointee()).ToStdString() << " €\n";
        textFile << "\n\n";

        // Écrire les types de transactions
        textFile << "TYPES DE TRANSACTIONS\n";
        textFile << "---------------------\n";
        auto types = mDatabase->GetAllTypes();
        for (const auto& type : types) {
            textFile << "- " << type.mNom << " (" 
                    << (type.mIsDepense ? "Dépense" : "Recette") << ")\n";
        }
        textFile << "\n\n";

        // Écrire toutes les transactions
        textFile << "LISTE DES TRANSACTIONS\n";
        textFile << "======================\n\n";

        for (const auto& trans : allTransactions) {
            textFile << "Transaction #" << trans.GetId() << "\n";
            textFile << "  Date         : " << settings.FormatDate(trans.GetDate()).ToStdString() << "\n";
            textFile << "  Libellé      : " << trans.GetLibelle() << "\n";
            
            bool isDepense = mDatabase->IsTypeDepense(trans.GetType());
            textFile << "  Somme        : " << (isDepense ? "-" : "+") 
                    << settings.FormatMoney(trans.GetSomme()).ToStdString() << " €\n";
            
            textFile << "  Type         : " << trans.GetType() << "\n";
            textFile << "  Pointée      : " << (trans.IsPointee() ? "Oui" : "Non") << "\n";
            
            if (trans.IsPointee() && trans.GetDatePointee().IsValid()) {
                textFile << "  Date pointée : " << settings.FormatDate(trans.GetDatePointee()).ToStdString() << "\n";
            }
            
            textFile << "\n";
        }

        textFile << "=================================================\n";
        textFile << "           FIN DE LA SAUVEGARDE\n";
        textFile << "=================================================\n";

        textFile.close();

        // Compresser le fichier texte dans un ZIP
        wxFileOutputStream out(zipPath);
        if (!out.IsOk()) {
            wxMessageBox(_("Unable to create backup file"), 
                        _("Error"), wxOK | wxICON_ERROR);
            wxRemoveFile(txtPath);
            return;
        }

        wxZipOutputStream zip(out);
        if (!zip.IsOk()) {
            wxMessageBox(_("Unable to create backup file"), 
                        _("Error"), wxOK | wxICON_ERROR);
            wxRemoveFile(txtPath);
            return;
        }

        // Ajouter le fichier texte au ZIP
        wxFileInputStream in(txtPath);
        if (!in.IsOk()) {
            wxMessageBox(_("Unable to create backup file"), 
                        _("Error"), wxOK | wxICON_ERROR);
            wxRemoveFile(txtPath);
            return;
        }

        wxString filename = wxFileName(txtPath).GetFullName();
        zip.PutNextEntry(filename);
        zip.Write(in);

        zip.CloseEntry();
        zip.Close();
        out.Close();

        // Supprimer le fichier texte temporaire
        wxRemoveFile(txtPath);

        wxMessageBox(wxString::Format(_("Backup created successfully!\n\nFile: %s\nTransactions: %zu"),
                                      zipPath, allTransactions.size()),
                    _("Backup successful"), wxOK | wxICON_INFORMATION);

    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format(_("Export error: %s"), e.what()),
                    _("Error"), wxOK | wxICON_ERROR);
        // Nettoyer les fichiers temporaires
        if (wxFileExists(txtPath)) {
            wxRemoveFile(txtPath);
        }
    }
}