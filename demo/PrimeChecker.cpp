#include "PrimeChecker.hpp"

bool PrimeChecker::isPrime(uint16_t n) {
    bool returnValue = true;

    if (n == 2) return true;

    if (n < 2 || n % 2 == 0) {
        returnValue = false;
    } else {
        for (uint16_t i = 2; i * i <= n; i++) {
            if (n % i == 0) {
                return false;
            }
        }
    }

    return returnValue;
}
