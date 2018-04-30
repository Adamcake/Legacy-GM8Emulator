#ifndef _A_GAMEPRIVATEGLOBALS_HPP_
#define _A_GAMEPRIVATEGLOBALS_HPP_

#define _CRTDBG_MAP_ALLOC
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "InstanceList.hpp"
#include "GlobalValues.hpp"
#include "GameSettings.hpp"
class CodeRunner;
class CodeActionManager;

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

// All active instances
extern InstanceList _instances;

// All global values such as room_speed, health, etc
extern GlobalValues _globals;

// Object for compiling and running GML
extern CodeRunner* _runner;

// Manager for drag-n-drop code actions
extern CodeActionManager* _codeActions;

// This refers to the "game information" text box that comes up if you press F1.
extern GameInfo _info;


// Order in which rooms should be played (this is different than the resource tree order)
extern unsigned int* _roomOrder;
extern unsigned int _roomOrderCount;

// Wrapper for game settings
extern GameSettings settings;

// Misc game variables
extern unsigned int _lastUsedRoomSpeed;

#endif