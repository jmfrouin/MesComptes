cmake_minimum_required(VERSION 3.15)
project(MesComptes VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_FILE "${CMAKE_SOURCE_DIR}/version.txt")
set(VERSION_TEMPLATE "${CMAKE_SOURCE_DIR}/core/version.h.in")
set(VERSION_OUTPUT "${CMAKE_SOURCE_DIR}/core/version.h")

add_custom_target(update_version ALL
        COMMAND ${CMAKE_COMMAND}
        -DVERSION_FILE=${VERSION_FILE}
        -DTEMPLATE_FILE=${VERSION_TEMPLATE}
        -DOUTPUT_FILE=${VERSION_OUTPUT}
        -P ${CMAKE_SOURCE_DIR}/gen_version.cmake
        COMMENT "Updating version number"
        VERBATIM
)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

find_package(SQLite3 REQUIRED)

set(SOURCES
        main.cpp
        ui/MainFrame.cpp
        ui/PreferencesDialog.cpp
        ui/InfoDialog.cpp
        ui/CSVImportDialog.cpp
        ui/RecurringDialog.cpp
        core/Database.cpp
        core/Settings.cpp
        core/Transaction.cpp
        core/RecurringTransaction.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_dependencies(${PROJECT_NAME} update_version)

target_link_libraries(${PROJECT_NAME}
        ${wxWidgets_LIBRARIES}
        SQLite::SQLite3
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Copier les ressources
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})