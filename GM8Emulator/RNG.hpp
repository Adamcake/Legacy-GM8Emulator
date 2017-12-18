#ifndef _A_RNG_HPP_
#define _A_RNG_HPP_

class RNG {
	__int32 seed;

	public:
		RNG();

		// Cycles the seed and returns a pseudo-random number between 0 and 1.
		double Random();

		// Get and set seed

		inline void SetSeed(int s) { seed = s; }
		inline int GetSeed() { return seed; }
};

#endif