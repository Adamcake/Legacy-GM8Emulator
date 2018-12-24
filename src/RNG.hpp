#pragma once

namespace RNG {
    // Sets the seed to a new value unrelated to the previous one, based on the current unix time.
    // Ideally you should call this before using RNG functions.
    void Randomize();

    // Cycles the seed and returns a pseudo-random double between 0 and the given bound.
    double Random(double bound);

    // Cycles the seed and returns a pseudo-random integer between 0 and the given bound (inclusive).
    int Irandom(int bound);

    // Get and set seed
    void SetSeed(int s);
    int GetSeed();
};
