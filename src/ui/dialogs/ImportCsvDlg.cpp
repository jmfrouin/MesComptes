//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "ImportCsvDlg.hpp"
#include "db/dao/TxnDAO.hpp"
#include <wx/sizer.h>
#include <fstream>
#include <sstream>

namespace mc::ui {

ImportCsvDlg::ImportCsvDlg(wxWindow* parent, mc::db::SqliteDB& db, int64_t account_id)
    : wxDialog(parent, wxID_ANY, "Importer des transactions",
               wxDefaultPosition, wxSize(800, 600)),
      db_(db), account_id_(account_id) {
    create_ui();
}

void ImportCsvDlg::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Sélection de fichier
    auto* file_sizer = new wxBoxSizer(wxHORIZONTAL);
    file_sizer->Add(new wxStaticText(this, wxID_ANY, "Fichier CSV:"),
                    0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    file_picker_ = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
                                        "Sélectionner un fichier CSV",
                                        "*.csv", wxDefaultPosition, wxDefaultSize,
                                        wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL);
    file_sizer->Add(file_picker_, 1, wxALL | wxEXPAND, 5);
    main_sizer->Add(file_sizer, 0, wxALL | wxEXPAND, 5);

    // Aperçu
    main_sizer->Add(new wxStaticText(this, wxID_ANY, "Aperçu:"),
                    0, wxALL, 5);
    preview_grid_ = new wxGrid(this, wxID_ANY);
    preview_grid_->CreateGrid(10, 5);
    preview_grid_->SetColLabelValue(0, "Date");
    preview_grid_->SetColLabelValue(1, "Libellé");
    preview_grid_->SetColLabelValue(2, "Montant");
    preview_grid_->SetColLabelValue(3, "Type");
    preview_grid_->SetColLabelValue(4, "Pointée");
    preview_grid_->EnableEditing(false);
    main_sizer->Add(preview_grid_, 1, wxALL | wxEXPAND, 5);

    // Boutons
    auto* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    import_btn_ = new wxButton(this, wxID_ANY, "Importer");
    import_btn_->Enable(false);
    auto* cancel_btn = new wxButton(this, wxID_CANCEL, "Annuler");
    btn_sizer->Add(import_btn_, 0, wxALL, 5);
    btn_sizer->Add(cancel_btn, 0, wxALL, 5);
    main_sizer->Add(btn_sizer, 0, wxALL | wxALIGN_RIGHT, 10);

    SetSizer(main_sizer);

    // Events
    file_picker_->Bind(wxEVT_FILEPICKER_CHANGED, &ImportCsvDlg::on_file_changed, this);
    import_btn_->Bind(wxEVT_BUTTON, &ImportCsvDlg::on_import, this);
}

void ImportCsvDlg::on_file_changed(wxFileDirPickerEvent& event) {
    wxString filepath = event.GetPath();
    if (!filepath.IsEmpty() && wxFileExists(filepath)) {
        if (load_preview(filepath)) {
            import_btn_->Enable(true);
        }
    } else {
        import_btn_->Enable(false);
    }
}

bool ImportCsvDlg::load_preview(const wxString& filepath) {
    std::ifstream file(filepath.ToStdString());
    if (!file.is_open()) {
        wxMessageBox("Impossible d'ouvrir le fichier", "Erreur", wxOK | wxICON_ERROR);
        return false;
    }

    preview_grid_->ClearGrid();

    std::string line;
    int row = 0;

    // Ignorer l'en-tête
    std::getline(file, line);

    while (std::getline(file, line) && row < 10) {
        std::istringstream iss(line);
        std::string token;
        int col = 0;

        while (std::getline(iss, token, ';') && col < 5) {
            preview_grid_->SetCellValue(row, col, token);
            col++;
        }
        row++;
    }

    file.close();
    return true;
}

void ImportCsvDlg::on_import(wxCommandEvent& WXUNUSED(event)) {
    wxString filepath = file_picker_->GetPath();

    if (perform_import(filepath)) {
        wxMessageBox("Import réussi", "Succès", wxOK | wxICON_INFORMATION);
        EndModal(wxID_OK);
    } else {
        wxMessageBox("Erreur lors de l'import", "Erreur", wxOK | wxICON_ERROR);
    }
}

bool ImportCsvDlg::perform_import(const wxString& filepath) {
    std::ifstream file(filepath.ToStdString());
    if (!file.is_open()) {
        return false;
    }

    db_.begin_transaction();

    db::dao::TxnDAO txn_dao(db_);
    std::string line;

    // Ignorer l'en-tête
    std::getline(file, line);

    int imported = 0;
    while (std::getline(file, line)) {
        // TODO: Parser la ligne CSV et créer une transaction
        // Format supposé: date;libellé;montant;type;pointée

        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (std::getline(iss, token, ';')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 3) {
            core::Txn txn;
            txn.account_id = account_id_;
            // TODO: Parser la date depuis tokens[0]
            txn.op_date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            txn.label = tokens[1];
            // TODO: Parser le montant depuis tokens[2]
            txn.amount_cents = static_cast<int64_t>(std::stod(tokens[2]) * 100);
            txn.created_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (txn_dao.insert(txn) > 0) {
                imported++;
            }
        }
    }

    file.close();

    if (imported > 0) {
        db_.commit();
        return true;
    } else {
        db_.rollback();
        return false;
    }
}

} // namespace mc::ui