//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/panel.h>
#include "../core/DataStore.hpp"

namespace mc::ui {

class HomePage : public wxPanel {
public:
    HomePage(wxWindow* parent, mc::core::DataStore& store);

    void refresh_stats();

private:
    mc::core::DataStore& store_;

    wxStaticText* account_count_text_;
    wxStaticText* total_balance_text_;
    wxStaticText* last_update_text_;

    void create_ui();

    wxString format_currency(double amount);
};

} // namespace mc::ui