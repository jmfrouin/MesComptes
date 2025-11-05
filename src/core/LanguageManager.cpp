//
// Created by Jean-Michel Frouin on 05/11/2025.
//

#include "LanguageManager.h"
#include <wx/stdpaths.h>

LanguageManager::LanguageManager()
    : mCurrentLanguage(wxLANGUAGE_DEFAULT) {
}

LanguageManager::~LanguageManager() {
}

LanguageManager& LanguageManager::GetInstance() {
    static LanguageManager instance;
    return instance;
}

void LanguageManager::Initialize(wxWindow* parent) {
    // Charger la langue depuis les préférences (par défaut: langue du système)
    wxLanguage lang = wxLANGUAGE_DEFAULT;

    // TODO: Charger depuis les préférences utilisateur
    // Pour l'instant, utiliser la langue du système

    SetLanguage(lang);
}

bool LanguageManager::SetLanguage(wxLanguage lang) {
    // Si c'est déjà la langue courante, ne rien faire
    if (mLocale && lang == mCurrentLanguage) {
        return true;
    }

    // Détruire l'ancien locale
    mLocale.reset();

    // Si DEFAULT, utiliser la langue du système
    if (lang == wxLANGUAGE_DEFAULT) {
        lang = static_cast<wxLanguage>(wxLocale::GetSystemLanguage());
    }

    // Vérifier que la langue est supportée
    const wxLanguageInfo* langInfo = wxLocale::GetLanguageInfo(lang);
    if (!langInfo) {
        return false;
    }

    // Créer le nouveau locale
    mLocale = std::make_unique<wxLocale>();

    if (!mLocale->Init(lang, wxLOCALE_LOAD_DEFAULT)) {
        mLocale.reset();
        return false;
    }

    // Ajouter le chemin vers les catalogues de traduction
    wxStandardPaths& stdPaths = wxStandardPaths::Get();
    wxString langPath = stdPaths.GetResourcesDir() + "/lang";

    // Essayer aussi le chemin relatif pour le développement
    if (!wxDirExists(langPath)) {
        langPath = "./lang";
    }

    mLocale->AddCatalogLookupPathPrefix(langPath);

    // Charger le catalogue de traduction
    mLocale->AddCatalog("mescomptes");

    mCurrentLanguage = lang;

    return true;
}

wxLanguage LanguageManager::GetCurrentLanguage() const {
    return mCurrentLanguage;
}

wxString LanguageManager::GetLanguageName(wxLanguage lang) const {
    const wxLanguageInfo* langInfo = wxLocale::GetLanguageInfo(lang);
    if (langInfo) {
        return langInfo->Description;
    }
    return wxEmptyString;
}

std::vector<wxLanguage> LanguageManager::GetSupportedLanguages() const {
    return {
        wxLANGUAGE_FRENCH,
        wxLANGUAGE_ENGLISH
    };
}