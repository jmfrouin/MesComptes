//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "MainFrame.hpp"
#include "HomePage.hpp"
#include "AccountsPage.hpp"
#include "PrefsPage.hpp"
#include <wx/aboutdlg.h>
#include <version.h>

namespace mc::ui {

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_ABOUT, MainFrame::on_about)
    EVT_MENU(wxID_EXIT, MainFrame::on_quit)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(mc::core::DataStore& store)
    : wxFrame(nullptr, wxID_ANY, "Mes Comptes", wxDefaultPosition, wxSize(800, 600)),
      store_(store), notebook_(nullptr) {
    create_menu_bar();
    create_notebook();
    create_status_bar();

    Centre();
}

void MainFrame::create_menu_bar() {
    auto* menu_bar = new wxMenuBar();

    // Menu Fichier
    auto* file_menu = new wxMenu();
    file_menu->Append(wxID_EXIT, "&Quitter\tCtrl-Q", "Quitter l'application");
    menu_bar->Append(file_menu, "&Fichier");

    // Menu Aide
    auto* help_menu = new wxMenu();
    help_menu->Append(wxID_ABOUT, "&À propos", "À propos de Mes Comptes");
    menu_bar->Append(help_menu, "&Aide");

    SetMenuBar(menu_bar);
}

void MainFrame::create_notebook() {
    notebook_ = new wxNotebook(this, wxID_ANY);

    // Page d'accueil
    auto* home_page = new HomePage(notebook_, store_);
    notebook_->AddPage(home_page, "Accueil", true);

    // Page des comptes
    auto* accounts_page = new AccountsPage(notebook_, store_);
    notebook_->AddPage(accounts_page, "Mes Comptes", false);

    // Page des préférences
    auto* prefs_page = new PrefsPage(notebook_, store_);
    notebook_->AddPage(prefs_page, "Préférences", false);
}

void MainFrame::create_status_bar() {
    CreateStatusBar(2);
    SetStatusText("Prêt", 0);
    SetStatusText(wxString::Format("Version %d.%d.%d", 
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH), 1);
}

void MainFrame::on_about(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName("Mes Comptes");
    info.SetVersion(wxString::Format("%d.%d.%d (build %d)",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, BUILD_NUMBER));
    info.SetDescription("Application de gestion de comptes personnels");
    info.SetCopyright("(C) 2025");
    info.AddDeveloper("Jean-Michel Frouin");

    wxAboutBox(info, this);
}

void MainFrame::on_quit(wxCommandEvent& event) {
    Close(true);
}

} // namespace mc::ui