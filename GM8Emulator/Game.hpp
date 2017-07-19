#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include <vector>
#include "Assets.hpp"

// This is the overall structure for the game state.
class Game {
	private:
		// These contain the assets loaded in from the gamedata.
		std::vector<Sound> _sounds;
		std::vector<Sprite> _sounds;
		std::vector<Background> _sounds;
		std::vector<Path> _sounds;
		std::vector<Script> _sounds;
		std::vector<Font> _sounds;
		std::vector<Timeline> _sounds;
		std::vector<Object> _sounds;
		std::vector<Room> _sounds;

	public:
		Game();
		~Game();

		// Load in game data from a file stream. Returns true on success, false on failure.
		// The Game object should be deleted on failure as it will be in an undefined state.
		bool Load(const char* filename);
};

#endif