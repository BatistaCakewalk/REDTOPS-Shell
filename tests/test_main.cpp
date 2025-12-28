#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

TEST_CASE("My first test", "[test_tag]") {
    REQUIRE(1 + 1 == 2);
}