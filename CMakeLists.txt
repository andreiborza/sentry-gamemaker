cmake_minimum_required(VERSION 3.16)
project(SentrySample)

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

# Create executable
add_executable(sample src/main.cpp)

# Set include directories
target_include_directories(sample PRIVATE ${SENTRY_INCLUDE_DIR})

# Link libraries
target_link_libraries(sample ${SENTRY_LIB})

# For macOS, we need to set the rpath to find the dylib
if(APPLE)
    set_target_properties(sample PROPERTIES
        INSTALL_RPATH "${SENTRY_LIB_DIR}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# Copy crashpad_handler binary to output directory
add_custom_command(TARGET sample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${SENTRY_NATIVE_DIR}/bin/crashpad_handler"
        $<TARGET_FILE_DIR:sample>
    COMMENT "Copying crashpad_handler to output directory"
)

# Print some info
message(STATUS "Sentry include directory: ${SENTRY_INCLUDE_DIR}")
message(STATUS "Sentry library: ${SENTRY_LIB}")
message(STATUS "Crashpad handler: ${SENTRY_NATIVE_DIR}/bin/crashpad_handler")