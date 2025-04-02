#include <iostream>

#include "PrimeChecker.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: \n\t" << argv[0] << " <number>" << std::endl;
        return 0;
    }

    int number = std::atoi(argv[1]);
    PrimeChecker pc;

    std::cout << "Is a prime number? " << pc.isPrime(number) << std::endl;

    return 0;
}
