//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "ImportCsvDlg.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <sstream>

namespace mc::ui {

ImportCsvDlg::ImportCsvDlg(wxWindow* parent, mc::core::DataStore& store, int64_t account_id)
    : wxDialog(parent, wxID_ANY, "Importer des transactions depuis CSV",
               wxDefaultPosition, wxSize(700, 500)),
      store_(store), account_id_(account_id) {
    create_ui();
}

void ImportCsvDlg::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Sélection du fichier
    auto* file_sizer = new wxBoxSizer(wxHORIZONTAL);
    file_sizer->Add(new wxStaticText(this, wxID_ANY, "Fichier CSV:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    file_picker_ = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, "Choisir un fichier CSV",
                                        "Fichiers CSV (*.csv)|*.csv|Tous les fichiers (*.*)|*.*");
    file_sizer->Add(file_picker_, 1, wxEXPAND);
    main_sizer->Add(file_sizer, 0, wxALL | wxEXPAND, 5);

    // Options d'import
    auto* options_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Séparateur
    options_sizer->Add(new wxStaticText(this, wxID_ANY, "Séparateur:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    separator_choice_ = new wxChoice(this, wxID_ANY);
    separator_choice_->Append("Point-virgule (;)");
    separator_choice_->Append("Virgule (,)");
    separator_choice_->Append("Tabulation");
    separator_choice_->SetSelection(0);
    options_sizer->Add(separator_choice_, 0, wxRIGHT, 15);

    // Format de date
    options_sizer->Add(new wxStaticText(this, wxID_ANY, "Format de date:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    date_format_choice_ = new wxChoice(this, wxID_ANY);
    date_format_choice_->Append("DD/MM/YYYY");
    date_format_choice_->Append("MM/DD/YYYY");
    date_format_choice_->Append("YYYY-MM-DD");
    date_format_choice_->SetSelection(0);
    options_sizer->Add(date_format_choice_, 0);

    main_sizer->Add(options_sizer, 0, wxALL, 5);

    // Aperçu
    auto* preview_label = new wxStaticText(this, wxID_ANY, "Aperçu des transactions:");
    main_sizer->Add(preview_label, 0, wxALL, 5);

    preview_list_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    preview_list_->AppendColumn("Date", wxLIST_FORMAT_LEFT, 100);
    preview_list_->AppendColumn("Libellé", wxLIST_FORMAT_LEFT, 300);
    preview_list_->AppendColumn("Montant", wxLIST_FORMAT_RIGHT, 100);
    main_sizer->Add(preview_list_, 1, wxALL | wxEXPAND, 5);

    // Boutons
    auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    import_button_ = new wxButton(this, wxID_OK, "Importer");
    import_button_->Enable(false);
    button_sizer->Add(import_button_, 0, wxRIGHT, 5);
    button_sizer->Add(new wxButton(this, wxID_CANCEL, "Annuler"), 0);
    main_sizer->Add(button_sizer, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(main_sizer);

    // Bind events
    Bind(wxEVT_FILEPICKER_CHANGED, &ImportCsvDlg::on_file_changed, this);
    Bind(wxEVT_CHOICE, &ImportCsvDlg::on_separator_changed, this, separator_choice_->GetId());
    Bind(wxEVT_CHOICE, &ImportCsvDlg::on_date_format_changed, this, date_format_choice_->GetId());
    Bind(wxEVT_BUTTON, &ImportCsvDlg::on_import, this, wxID_OK);
    Bind(wxEVT_BUTTON, &ImportCsvDlg::on_cancel, this, wxID_CANCEL);
}

void ImportCsvDlg::on_file_changed(wxFileDirPickerEvent& event) {
    wxString filepath = file_picker_->GetPath();
    if (!filepath.IsEmpty() && wxFileExists(filepath)) {
        if (parse_csv_file(filepath)) {
            update_preview();
            import_button_->Enable(parsed_transactions_.size() > 0);
        }
    }
}

void ImportCsvDlg::on_separator_changed(wxCommandEvent& event) {
    wxString filepath = file_picker_->GetPath();
    if (!filepath.IsEmpty() && wxFileExists(filepath)) {
        parse_csv_file(filepath);
        update_preview();
    }
}

void ImportCsvDlg::on_date_format_changed(wxCommandEvent& event) {
    wxString filepath = file_picker_->GetPath();
    if (!filepath.IsEmpty() && wxFileExists(filepath)) {
        parse_csv_file(filepath);
        update_preview();
    }
}

void ImportCsvDlg::on_import(wxCommandEvent& event) {
    imported_count_ = 0;
    
    for (const auto& txn : parsed_transactions_) {
        int64_t id = store_.add_transaction(txn);
        if (id > 0) {
            imported_count_++;
        }
    }

    if (imported_count_ > 0) {
        wxMessageBox(wxString::Format("%d transaction(s) importée(s) avec succès", imported_count_),
                     "Import réussi", wxOK | wxICON_INFORMATION, this);
        EndModal(wxID_OK);
    } else {
        wxMessageBox("Aucune transaction n'a pu être importée", "Erreur", wxOK | wxICON_ERROR, this);
    }
}

void ImportCsvDlg::on_cancel(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

bool ImportCsvDlg::parse_csv_file(const wxString& filepath) {
    parsed_transactions_.clear();

    wxTextFile file(filepath);
    if (!file.Open()) {
        wxMessageBox("Impossible d'ouvrir le fichier", "Erreur", wxOK | wxICON_ERROR, this);
        return false;
    }

    wxString separator = get_selected_separator();
    wxString date_format = get_selected_date_format();

    // Sauter la première ligne (en-têtes)
    bool first_line = true;
    
    for (size_t i = 0; i < file.GetLineCount(); i++) {
        wxString line = file[i].Trim();
        if (line.IsEmpty()) continue;
        
        if (first_line) {
            first_line = false;
            continue; // Sauter les en-têtes
        }

        std::vector<wxString> fields = split_csv_line(line, separator);
        
        // Format attendu: Date, Libellé, Montant
        if (fields.size() >= 3) {
            core::Txn txn;
            txn.account_id = account_id_;
            txn.op_date = parse_date(fields[0], date_format);
            txn.label = fields[1].ToStdString();
            
            // Parser le montant
            wxString amount_str = fields[2];
            amount_str.Replace(",", ".");
            double amount;
            if (amount_str.ToDouble(&amount)) {
                txn.amount_cents = static_cast<int64_t>(amount * 100);
            }
            
            parsed_transactions_.push_back(txn);
        }
    }

    file.Close();
    return parsed_transactions_.size() > 0;
}

void ImportCsvDlg::update_preview() {
    preview_list_->DeleteAllItems();

    for (size_t i = 0; i < parsed_transactions_.size() && i < 100; i++) {
        const auto& txn = parsed_transactions_[i];
        
        // Date
        wxDateTime dt;
        dt.Set(static_cast<time_t>(txn.op_date));
        wxString date_str = dt.FormatDate();
        
        long index = preview_list_->InsertItem(i, date_str);
        preview_list_->SetItem(index, 1, txn.label);
        preview_list_->SetItem(index, 2, wxString::Format("%.2f €", txn.amount_cents / 100.0));
    }

    if (parsed_transactions_.size() > 100) {
        wxLogMessage("Aperçu limité aux 100 premières transactions");
    }
}

wxString ImportCsvDlg::get_selected_separator() const {
    switch (separator_choice_->GetSelection()) {
        case 0: return ";";
        case 1: return ",";
        case 2: return "\t";
        default: return ";";
    }
}

wxString ImportCsvDlg::get_selected_date_format() const {
    switch (date_format_choice_->GetSelection()) {
        case 0: return "DD/MM/YYYY";
        case 1: return "MM/DD/YYYY";
        case 2: return "YYYY-MM-DD";
        default: return "DD/MM/YYYY";
    }
}

std::vector<wxString> ImportCsvDlg::split_csv_line(const wxString& line, const wxString& separator) {
    std::vector<wxString> fields;
    wxStringTokenizer tokenizer(line, separator);
    
    while (tokenizer.HasMoreTokens()) {
        wxString token = tokenizer.GetNextToken().Trim(true).Trim(false);
        // Enlever les guillemets si présents
        if (token.StartsWith("\"") && token.EndsWith("\"")) {
            token = token.Mid(1, token.Length() - 2);
        }
        fields.push_back(token);
    }
    
    return fields;
}

int64_t ImportCsvDlg::parse_date(const wxString& date_str, const wxString& format) {
    wxDateTime dt;
    
    if (format == "DD/MM/YYYY") {
        dt.ParseFormat(date_str, "%d/%m/%Y");
    } else if (format == "MM/DD/YYYY") {
        dt.ParseFormat(date_str, "%m/%d/%Y");
    } else if (format == "YYYY-MM-DD") {
        dt.ParseFormat(date_str, "%Y-%m-%d");
    }
    
    if (dt.IsValid()) {
        return dt.GetTicks();
    }
    
    return 0;
}

} // namespace mc::ui