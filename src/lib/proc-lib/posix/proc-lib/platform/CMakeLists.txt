#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Define the target.
add_library(proc-lib.platform OBJECT
    "process.cxx"
)

# Set the target's defaults.
pl_set_target_defaults(proc-lib.platform)

target_include_directories(proc-lib.platform
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../"

    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../"
)

# Add alias library.
add_library(proc-lib::platform ALIAS proc-lib.platform)
