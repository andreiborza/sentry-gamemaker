cmake_minimum_required(VERSION 3.16)
project(SentryGameMaker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set paths to sentry-native
set(SENTRY_NATIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/sentry-native")
set(SENTRY_INCLUDE_DIR "${SENTRY_NATIVE_DIR}/include")
set(SENTRY_LIB_DIR "${SENTRY_NATIVE_DIR}/lib")


# Find the sentry library
find_library(SENTRY_LIB
    NAMES sentry libsentry
    PATHS ${SENTRY_LIB_DIR}
    NO_DEFAULT_PATH
)

if(NOT SENTRY_LIB)
    message(FATAL_ERROR "Sentry library not found in ${SENTRY_LIB_DIR}")
endif()

# Create GameMaker extension library (main target)
add_library(sentry_gm SHARED src/sentry_gm.cpp)

# Set include directories for extension
target_include_directories(sentry_gm PRIVATE ${SENTRY_INCLUDE_DIR})


# Link libraries for extension
target_link_libraries(sentry_gm ${SENTRY_LIB})

# For macOS, we need to set the rpath to find the dylib
if(APPLE)
    set_target_properties(sentry_gm PROPERTIES
        INSTALL_RPATH "${SENTRY_LIB_DIR}"
        BUILD_WITH_INSTALL_RPATH TRUE
        PREFIX "lib"
        SUFFIX ".dylib"
    )
endif()

# Copy extension files from src to demo project
add_custom_command(TARGET sentry_gm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying Sentry extension to demo project..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/extensions/Sentry"
        "${CMAKE_CURRENT_SOURCE_DIR}/demo/SentryGameMaker/extensions/Sentry"
    COMMENT "Copying extension files to demo project"
)

# Copy built dylib and crashpad_handler to demo extension directory
add_custom_command(TARGET sentry_gm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying libsentry_gm.dylib to SentryGameMaker..."
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:sentry_gm>
        "${CMAKE_CURRENT_SOURCE_DIR}/demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib"
    COMMENT "Copying dylib to SentryGameMaker extension directory"
)

add_custom_command(TARGET sentry_gm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying crashpad_handler to SentryGameMaker..."
    COMMAND ${CMAKE_COMMAND} -E copy
        "${SENTRY_NATIVE_DIR}/bin/crashpad_handler"
        "${CMAKE_CURRENT_SOURCE_DIR}/demo/SentryGameMaker/extensions/Sentry/crashpad_handler"
    COMMENT "Copying crashpad_handler to SentryGameMaker extension directory"
)


# Print some info
message(STATUS "Sentry include directory: ${SENTRY_INCLUDE_DIR}")
message(STATUS "Sentry library: ${SENTRY_LIB}")
message(STATUS "Crashpad handler: ${SENTRY_NATIVE_DIR}/bin/crashpad_handler")