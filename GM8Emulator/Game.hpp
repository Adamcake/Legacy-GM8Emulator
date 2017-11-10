#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include <vector>
#include "Assets.hpp"
#include "GameSettings.hpp"


// Wrapper for game information
struct GameInfo {
	unsigned int backgroundColour;
	bool separateWindow;
	char* caption;
	unsigned int left;
	unsigned int top;
	unsigned int width;
	unsigned int height;
	bool showBorder;
	bool allowWindowResize;
	bool onTop;
	bool freezeGame;
	char* gameInfo;
};

// This is the overall structure for the game state.
class Game {
	private:
		// These contain the assets loaded in from the gamedata.
		std::vector<Extension> _extensions;
		std::vector<Trigger> _triggers;
		std::vector<Constant> _constants;

		std::vector<Sound> _sounds;
		std::vector<Sprite> _sprites;
		std::vector<Background> _backgrounds;
		std::vector<Path> _paths;
		std::vector<Script> _scripts;
		std::vector<Font> _fonts;
		std::vector<Timeline> _timelines;
		std::vector<Object> _objects;
		std::vector<Room> _rooms;
		
		std::vector<IncludeFile> _includeFiles;

		// This refers to the "game information" text box that comes up if you press F1.
		GameInfo info;

		// Order in which rooms should be played (this is different than the resource tree order)
		unsigned int* roomOrder;

		// Renderer object for the game window.
		GameRenderer* renderer;

		// Misc game variables
		unsigned int roomId;

	public:
		Game();
		~Game();

		// Wrapper for game settings
		GameSettings settings;

		// Load in game data from a file stream. Returns true on success, false on failure.
		// The Game object should be deleted on failure as it will be in an undefined state.
		bool Load(const char* filename);

		// Opens a window for the game and loads the first room.
		// Returns true if successful, otherwise false.
		bool StartGame();

		// Discards the current room and loads a new one with the given index. Does nothing if we're already in this room.
		// Returns true on success, false if the id is invalid. If this returns false, the application should exit.
		bool LoadRoom(unsigned int id);

		// Call this every time you want a frame advance.
		// Returns false if the game should exit, otherwise true.
		bool Frame();
};

#endif