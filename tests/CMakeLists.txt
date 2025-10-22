# Download and configure Google Test if not found
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(mes_comptes_tests
        dao_tests.cpp
        totals_tests.cpp
)

target_link_libraries(mes_comptes_tests
        GTest::gtest
        GTest::gtest_main
        SQLite::SQLite3
)

target_include_directories(mes_comptes_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/generated
)

# Ajouter les sources nécessaires
target_sources(mes_comptes_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/db/SqliteDB.cpp
        ${CMAKE_SOURCE_DIR}/src/db/Migrations.cpp
        ${CMAKE_SOURCE_DIR}/src/db/dao/AccountDAO.cpp
        ${CMAKE_SOURCE_DIR}/src/db/dao/TxnDAO.cpp
        ${CMAKE_SOURCE_DIR}/src/db/dao/TypeDAO.cpp
)

include(GoogleTest)
gtest_discover_tests(mes_comptes_tests)