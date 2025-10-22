//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <memory>
#include "db/SqliteDB.hpp"

namespace mc::app {

class MesComptesApp : public wxApp {
public:
    bool OnInit() override;
    int OnExit() override;

    mc::db::SqliteDB& get_database() { return *db_; }

private:
    std::unique_ptr<mc::db::SqliteDB> db_;

    bool initialize_database();
    std::string get_database_path();
};

} // namespace mc::app

wxDECLARE_APP(mc::app::MesComptesApp);