//
// Created by Jean-Michel Frouin on 01/11/2025.
//

#include "RecurringDialog.h"
#include <wx/datectrl.h>
#include <wx/stattext.h>

wxBEGIN_EVENT_TABLE(RecurringDialog, wxDialog)
    EVT_BUTTON(ID_ADD_RECURRING, RecurringDialog::OnAdd)
    EVT_BUTTON(ID_EDIT_RECURRING, RecurringDialog::OnEdit)
    EVT_BUTTON(ID_DELETE_RECURRING, RecurringDialog::OnDelete)
    EVT_BUTTON(ID_EXECUTE_NOW, RecurringDialog::OnExecuteNow)
    EVT_LIST_ITEM_SELECTED(ID_RECURRING_LIST, RecurringDialog::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(ID_RECURRING_LIST, RecurringDialog::OnItemDeselected)
wxEND_EVENT_TABLE()

RecurringDialog::RecurringDialog(wxWindow* parent, Database* db)
    : wxDialog(parent, wxID_ANY, "Gestion des transactions récurrentes",
               wxDefaultPosition, wxSize(800, 500)), mDatabase(db) {
    CreateControls();
    LoadRecurringTransactions();
}

void RecurringDialog::CreateControls() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Liste des transactions récurrentes
    mRecurringList = new wxListCtrl(this, ID_RECURRING_LIST, wxDefaultPosition,
                                     wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mRecurringList->AppendColumn("Libellé", wxLIST_FORMAT_LEFT, 200);
    mRecurringList->AppendColumn("Montant", wxLIST_FORMAT_RIGHT, 100);
    mRecurringList->AppendColumn("Type", wxLIST_FORMAT_LEFT, 120);
    mRecurringList->AppendColumn("Récurrence", wxLIST_FORMAT_LEFT, 100);
    mRecurringList->AppendColumn("Début", wxLIST_FORMAT_LEFT, 100);
    mRecurringList->AppendColumn("Prochaine", wxLIST_FORMAT_LEFT, 100);
    mRecurringList->AppendColumn("Actif", wxLIST_FORMAT_CENTER, 60);

    mainSizer->Add(mRecurringList, 1, wxALL | wxEXPAND, 10);

    // Boutons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* addBtn = new wxButton(this, ID_ADD_RECURRING, "Ajouter");
    mEditBtn = new wxButton(this, ID_EDIT_RECURRING, "Modifier");
    mDeleteBtn = new wxButton(this, ID_DELETE_RECURRING, "Supprimer");
    mExecuteBtn = new wxButton(this, ID_EXECUTE_NOW, "Exécuter maintenant");

    mEditBtn->Enable(false);
    mDeleteBtn->Enable(false);
    mExecuteBtn->Enable(false);

    buttonSizer->Add(addBtn, 0, wxALL, 5);
    buttonSizer->Add(mEditBtn, 0, wxALL, 5);
    buttonSizer->Add(mDeleteBtn, 0, wxALL, 5);
    buttonSizer->Add(mExecuteBtn, 0, wxALL, 5);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(new wxButton(this, wxID_OK, "Fermer"), 0, wxALL, 5);

    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 10);
    SetSizer(mainSizer);
}

void RecurringDialog::LoadRecurringTransactions() {
    mRecurringList->DeleteAllItems();
    auto transactions = mDatabase->GetAllRecurringTransactions();

    for (size_t i = 0; i < transactions.size(); ++i) {
        const auto& trans = transactions[i];
        long index = mRecurringList->InsertItem(i, trans.GetLibelle());

        mRecurringList->SetItem(index, 1, wxString::Format("%.2f €", trans.GetSomme()));
        mRecurringList->SetItem(index, 2, trans.GetType());
        mRecurringList->SetItem(index, 3, RecurrenceTypeToString(trans.GetRecurrence()));
        mRecurringList->SetItem(index, 4, trans.GetStartDate().FormatISODate());
        mRecurringList->SetItem(index, 5, trans.GetNextExecutionDate().FormatISODate());
        mRecurringList->SetItem(index, 6, trans.IsActive() ? "Oui" : "Non");

        mRecurringList->SetItemData(index, trans.GetId());

        // Colorer selon l'état actif/inactif
        if (!trans.IsActive()) {
            mRecurringList->SetItemTextColour(index, *wxLIGHT_GREY);
        }
    }
}

