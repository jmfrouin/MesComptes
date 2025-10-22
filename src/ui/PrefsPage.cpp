//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#include "PrefsPage.hpp"
#include "db/dao/TypeDAO.hpp"
#include <wx/sizer.h>
#include <wx/statbox.h>

namespace mc::ui {

enum {
    ID_AddType = wxID_HIGHEST + 100,
    ID_EditType,
    ID_DeleteType,
    ID_TypeList
};

PrefsPage::PrefsPage(wxWindow* parent, mc::db::SqliteDB& db)
    : wxPanel(parent), db_(db) {
    create_ui();
    refresh_types();
}

void PrefsPage::create_ui() {
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Titre
    auto* title = new wxStaticText(this, wxID_ANY, "Préférences");
    wxFont title_font = title->GetFont();
    title_font.SetPointSize(14);
    title_font.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(title_font);
    main_sizer->Add(title, 0, wxALL, 10);

    // Section Types
    auto* type_box = new wxStaticBoxSizer(wxVERTICAL, this, "Types de transactions");

    // Boutons
    auto* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* btn_add = new wxButton(this, ID_AddType, "Ajouter");
    auto* btn_edit = new wxButton(this, ID_EditType, "Modifier");
    auto* btn_delete = new wxButton(this, ID_DeleteType, "Supprimer");

    btn_sizer->Add(btn_add, 0, wxALL, 5);
    btn_sizer->Add(btn_edit, 0, wxALL, 5);
    btn_sizer->Add(btn_delete, 0, wxALL, 5);

    type_box->Add(btn_sizer, 0, wxALL, 5);

    // Liste des types
    type_list_ = new wxListCtrl(this, ID_TypeList, wxDefaultPosition, wxSize(-1, 300),
                                wxLC_REPORT | wxLC_SINGLE_SEL);
    type_list_->AppendColumn("ID", wxLIST_FORMAT_LEFT, 80);
    type_list_->AppendColumn("Nom", wxLIST_FORMAT_LEFT, 200);

    type_box->Add(type_list_, 1, wxALL | wxEXPAND, 5);

    main_sizer->Add(type_box, 1, wxALL | wxEXPAND, 10);

    SetSizer(main_sizer);

    // Bind events
    Bind(wxEVT_BUTTON, &PrefsPage::on_add_type, this, ID_AddType);
    Bind(wxEVT_BUTTON, &PrefsPage::on_edit_type, this, ID_EditType);
    Bind(wxEVT_BUTTON, &PrefsPage::on_delete_type, this, ID_DeleteType);
    Bind(wxEVT_LIST_ITEM_ACTIVATED, &PrefsPage::on_type_activated, this, ID_TypeList);
}

void PrefsPage::refresh_types() {
    type_list_->DeleteAllItems();

    db::dao::TypeDAO type_dao(db_);
    types_ = type_dao.find_all();

    for (size_t i = 0; i < types_.size(); ++i) {
        const auto& type = types_[i];
        long index = type_list_->InsertItem(i, wxString::Format("%lld", type.id));
        type_list_->SetItem(index, 1, type.name);
    }
}

void PrefsPage::on_add_type(wxCommandEvent& WXUNUSED(event)) {
    wxTextEntryDialog dlg(this, "Nom du nouveau type:", "Ajouter un type");

    if (dlg.ShowModal() == wxID_OK) {
        wxString name = dlg.GetValue().Trim();
        if (name.IsEmpty()) {
            wxMessageBox("Le nom ne peut pas être vide", "Erreur", wxOK | wxICON_ERROR);
            return;
        }

        core::Type type(name.ToStdString());
        db::dao::TypeDAO type_dao(db_);

        int64_t id = type_dao.insert(type);
        if (id > 0) {
            refresh_types();
        } else {
            wxMessageBox("Impossible d'ajouter le type (nom déjà existant ?)",
                        "Erreur", wxOK | wxICON_ERROR);
        }
    }
}

void PrefsPage::on_edit_type(wxCommandEvent& WXUNUSED(event)) {
    long selected = type_list_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) {
        wxMessageBox("Veuillez sélectionner un type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    if (selected >= 0 && selected < static_cast<long>(types_.size())) {
        const auto& type = types_[selected];

        wxTextEntryDialog dlg(this, "Nouveau nom:", "Modifier le type", type.name);

        if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue().Trim();
            if (name.IsEmpty()) {
                wxMessageBox("Le nom ne peut pas être vide", "Erreur", wxOK | wxICON_ERROR);
                return;
            }

            core::Type updated_type = type;
            updated_type.name = name.ToStdString();

            db::dao::TypeDAO type_dao(db_);
            if (type_dao.update(updated_type)) {
                refresh_types();
            } else {
                wxMessageBox("Impossible de modifier le type", "Erreur", wxOK | wxICON_ERROR);
            }
        }
    }
}

void PrefsPage::on_delete_type(wxCommandEvent& WXUNUSED(event)) {
    long selected = type_list_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) {
        wxMessageBox("Veuillez sélectionner un type", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    if (selected >= 0 && selected < static_cast<long>(types_.size())) {
        const auto& type = types_[selected];

        int answer = wxMessageBox(
            wxString::Format("Êtes-vous sûr de vouloir supprimer le type '%s' ?", type.name),
            "Confirmation", wxYES_NO | wxICON_QUESTION);

        if (answer == wxYES) {
            db::dao::TypeDAO type_dao(db_);

            if (type_dao.is_used(type.id)) {
                wxMessageBox("Ce type est utilisé par des transactions et ne peut pas être supprimé",
                            "Erreur", wxOK | wxICON_ERROR);
                return;
            }

            if (type_dao.remove(type.id)) {
                refresh_types();
            } else {
                wxMessageBox("Impossible de supprimer le type", "Erreur", wxOK | wxICON_ERROR);
            }
        }
    }
}

void PrefsPage::on_type_activated(wxListEvent& WXUNUSED(event)) {
    wxCommandEvent dummy;
    on_edit_type(dummy);
}

} // namespace mc::ui