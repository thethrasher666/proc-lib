//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include <catch2/catch_test_macros.hpp>

#include <proc-lib/version.hxx>

namespace test
{
    TEST_CASE("test.proc-lib.version")
    {
        REQUIRE(pl::version::major() == 0);
        REQUIRE(pl::version::minor() == 0);
        REQUIRE(pl::version::patch() == 1);
        REQUIRE(pl::version::tweak() == 0);
    }
} // namespace test
