//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/panel.h>
#include "db/SqliteDB.hpp"

namespace mc::ui {

class HomePage : public wxPanel {
public:
    HomePage(wxWindow* parent, mc::db::SqliteDB& db);

private:
    mc::db::SqliteDB& db_;

    wxStaticText* account_count_text_;
    wxStaticText* total_balance_text_;
    wxStaticText* last_update_text_;

    void create_ui();
    void refresh_stats();

    wxString format_currency(int64_t cents);
};

} // namespace mc::ui