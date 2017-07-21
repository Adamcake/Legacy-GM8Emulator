#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include <vector>
#include "Assets.hpp"

// This is the overall structure for the game state.
class Game {
	private:

		// These contain the assets loaded in from the gamedata.
		std::vector<Sound*> _sounds;
		std::vector<Sprite*> _sprites;
		std::vector<Background*> _backgrounds;
		std::vector<Path*> _paths;
		std::vector<Script*> _scripts;
		std::vector<Font*> _fonts;
		std::vector<Timeline*> _timelines;
		std::vector<Object*> _objects;
		std::vector<Room*> _rooms;

	public:
		Game();
		~Game();

		// Load in game data from a file stream. Returns true on success, false on failure.
		// The Game object should be deleted on failure as it will be in an undefined state.
		bool Load(const char* filename);
};

#endif