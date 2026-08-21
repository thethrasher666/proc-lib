#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Configure files.
configure_file("version.hxx.in" "version.hxx")

# Define the target.
add_library(proc-lib STATIC
    "error.cxx"
    "process.cxx"
)

# Set the target's defaults.
pl_set_target_defaults(proc-lib)

# Set the target's include directories.
target_include_directories(proc-lib
    PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}/.."
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

# Set the target's dependencies.
target_link_libraries(proc-lib
    PRIVATE
        proc-lib::platform
)

# Add alias library.
add_library(proc-lib::proc-lib ALIAS proc-lib)

# Add the platform-specific implementations.
add_subdirectory("posix")
add_subdirectory("win32")
