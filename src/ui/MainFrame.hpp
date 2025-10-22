//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include "../core/DataStore.hpp"
#include <version.h>

namespace mc::ui {

class MainFrame : public wxFrame {
public:
    explicit MainFrame(mc::core::DataStore& store);

private:
    mc::core::DataStore& store_;
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