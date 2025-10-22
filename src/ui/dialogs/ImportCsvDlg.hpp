//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/grid.h>
#include "db/SqliteDB.hpp"

namespace mc::ui {

class ImportCsvDlg : public wxDialog {
public:
    ImportCsvDlg(wxWindow* parent, mc::db::SqliteDB& db, int64_t account_id);

private:
    mc::db::SqliteDB& db_;
    int64_t account_id_;

    wxFilePickerCtrl* file_picker_;
    wxGrid* preview_grid_;
    wxButton* import_btn_;

    void create_ui();
    void on_file_changed(wxFileDirPickerEvent& event);
    void on_import(wxCommandEvent& event);

    bool load_preview(const wxString& filepath);
    bool perform_import(const wxString& filepath);
};

} // namespace mc::ui