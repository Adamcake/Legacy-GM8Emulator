#ifndef _A_RNG_HPP_
#define _A_RNG_HPP_

// Sets the seed to a new value unrelated to the previous one, based on the current unix time.
// Ideally you should call this before using RNG functions.
void RNGRandomize();

// Cycles the seed and returns a pseudo-random double between 0 and the given bound.
double RNGRandom(double bound);

// Cycles the seed and returns a pseudo-random integer between 0 and the given bound (inclusive).
int RNGIrandom(int bound);

// Get and set seed
void RNGSetSeed(int s);
int RNGGetSeed();

#endif