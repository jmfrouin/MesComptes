//
// Created by Jean-Michel Frouin on 22/10/2025.
//

#pragma once
#include <wx/wx.h>
#include "core/DataStore.hpp"
#include <memory>

namespace mc {

class MesComptesApp : public wxApp {
public:
    virtual bool OnInit() override;
    mc::core::DataStore& get_datastore() { return *store_; }

private:
    std::unique_ptr<mc::core::DataStore> store_;
};

} // namespace mc

wxDECLARE_APP(mc::MesComptesApp);