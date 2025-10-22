//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>
#include "db/SqliteDB.hpp"
#include "core/Type.hpp"
#include <vector>

namespace mc::ui {

class PrefsPage : public wxPanel {
public:
    PrefsPage(wxWindow* parent, mc::db::SqliteDB& db);

private:
    mc::db::SqliteDB& db_;
    wxListCtrl* type_list_;
    std::vector<core::Type> types_;

    void create_ui();
    void refresh_types();

    // Event handlers
    void on_add_type(wxCommandEvent& event);
    void on_edit_type(wxCommandEvent& event);
    void on_delete_type(wxCommandEvent& event);
    void on_type_activated(wxListEvent& event);
};

} // namespace mc::ui