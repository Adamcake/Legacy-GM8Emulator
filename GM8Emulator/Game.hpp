#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include <vector>
#include "Assets.hpp"

// Wrapper for game settings
struct Settings {
	bool fullscreen;				// Does the game start in fullscreen?
	bool interpolate;				// Interpolate movement between keyframes?
	bool drawBorder;				// Draw the window border?
	bool displayCursor;				// Display the mouse cursor when it's inside the game window?
	unsigned int scaling;			// ?
	bool allowWindowResize;			// Allow resizing of the game window?
	bool onTop;						// Should the game window be "always on top"?
	unsigned int colourOutsideRoom; // ? (This appears to be RGBA format)
	bool setResolution;				// ?
	unsigned int colourDepth;		// ?
	unsigned int resoltuion;		// ?
	unsigned int frequency;			// ?
	bool showButtons;				// ? (I think this refers to the window minimize/maximize/close buttons)
	bool vsync;						// ?
	bool disableScreen;				// ?
	bool letF4;						// ?
	bool letF1;						// ?
	bool letEsc;					// ?
	bool letF5;						// ?
	bool letF9;						// ?
	bool treatCloseAsEsc;			// ?
	unsigned int priority;			// ?
	bool freeze;					// ?
	unsigned int loadingBar;		// ? (this is followed by up to two data blocks called "backdata" and "frontdata")
	bool customLoadImage;			// Is there a custom load image? (if true, this is followed by a data block)
	bool transparent;				// ?
	unsigned int translucency;		// ?
	bool scaleProgressBar;			// ?
	bool errorDisplay;				// ?
	bool errorLog;					// ?
	bool errorAbort;				// ?
	unsigned int treatAsZero;		// ?
};

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

		Settings settings;

		// Load in game data from a file stream. Returns true on success, false on failure.
		// The Game object should be deleted on failure as it will be in an undefined state.
		bool Load(const char* filename);
};

#endif