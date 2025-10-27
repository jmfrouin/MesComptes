//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <wx/wx.h>
#include <core/Database.h>

class InfoDialog : public wxDialog {
public:
    InfoDialog(wxWindow* parent, Database* database);

private:
    Database* mDatabase;
};

#endif // INFODIALOG_H