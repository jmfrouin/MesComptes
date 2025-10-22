//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>
#include "core/DataStore.hpp"
#include "core/Txn.hpp"
#include <vector>

namespace mc::ui {

class ImportCsvDlg : public wxDialog {
public:
    ImportCsvDlg(wxWindow* parent, mc::core::DataStore& store, int64_t account_id);

    int get_imported_count() const { return imported_count_; }

private:
    mc::core::DataStore& store_;
    int64_t account_id_;
    int imported_count_ = 0;

    wxFilePickerCtrl* file_picker_;
    wxChoice* separator_choice_;
    wxChoice* date_format_choice_;
    wxListCtrl* preview_list_;
    wxButton* import_button_;

    std::vector<core::Txn> parsed_transactions_;

    void create_ui();
    void on_file_changed(wxFileDirPickerEvent& event);
    void on_separator_changed(wxCommandEvent& event);
    void on_date_format_changed(wxCommandEvent& event);
    void on_import(wxCommandEvent& event);
    void on_cancel(wxCommandEvent& event);

    bool parse_csv_file(const wxString& filepath);
    void update_preview();
    wxString get_selected_separator() const;
    wxString get_selected_date_format() const;
    
    // Parse une ligne CSV avec le séparateur donné
    std::vector<wxString> split_csv_line(const wxString& line, const wxString& separator);
    
    // Parse une date avec le format donné
    int64_t parse_date(const wxString& date_str, const wxString& format);
};

} // namespace mc::ui