//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <wx/fileconf.h>

class Settings {
public:
    enum DateFormat {
        FORMAT_YYYY_MM_DD,  // 2025-10-27
        FORMAT_DD_MM_YYYY,  // 27/10/2025
        FORMAT_DD_MM_YY,    // 27/10/25
        FORMAT_MM_DD_YYYY,  // 10/27/2025
        FORMAT_DD_MMM_YYYY  // 27 Oct 2025
    };

    enum DecimalSeparator {
        SEPARATOR_COMMA,    // 1 234,56
        SEPARATOR_POINT     // 1,234.56
    };

    static Settings& GetInstance();

    // Getters
    DateFormat GetDateFormat() const { return mDateFormat; }
    DecimalSeparator GetDecimalSeparator() const { return mDecimalSeparator; }

    // Setters
    void SetDateFormat(DateFormat format);
    void SetDecimalSeparator(DecimalSeparator separator);

    // Formatage
    wxString FormatDate(const wxDateTime& date) const;
    wxString FormatMoney(double amount) const;

    // Sauvegarde et chargement
    void Save();
    void Load();

private:
    Settings();
    ~Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    DateFormat mDateFormat;
    DecimalSeparator mDecimalSeparator;
    wxFileConfig* mConfig;
};

#endif // SETTINGS_H