#define CHECK_MEMORY_LEAKS 0
#define OUTPUT_FRAME_TIME 0

#include <GLFW/glfw3.h>
#include "Game.hpp"

#if CHECK_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

#if OUTPUT_FRAME_TIME
#include <iostream>
#include <chrono>
#endif

int main(int argc, char** argv) {
	if (!glfwInit()) {
		// Failed to initialize GLFW
		return 1;
	}

	Game* game = new Game();

	// If you want the runner to load the data from itself (like how normal gm8 games do it), set the game name to argv[0].
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		glfwTerminate();
		return 2;
	}

	if (!game->StartGame()) {
		// Starting game failed
		delete game;
		glfwTerminate();
		return 3;
	}

	while (true) {
#if OUTPUT_FRAME_TIME
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
#endif
		if (!game->Frame()) {
			break;
		}
#if OUTPUT_FRAME_TIME
		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		std::cout << "Frame took " << time_span.count() << " seconds" << std::endl;
#endif
	}

	// Natural end of application
	delete game;
	glfwTerminate();

#if CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}