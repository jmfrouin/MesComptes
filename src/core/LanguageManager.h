//
// Created by Jean-Michel Frouin on 05/11/2025.
//

#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <wx/wx.h>
#include <wx/intl.h>
#include <memory>

class LanguageManager {
public:
    static LanguageManager& GetInstance();

    void Initialize(wxWindow* parent = nullptr);
    bool SetLanguage(wxLanguage lang);
    wxLanguage GetCurrentLanguage() const;

    // Helper pour obtenir le nom de la langue
    wxString GetLanguageName(wxLanguage lang) const;

    // Liste des langues supportées
    std::vector<wxLanguage> GetSupportedLanguages() const;

private:
    LanguageManager();
    ~LanguageManager();

    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;

    std::unique_ptr<wxLocale> mLocale;
    wxLanguage mCurrentLanguage;
};

#endif // LANGUAGEMANAGER_H