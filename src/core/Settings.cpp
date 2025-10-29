//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#include "Settings.h"
#include <wx/stdpaths.h>

Settings::Settings()
    : mDateFormat(FORMAT_DD_MM_YY),
      mDecimalSeparator(SEPARATOR_COMMA) {

    wxString configPath = wxStandardPaths::Get().GetUserDataDir();
    if (!wxFileName::DirExists(configPath)) {
        wxFileName::Mkdir(configPath, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    mConfig = new wxFileConfig("MesComptes", wxEmptyString,
                               configPath + "/mescomptes.ini");
    Load();
}

Settings::~Settings() {
    Save();
    delete mConfig;
}

Settings& Settings::GetInstance() {
    static Settings instance;
    return instance;
}

void Settings::SetDateFormat(DateFormat format) {
    mDateFormat = format;
    Save();
}

void Settings::SetDecimalSeparator(DecimalSeparator separator) {
    mDecimalSeparator = separator;
    Save();
}

wxString Settings::FormatDate(const wxDateTime& date) const {
    if (!date.IsValid()) {
        return "";
    }

    switch (mDateFormat) {
        case FORMAT_YYYY_MM_DD:
            return date.Format("%Y-%m-%d");
        case FORMAT_DD_MM_YYYY:
            return date.Format("%d/%m/%Y");
        case FORMAT_DD_MM_YY:
            return date.Format("%d/%m/%y");
        case FORMAT_MM_DD_YYYY:
            return date.Format("%m/%d/%Y");
        case FORMAT_DD_MMM_YYYY:
            return date.Format("%d %b %Y");
        default:
            return date.Format("%Y-%m-%d");
    }
}

wxString Settings::FormatMoney(double amount) const {
    wxString formatted;

    if (mDecimalSeparator == SEPARATOR_COMMA) {
        // Format français : 1 234,56
        formatted = wxString::Format("%.2f", amount);
        formatted.Replace(".", ",");

        // Ajouter les espaces pour les milliers
        int dotPos = formatted.Find(',');
        if (dotPos == wxNOT_FOUND) {
            dotPos = formatted.Length();
        }

        int pos = dotPos - 3;
        while (pos > 0) {
            if (formatted[pos - 1] != '-') {
                formatted.insert(pos, ' ');
            }
            pos -= 3;
        }
    } else {
        // Format anglais : 1,234.56
        formatted = wxString::Format("%.2f", amount);

        int dotPos = formatted.Find('.');
        if (dotPos == wxNOT_FOUND) {
            dotPos = formatted.Length();
        }

        int pos = dotPos - 3;
        while (pos > 0) {
            if (formatted[pos - 1] != '-') {
                formatted.insert(pos, ',');
            }
            pos -= 3;
        }
    }

    return formatted;
}

void Settings::Save() {
    mConfig->Write("/Display/DateFormat", static_cast<int>(mDateFormat));
    mConfig->Write("/Display/DecimalSeparator", static_cast<int>(mDecimalSeparator));
    mConfig->Flush();
}

void Settings::Load() {
    long dateFormat = mConfig->Read("/Display/DateFormat", static_cast<long>(FORMAT_DD_MM_YY));
    long decimalSep = mConfig->Read("/Display/DecimalSeparator", static_cast<long>(SEPARATOR_COMMA));

    mDateFormat = static_cast<DateFormat>(dateFormat);
    mDecimalSeparator = static_cast<DecimalSeparator>(decimalSep);
}