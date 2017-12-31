#include "RNG.hpp"
#include <ctime>

__int32 seed;

void RNGRandomize() {
	seed = (int)std::time(NULL);
}

double RNGRandom(double bound) {
	seed *= 0x8088405;
	seed++;
	return (((unsigned int)seed) * (1.0 / 0x100000000) * bound);
}

int RNGIrandom(int bound) {
	seed *= 0x8088405;
	seed++;
	long long v = ((long long)seed) * ((long long)bound);
	v >>= 32;
	return (int)v;
}

void RNGSetSeed(int s) {
	seed = s;
}

int RNGGetSeed() {
	return seed;
}