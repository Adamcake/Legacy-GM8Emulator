#define CHECK_MEMORY_LEAKS 0
#define OUTPUT_FRAME_TIME 0
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
#include <string>
#endif


int main(int argc, char** argv) {
	std::chrono::high_resolution_clock::time_point t1, t2;

#if OUTPUT_FRAME_TIME
	t1 = std::chrono::high_resolution_clock::now();
#endif

	Game* game = new Game();

	// This is just temp - you must place a game called "game.exe" in the project directory (or in the same directory as your built exe) to load it.
	// This can easily be changed to load from anywhere when the project is done.
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
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
		return 3;
	}

	unsigned int a = 0;
	double totMus = 0;
	while (true) {
		t1 = std::chrono::high_resolution_clock::now();
		if (!game->Frame()) {
			break;
		}
		
#if OUTPUT_FRAME_TIME
		t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		double mus = time_span.count() * 1000000.0;
		std::cout << "Frame took " << (int)mus << " microseconds" << std::endl;
#endif

		while (true) {
			t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			double mus2 = time_span.count() * 1000000.0;
			long long waitMus = (long long)((((double)1000000.0) / game->GetRoomSpeed()) - mus2);
			if (waitMus <= 0) {
				break;
			}
		}
	}

	// Natural end of application
	delete game;

#if CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}