#ifndef _A_GAMESETTINGS_HPP_
#define _A_GAMESETTINGS_HPP_


// Wrapper for game settings
struct GameSettings {
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

#endif