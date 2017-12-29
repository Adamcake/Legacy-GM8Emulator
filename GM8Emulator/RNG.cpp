#include "RNG.hpp"
#include <ctime>

RNG::RNG() {
	seed = (int)std::time(NULL);
}

double RNG::Random(double bound) {
	seed *= 0x8088405;
	seed++;
	return (((unsigned int)seed) * (1.0 / 0x100000000) * bound);
}

int RNG::Irandom(int bound) {
	seed *= 0x8088405;
	seed++;
	long long v = ((long long)seed) * ((long long)bound);
	v >>= 32;
	return (int)v;
}