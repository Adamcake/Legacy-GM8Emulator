#define CHECK_MEMORY_LEAKS 0

#define GLEW_STATIC
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "Game.hpp"

#if CHECK_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif


#include <iostream>

int main(int argc, char** argv) {
	
	if (!glfwInit()) {
		// Failed to initialize GLFW
		return 1;
	}

	glewExperimental = GL_TRUE;
	if (!glewInit()) {
		// Failed to init GLEW
		glfwTerminate();
		return 2;
	}

	Game* game = new Game();

	// If you want the runner to load the data from itself (like how normal gm8 games do it), set the game name to argv[0].
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		glfwTerminate();
		return 3;
	}

	if (!game->StartGame()) {
		// Starting game failed
		delete game;
		glfwTerminate();
		return 4;
	}

	while (true) {
		if (!game->Frame()) {
			break;
		}
	}

	// Natural end of application
	delete game;
	glfwTerminate();

#if CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}