#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include <vector>
#include "Assets.hpp"
struct SDL_Window;

// Wrapper for game settings
struct Settings {
	bool fullscreen;				// Start in fullscreen mode
	bool interpolate;				// Interpolate colours between pixels
	bool drawBorder;				// Draw window border when in windowed mode
	bool displayCursor;				// Display the cursor
	unsigned int scaling;			// ??? tbd 3 window scaling modes: Fixed scale (as a percentage - default 100%) / Keep aspect ratio when resizing / Full scale (Original resolution)
	bool allowWindowResize;			// Allow the user to resize the game window
	bool onTop;						// Let the game window always stay on top
	unsigned int colourOutsideRoom; // Colour outside of the room region (This appears to be RGBA format)
	bool setResolution;				// Set the resolution of the screen
	unsigned int colourDepth;		//     - Colour depth (No change / 16-bit / 32-bit)
	unsigned int resolution;		//     - Resolution (No change / 320x240 / 640x480 / 800x600 / 1024x768 / 1280x1024 / 1600x1200)
	unsigned int frequency;			//     - Frequency (No change / 60 / 70 / 85 / 100 / 120)
	bool showButtons;				// Show maximize/minimize/close buttons in windowed mode
	bool vsync;						// Enable VSync
	bool disableScreen;				// Disable screensavers and power saving actions
	bool letF4;						// Let <F4> toggle windowed and fullscreen
	bool letF1;						// Let <F1> bring up the game information window
	bool letEsc;					// Let <Esc> end the game
	bool letF5;						// Let <F5> save the game and <F6> load the game
	bool letF9;						// Let <F9> take a screenshot of the game
	bool treatCloseAsEsc;			// Treat the close button as the <Esc> key
	unsigned int priority;			// Game process priority (Normal / High / Highest)
	bool freeze;					// Freeze the game window when it loses focus
	unsigned int loadingBar;		// Loading bar data with 2 data blocks:
										// backdata: the background of the loading bar which represents remaining progress,
										// frontdata: the loading bar itself overlayed on the backdata image showing how much is loaded.
	bool customLoadImage;			// Show custom image while loading (if true, this is followed by a data block)
	bool transparent;				// Make loading image transparent
	unsigned int translucency;		// Transparency of image (0 - 255)
	bool scaleProgressBar;			// Scale progressbar image
	bool errorDisplay;				// Display error messages
	bool errorLog;					// Write error messages to game_errors.log
	bool errorAbort;				// Abort on all error messages
	bool treatAsZero;				// Treat uninitialized variables as 0
	bool errorOnUninitialization;	// Throw an error when script arguments aren't initialized correctly
};

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

	public:
		Game();
		~Game();

		// Wrapper for game settings
		Settings settings;

		// Load in game data from a file stream. Returns true on success, false on failure.
		// The Game object should be deleted on failure as it will be in an undefined state.
		bool Load(const char* filename);

		// Loads the first room of the game.
		void loadFirstRoom(SDL_Window* renderer);

		// Call this every time you want a frame advance.
		// Returns false if the game should exit, otherwise true.
		bool Frame(SDL_Window* surface);
};

#endif