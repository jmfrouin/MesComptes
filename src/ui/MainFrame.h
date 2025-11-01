//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <core/Database.h>
#include <memory>

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

private:
    void CreateMenuBar();
    void CreateControls();
    void LoadTransactions();
    void UpdateSummary();

    // Event handlers
    void OnQuit(wxCommandEvent& event);
    void OnPreferences(wxCommandEvent& event);
    void OnInfo(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnAddTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnImportCSV(wxCommandEvent& event);
    void OnTogglePointee(wxCommandEvent& event);
    void OnSommeEnLigneChanged(wxCommandEvent& event);
    void OnTransactionDoubleClick(wxListEvent& event);
    void OnTransactionRightClick(wxListEvent& event);
    void OnColumnClick(wxListEvent& event);
    void OnSearchChanged(wxCommandEvent& event);


    // Helper methods
    void ShowTransactionDialog(Transaction* existingTransaction = nullptr);
    void SortTransactions(int column);
    void UpdateColumnHeaders();
    void FilterTransactions();

    // Widgets
    wxListCtrl* mTransactionList;
    wxTextCtrl* mRestantText;
    wxTextCtrl* mPointeeText;
    wxTextCtrl* mSommeEnLigneText;
    wxTextCtrl* mDiffText;
    wxSearchCtrl* mSearchBox;

    // Database
    std::unique_ptr<Database> mDatabase;
    double mSommeEnLigne;

    // Sorting
    int mSortColumn;
    bool mSortAscending;
    std::vector<Transaction> mCachedTransactions;
    std::vector<Transaction> mAllTransactions;
    wxString mSearchText;

    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum {
    ID_PREFERENCES = wxID_HIGHEST + 1,
    ID_INFO,
    ID_ADD_TRANSACTION,
    ID_IMPORT_CSV,
    ID_DELETE_TRANSACTION,
    ID_TOGGLE_POINTEE,
    ID_SOMME_EN_LIGNE,
    ID_TRANSACTION_LIST,
    ID_SEARCH_BOX
};

#endif // MAINFRAME_H