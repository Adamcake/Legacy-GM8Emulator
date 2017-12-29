#ifndef _A_RNG_HPP_
#define _A_RNG_HPP_

class RNG {
	__int32 seed;

	public:
		RNG();

		// Cycles the seed and returns a pseudo-random double between 0 and the given bound.
		double Random(double bound);

		// Cycles the seed and returns a pseudo-random integer between 0 and the given bound (inclusive).
		int Irandom(int bound);

		// Get and set seed

		inline void SetSeed(int s) { seed = s; }
		inline int GetSeed() { return seed; }
};

#endif