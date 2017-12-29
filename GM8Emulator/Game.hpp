#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include "AssetManager.hpp"
#include "GameSettings.hpp"
#include "InstanceList.hpp"
#include "GlobalValues.hpp"
class GameRenderer;
class CodeRunner;
class CodeActionManager;


// This is the overall structure for the game state.
class Game {
	private:
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

		// Asset manager for all game asset types
		AssetManager _assetManager;

		// All active instances
		InstanceList _instances;

		// All global values such as room_speed, health, etc
		GlobalValues _globals;

		// Object for compiling and running GML
		CodeRunner* _runner;

		// Manager for drag-n-drop code actions
		CodeActionManager* _codeActions;

		// This refers to the "game information" text box that comes up if you press F1.
		GameInfo _info;


		// Order in which rooms should be played (this is different than the resource tree order)
		unsigned int* _roomOrder;

		// Renderer object for the game window.
		GameRenderer* _renderer;

		// Wrapper for game settings
		GameSettings settings;

		// Misc game variables
		unsigned int _lastUsedRoomSpeed;

	public:
		Game();
		~Game();

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

		// Sets all the values of an Instance to their defaults. Returns true on success, otherwise false (game should close.)
		// bool InitInstance(Instance* instance, double x, double y, unsigned int objectIndex);

		// Gets the current room_speed.
		inline unsigned int GetRoomSpeed() const { return _globals.room_speed; }
};

#endif