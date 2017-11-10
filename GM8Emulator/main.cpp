#define CHECK_MEMORY_LEAKS 0

#include <GLFW\glfw3.h>
#include "Game.hpp"

#if CHECK_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

int main(int argc, char** argv) {
	
	glfwInit();

	Game* game = new Game();

	// If you want the runner to load the data from itself (like how normal gm8 games do it), set the game name to argv[0].
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		return 1;
	}

	if (!game->StartGame()) {
		// Starting game failed
		delete game;
		return 2;
	}

	while (true) {
		if (!game->Frame()) {
			break;
		}
	}

	delete game;

	glfwTerminate();

#if CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}