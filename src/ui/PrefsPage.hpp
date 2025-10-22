//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/listctrl.h>
#include "../core/DataStore.hpp"
#include "core/Type.hpp"
#include <vector>

namespace mc::ui {

class PrefsPage : public wxPanel {
public:
    PrefsPage(wxWindow* parent, mc::core::DataStore& store);

    void refresh_types();

private:
    mc::core::DataStore& store_;
    wxListCtrl* type_list_;
    std::vector<core::Type> types_;
    bool is_initialized_ = false;

    void create_ui();
    void initialize_data();

    // Event handlers
    void on_add_type(wxCommandEvent& event);
    void on_edit_type(wxCommandEvent& event);
    void on_delete_type(wxCommandEvent& event);
    void on_type_activated(wxListEvent& event);
    void on_show(wxShowEvent& event);
};

} // namespace mc::ui