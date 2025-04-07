#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "PrimeChecker.hpp"

TEST_CASE("Test PrimeChecker is prime") {
    PrimeChecker pc;
    REQUIRE(pc.isPrime(5));
}

TEST_CASE("Test PrimeChecker is not prime") {
    PrimeChecker pc;
    REQUIRE(!pc.isPrime(4));
}
