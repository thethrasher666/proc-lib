#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Define the target.
add_executable(test_proc_lib
    "test-version.cxx"
)

# Set the defaults.
pl_set_target_defaults(test_proc_lib)

# Set the target's dependencies.
target_link_libraries(test_proc_lib
    PRIVATE
        proc-lib::proc-lib
        Catch2::Catch2WithMain
)

# Discover the tests.
catch_discover_tests(test_proc_lib)
