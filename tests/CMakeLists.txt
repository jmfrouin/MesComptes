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
        totals_tests.cpp
)

target_link_libraries(mes_comptes_tests
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(mes_comptes_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/generated
)