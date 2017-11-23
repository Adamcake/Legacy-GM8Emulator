#define CHECK_MEMORY_LEAKS 0
#define OUTPUT_FRAME_TIME 0

#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>
#include "Game.hpp"

#if CHECK_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

#if OUTPUT_FRAME_TIME
#include <iostream>
#endif


int main(int argc, char** argv) {
	std::chrono::high_resolution_clock::time_point t1;
	std::chrono::high_resolution_clock::time_point t2;

#if OUTPUT_FRAME_TIME
	t1 = std::chrono::high_resolution_clock::now();
#endif

	if (!glfwInit()) {
		// Failed to initialize GLFW
		return 1;
	}

	Game* game = new Game();

	// This is just temp - you must place a game called "game.exe" in the project directory (or in the same directory as your built exe) to load it.
	// This can easily be changed to load from anywhere when the project is done.
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		glfwTerminate();
		return 2;
	}

#if OUTPUT_FRAME_TIME
	t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	double se = time_span.count();
	std::cout << "Successful load in " << se << " seconds" << std::endl;
#endif

	if (!game->StartGame()) {
		// Starting game failed
		delete game;
		glfwTerminate();
		return 3;
	}

	while (true) {
		t1 = std::chrono::high_resolution_clock::now();
		if (!game->Frame()) {
			break;
		}
		t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		double mus = time_span.count() * 1000000.0;
		long long waitMus = (long long)((((double)1000000.0) / game->GetRoomSpeed()) - mus);
		if (waitMus > 0) {
			std::this_thread::sleep_for(std::chrono::microseconds(waitMus));
		}
#if OUTPUT_FRAME_TIME
		std::cout << "Frame took " << mus << " microseconds, waiting " << waitMus << " microseconds" << std::endl;
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