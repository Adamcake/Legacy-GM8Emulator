#include "RNG.hpp"
#include <ctime>

RNG::RNG() {
	seed = (int)std::time(NULL);
}

double RNG::Random() {
	seed *= 0x8088405;
	seed++;
	return (seed * (1.0 / 0x100000000)) + 0.5;
}