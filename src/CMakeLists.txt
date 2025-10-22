set(CORE_HEADERS
        core/Account.hpp
        core/Txn.hpp
        core/Type.hpp
        core/Totals.hpp
        core/Settings.hpp
)

set(DB_SOURCES
        db/SqliteDB.cpp
        db/Migrations.cpp
        db/dao/AccountDAO.cpp
        db/dao/TxnDAO.cpp
        db/dao/TypeDAO.cpp
)

set(DB_HEADERS
        db/SqliteDB.hpp
        db/Migrations.hpp
        db/dao/AccountDAO.hpp
        db/dao/TxnDAO.hpp
        db/dao/TypeDAO.hpp
)

set(UI_SOURCES
        ui/MainFrame.cpp
        ui/HomePage.cpp
        ui/AccountsPage.cpp
        ui/PrefsPage.cpp
        ui/dialogs/TxnEditDlg.cpp
        ui/dialogs/ImportCsvDlg.cpp
)

set(UI_HEADERS
        ui/MainFrame.hpp
        ui/HomePage.hpp
        ui/AccountsPage.hpp
        ui/PrefsPage.hpp
        ui/dialogs/TxnEditDlg.hpp
        ui/dialogs/ImportCsvDlg.hpp
)

set(APP_SOURCES
        app/MesComptesApp.cpp
)

set(APP_HEADERS
        app/MesComptesApp.hpp
)

add_executable(MesComptes
        ${APP_SOURCES}
        ${APP_HEADERS}
        ${CORE_HEADERS}
        ${DB_SOURCES}
        ${DB_HEADERS}
        ${UI_SOURCES}
        ${UI_HEADERS}
)

target_link_libraries(MesComptes
        ${wxWidgets_LIBRARIES}
        SQLite::SQLite3
)

target_include_directories(MesComptes PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated
)

# Platform-specific settings
if(WIN32)
    set_target_properties(MesComptes PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(MesComptes PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist
    )
endif()

install(TARGETS MesComptes DESTINATION bin)