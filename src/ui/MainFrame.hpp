//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include "db/SqliteDB.hpp"
#include "version.h"

namespace mc::ui {

class MainFrame : public wxFrame {
public:
    explicit MainFrame(mc::db::SqliteDB& db);

private:
    mc::db::SqliteDB& db_;
    wxNotebook* notebook_;

    void create_menu_bar();
    void create_notebook();
    void create_status_bar();

    // Event handlers
    void on_about(wxCommandEvent& event);
    void on_quit(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

} // namespace mc::ui