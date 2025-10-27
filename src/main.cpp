#include <wx/wx.h>
#include <ui/MainFrame.h>

class MesComptesApp : public wxApp {
public:
    virtual bool OnInit() override {
        MainFrame* frame = new MainFrame("Mes Comptes");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MesComptesApp);