wxString RecurringDialog::RecurrenceTypeToString(RecurrenceType type) {
    switch (type) {
        case RecurrenceType::DAILY: return "Quotidien";
        case RecurrenceType::WEEKLY: return "Hebdomadaire";
        case RecurrenceType::MONTHLY: return "Mensuel";
        case RecurrenceType::YEARLY: return "Annuel";
        default: return "Inconnu";
    }
}

void RecurringDialog::OnAdd(wxCommandEvent& event) {
    ShowRecurringDialog(nullptr);
}

void RecurringDialog::OnEdit(wxCommandEvent& event) {
    long selectedItem = mRecurringList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) return;

    int id = mRecurringList->GetItemData(selectedItem);
    auto transactions = mDatabase->GetAllRecurringTransactions();

    for (auto& trans : transactions) {
        if (trans.GetId() == id) {
            ShowRecurringDialog(&trans);
            break;
        }
    }
}

void RecurringDialog::OnDelete(wxCommandEvent& event) {
    long selectedItem = mRecurringList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) return;

    int id = mRecurringList->GetItemData(selectedItem);

    if (wxMessageBox("Êtes-vous sûr de vouloir supprimer cette récurrence?",
                     "Confirmation", wxYES_NO | wxICON_QUESTION) == wxYES) {
        if (mDatabase->DeleteRecurringTransaction(id)) {
            LoadRecurringTransactions();
        }
    }
}

void RecurringDialog::OnExecuteNow(wxCommandEvent& event) {
    long selectedItem = mRecurringList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == -1) return;

    int id = mRecurringList->GetItemData(selectedItem);
    auto transactions = mDatabase->GetAllRecurringTransactions();

    for (auto& trans : transactions) {
        if (trans.GetId() == id) {
            Transaction newTrans;
            newTrans.SetDate(wxDateTime::Today());
            newTrans.SetLibelle(trans.GetLibelle());
            newTrans.SetSomme(trans.GetSomme());
            newTrans.SetType(trans.GetType());
            newTrans.SetPointee(false);

            if (mDatabase->AddTransaction(newTrans)) {
                trans.SetLastExecuted(wxDateTime::Today());
                mDatabase->UpdateRecurringTransaction(trans);

                wxMessageBox("Transaction ajoutée avec succès", "Succès",
                           wxOK | wxICON_INFORMATION);
                LoadRecurringTransactions();
            }
            break;
        }
    }
}

void RecurringDialog::OnItemSelected(wxListEvent& event) {
    mEditBtn->Enable(true);
    mDeleteBtn->Enable(true);
    mExecuteBtn->Enable(true);
}

void RecurringDialog::OnItemDeselected(wxListEvent& event) {
    mEditBtn->Enable(false);
    mDeleteBtn->Enable(false);
    mExecuteBtn->Enable(false);
}

