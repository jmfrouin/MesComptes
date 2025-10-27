
# Read current version
if(EXISTS "${VERSION_FILE}")
    file(READ "${VERSION_FILE}" CURRENT_VERSION)
    string(STRIP "${CURRENT_VERSION}" CURRENT_VERSION)
else()
    set(CURRENT_VERSION "1.0.0")
endif()

# Parse version components
string(REPLACE "." ";" VERSION_LIST ${CURRENT_VERSION})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)

# Increment patch version
math(EXPR VERSION_PATCH "${VERSION_PATCH} + 1")

# Create new version string
set(NEW_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Write new version back to file
file(WRITE "${VERSION_FILE}" "${NEW_VERSION}")

# Generate version header file
configure_file("${TEMPLATE_FILE}" "${OUTPUT_FILE}")

message(STATUS "Version updated to: ${NEW_VERSION}")