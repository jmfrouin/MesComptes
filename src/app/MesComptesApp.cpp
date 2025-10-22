//
//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "MesComptesApp.hpp"
#include "../ui/MainFrame.hpp"
#include <wx/stdpaths.h>
#include <wx/filename.h>

wxIMPLEMENT_APP(mc::MesComptesApp);

namespace mc {

bool MesComptesApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }

    // Déterminer le chemin du fichier de données
    wxStandardPaths& paths = wxStandardPaths::Get();
    wxString dataDir = paths.GetUserDataDir();
    
    // Créer le répertoire s'il n'existe pas
    wxFileName::Mkdir(dataDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    
    wxString dataFile = dataDir + wxFileName::GetPathSeparator() + "mescomptes.json";
    
    // Initialiser le DataStore
    store_ = std::make_unique<mc::core::DataStore>(dataFile.ToStdString());
    
    if (!store_->load()) {
        wxLogWarning("Impossible de charger les données, un nouveau fichier sera créé");
    }

    // Créer et afficher la fenêtre principale
    auto* frame = new mc::ui::MainFrame(*store_);
    frame->Show(true);

    return true;
}

} // namespace mc