//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "MainFrame.hpp"
#include "HomePage.hpp"
#include "AccountsPage.hpp"
#include "PrefsPage.hpp"
#include <wx/aboutdlg.h>

namespace mc::ui {

enum {
    ID_Quit = wxID_EXIT,
    ID_About = wxID_ABOUT
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_Quit, MainFrame::on_quit)
    EVT_MENU(ID_About, MainFrame::on_about)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(mc::db::SqliteDB& db)
    : wxFrame(nullptr, wxID_ANY, "Mes Comptes", wxDefaultPosition, wxSize(1024, 768)),
      db_(db), notebook_(nullptr) {

    create_menu_bar();
    create_notebook();
    create_status_bar();

    Centre();
}

void MainFrame::create_menu_bar() {
    auto* menu_file = new wxMenu;
    menu_file->Append(ID_Quit, "&Quitter\tCtrl-Q", "Quitter l'application");

    auto* menu_help = new wxMenu;
    menu_help->Append(ID_About, "&À propos", "À propos de Mes Comptes");

    auto* menu_bar = new wxMenuBar;
    menu_bar->Append(menu_file, "&Fichier");
    menu_bar->Append(menu_help, "&Aide");

    SetMenuBar(menu_bar);
}

void MainFrame::create_notebook() {
    notebook_ = new wxNotebook(this, wxID_ANY);

    // Onglet Accueil
    auto* home_page = new HomePage(notebook_, db_);
    notebook_->AddPage(home_page, "Accueil", true);

    // Onglet Mes Comptes
    auto* accounts_page = new AccountsPage(notebook_, db_);
    notebook_->AddPage(accounts_page, "Mes Comptes", false);

    // Onglet Préférences
    auto* prefs_page = new PrefsPage(notebook_, db_);
    notebook_->AddPage(prefs_page, "Préférences", false);
}

void MainFrame::create_status_bar() {
    CreateStatusBar(1);
    SetStatusText("Bienvenue dans Mes Comptes");
}

void MainFrame::on_about(wxCommandEvent& WXUNUSED(event)) {
    wxAboutDialogInfo info;
    info.SetName("Mes Comptes");
    info.SetVersion(wxString::Format("%d.%d.%d",
                                     MC_VERSION_MAJOR,
                                     MC_VERSION_MINOR,
                                     MC_VERSION_PATCH));
    info.SetDescription("Application de gestion de comptes bancaires");
    info.SetCopyright("(C) 2025");
    info.SetWebSite("https://github.com/yourname/mes-comptes");

    info.AddDeveloper("Votre Nom");

    // Ajouter les crédits pour les bibliothèques
    wxString credits = wxString::Format(
        "Construit avec:\n"
        "- wxWidgets %s\n"
        "- SQLite %s",
        wxVERSION_STRING,
        sqlite3_libversion()
    );
    info.SetLicense(credits);

    wxAboutBox(info, this);
}

void MainFrame::on_quit(wxCommandEvent& WXUNUSED(event)) {
    Close(true);
}

} // namespace mc::ui