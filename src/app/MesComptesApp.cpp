//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "MesComptesApp.hpp"
#include "ui/MainFrame.hpp"
#include "db/Migrations.hpp"
#include <wx/stdpaths.h>
#include <wx/filename.h>

wxIMPLEMENT_APP(mc::app::MesComptesApp);

namespace mc::app {

bool MesComptesApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }

    // Initialisation de la base de données
    if (!initialize_database()) {
        wxMessageBox("Impossible d'initialiser la base de données",
                     "Erreur", wxOK | wxICON_ERROR);
        return false;
    }

    // Création de la fenêtre principale
    auto* frame = new mc::ui::MainFrame(*db_);
    frame->Show(true);

    return true;
}

int MesComptesApp::OnExit() {
    db_.reset();
    return wxApp::OnExit();
}

bool MesComptesApp::initialize_database() {
    std::string db_path = get_database_path();

    db_ = std::make_unique<mc::db::SqliteDB>(db_path);

    if (!db_->open()) {
        return false;
    }

    // Application des migrations
    mc::db::Migrations migrations(*db_);
    if (!migrations.apply_all()) {
        return false;
    }

    return true;
}

std::string MesComptesApp::get_database_path() {
    wxStandardPaths& paths = wxStandardPaths::Get();
    wxString user_data_dir = paths.GetUserLocalDataDir();

    // Créer le répertoire s'il n'existe pas
    if (!wxFileName::DirExists(user_data_dir)) {
        wxFileName::Mkdir(user_data_dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    wxFileName db_file(user_data_dir, "mes_comptes.db");
    return db_file.GetFullPath().ToStdString();
}

} // namespace mc::app