void RecurringDialog::ShowRecurringDialog(RecurringTransaction* existing) {
    bool isEdit = (existing != nullptr);
    wxDialog dialog(this, wxID_ANY,
                   isEdit ? "Modifier une récurrence" : "Ajouter une récurrence",
                   wxDefaultPosition, wxSize(450, 450));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(8, 2, 5, 10);
    gridSizer->AddGrowableCol(1);

    // Libellé
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Libellé:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* libelleText = new wxTextCtrl(&dialog, wxID_ANY);
    if (isEdit) libelleText->SetValue(existing->GetLibelle());
    gridSizer->Add(libelleText, 1, wxEXPAND);

    // Somme
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Montant:"), 0, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* sommeText = new wxTextCtrl(&dialog, wxID_ANY);
    if (isEdit) sommeText->SetValue(wxString::Format("%.2f", existing->GetSomme()));
    gridSizer->Add(sommeText, 1, wxEXPAND);

    // Type
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Type:"), 0, wxALIGN_CENTER_VERTICAL);
    wxChoice* typeChoice = new wxChoice(&dialog, wxID_ANY);
    auto types = mDatabase->GetAllTypes();
    int selectedIndex = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        wxString displayName = types[i].mNom + (types[i].mIsDepense ? " (Dépense)" : " (Recette)");
        typeChoice->Append(displayName, new wxStringClientData(types[i].mNom));
        if (isEdit && types[i].mNom == existing->GetType()) {
            selectedIndex = i;
        }
    }
    if (!types.empty()) typeChoice->SetSelection(selectedIndex);
    gridSizer->Add(typeChoice, 1, wxEXPAND);

    // Récurrence
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Récurrence:"), 0, wxALIGN_CENTER_VERTICAL);
    wxChoice* recurrenceChoice = new wxChoice(&dialog, wxID_ANY);
    recurrenceChoice->Append("Quotidien");
    recurrenceChoice->Append("Hebdomadaire");
    recurrenceChoice->Append("Mensuel");
    recurrenceChoice->Append("Annuel");
    if (isEdit) {
        recurrenceChoice->SetSelection(static_cast<int>(existing->GetRecurrence()));
    } else {
        recurrenceChoice->SetSelection(2); // Mensuel par défaut
    }
    gridSizer->Add(recurrenceChoice, 1, wxEXPAND);

    // Jour du mois (pour mensuel)
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Jour du mois:"), 0, wxALIGN_CENTER_VERTICAL);
    wxSpinCtrl* dayOfMonthSpin = new wxSpinCtrl(&dialog, wxID_ANY);
    dayOfMonthSpin->SetRange(1, 31);
    if (isEdit) {
        dayOfMonthSpin->SetValue(existing->GetDayOfMonth());
    } else {
        dayOfMonthSpin->SetValue(1);
    }
    gridSizer->Add(dayOfMonthSpin, 1, wxEXPAND);

    // Date de début
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Date de début:"), 0, wxALIGN_CENTER_VERTICAL);
    wxDatePickerCtrl* startDatePicker = new wxDatePickerCtrl(&dialog, wxID_ANY);
    if (isEdit) {
        startDatePicker->SetValue(existing->GetStartDate());
    } else {
        startDatePicker->SetValue(wxDateTime::Today());
    }
    gridSizer->Add(startDatePicker, 1, wxEXPAND);

    // Date de fin
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Date de fin:"), 0, wxALIGN_CENTER_VERTICAL);
    wxCheckBox* hasEndDateCheck = new wxCheckBox(&dialog, wxID_ANY, "Définir une date de fin");
    gridSizer->Add(hasEndDateCheck, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, ""), 0);
    wxDatePickerCtrl* endDatePicker = new wxDatePickerCtrl(&dialog, wxID_ANY);
    endDatePicker->Enable(false);
    if (isEdit && existing->GetEndDate().IsValid()) {
        hasEndDateCheck->SetValue(true);
        endDatePicker->SetValue(existing->GetEndDate());
        endDatePicker->Enable(true);
    }
    gridSizer->Add(endDatePicker, 1, wxEXPAND);

    hasEndDateCheck->Bind(wxEVT_CHECKBOX, [endDatePicker](wxCommandEvent& evt) {
        endDatePicker->Enable(evt.IsChecked());
    });

    // Actif
    gridSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Actif:"), 0, wxALIGN_CENTER_VERTICAL);
    wxCheckBox* activeCheck = new wxCheckBox(&dialog, wxID_ANY, "");
    activeCheck->SetValue(isEdit ? existing->IsActive() : true);
    gridSizer->Add(activeCheck, 1, wxEXPAND);

    sizer->Add(gridSizer, 1, wxALL | wxEXPAND, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(new wxButton(&dialog, wxID_OK, "OK"), 0, wxALL, 5);
    buttonSizer->Add(new wxButton(&dialog, wxID_CANCEL, "Annuler"), 0, wxALL, 5);
    sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        RecurringTransaction trans;
        if (isEdit) trans.SetId(existing->GetId());

        trans.SetLibelle(libelleText->GetValue().ToStdString());

        double somme;
        if (sommeText->GetValue().ToDouble(&somme)) {
            trans.SetSomme(somme);
        }

        wxStringClientData* data = static_cast<wxStringClientData*>(
            typeChoice->GetClientObject(typeChoice->GetSelection())
        );
        trans.SetType(data->GetData().ToStdString());

        trans.SetRecurrence(static_cast<RecurrenceType>(recurrenceChoice->GetSelection()));
        trans.SetDayOfMonth(dayOfMonthSpin->GetValue());
        trans.SetStartDate(startDatePicker->GetValue());

        if (hasEndDateCheck->GetValue()) {
            trans.SetEndDate(endDatePicker->GetValue());
        }

        trans.SetActive(activeCheck->GetValue());

        if (isEdit) {
            trans.SetLastExecuted(existing->GetLastExecuted());
        }

        bool success = isEdit ? mDatabase->UpdateRecurringTransaction(trans)
                             : mDatabase->AddRecurringTransaction(trans);

        if (success) {
            LoadRecurringTransactions();
        } else {
            wxMessageBox("Erreur lors de l'opération", "Erreur", wxOK | wxICON_ERROR);
        }
    }
}