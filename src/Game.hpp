#ifndef _GM8_GAME_HPP_
#define _GM8_GAME_HPP_
#include "AssetManager.hpp"
#include "GameSettings.hpp"
#include "InstanceList.hpp"
#include "GlobalValues.hpp"
class CodeRunner;


void GameInit();
void GameTerminate();

// Load in game data from a file stream. Returns true on success, false on failure.
// The Game object should be deleted on failure as it will be in an undefined state.
bool GameLoad(const char* filename);

// Opens a window for the game and loads the first room.
// Returns true if successful, otherwise false.
bool GameStart();

// Discards the current room and loads a new one with the given index. Does nothing if we're already in this room.
// Can also be passed ROOM_TO_NEXT or ROOM_TO_PREV to load the next or previous room in the room order.
// Returns true on success, false if the id is invalid. If this returns false, the application should exit.
bool GameLoadRoom(int id);

// Call this every time you want a frame advance.
// Returns false if the game should exit, otherwise true.
bool GameFrame();

// Gets the current room_speed.
unsigned int GameGetRoomSpeed();

// Checks if the game closed because of an error and, if so, the associated error message
bool GameGetError(const char**);

// Get event holder lists
std::map<unsigned int, std::vector<unsigned int>>& GetEventHolderList(unsigned int ev);
std::vector<unsigned int>& GetEventHolderList(unsigned int ev, unsigned int sub);

#endif