#include "RNG.hpp"
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

int32_t seed;

void _cycleSeed() {
    seed *= 0x8088405;
    seed++;
}

void RNG::Randomize() {
    seed = static_cast<int>(time(NULL));
    _cycleSeed();
}

double RNG::Random(double bound) {
    _cycleSeed();
    return (static_cast<unsigned int>(seed) * 0.00000000023283064365386962890625 * bound);
}

int RNG::Irandom(int bound) {
    _cycleSeed();
    unsigned long long ls = static_cast<unsigned long long>(seed) & 0xFFFFFFFF;
    long long lb = static_cast<long long>(bound) + 1;
    unsigned long long v = ls * lb;
    v >>= 32;
    return static_cast<int>(v);
}

void RNG::SetSeed(int s) { seed = s; }

int RNG::GetSeed() { return seed; }
