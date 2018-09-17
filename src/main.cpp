#include <pch.h>
#define CHECK_MEMORY_LEAKS 0
#define OUTPUT_FRAME_TIME 1
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
#if CHECK_MEMORY_LEAKS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::chrono::high_resolution_clock::time_point t1, t2, t3;

#if OUTPUT_FRAME_TIME
	t1 = std::chrono::high_resolution_clock::now();
#endif

	GameInit();

	// This is just temp - you must place a game called "game.exe" in the project directory (or in the same directory as your built exe) to load it.
	// This can easily be changed to load from anywhere when the project is done.
	if (!GameLoad("game.exe")) {
		// Load failed
		GameTerminate();
		return 2;
	}

#if OUTPUT_FRAME_TIME
	t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	double se = time_span.count();
	std::cout << "Successful load in " << se << " seconds" << std::endl;
#endif

	if (!GameStart()) {
		// Starting game failed
		GameTerminate();
		return 3;
	}

#if OUTPUT_FRAME_TIME
	t3 = std::chrono::high_resolution_clock::now();
	time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t1);
	se = time_span.count();
	std::cout << "Successful game start in " << se << " seconds" << std::endl;
#endif

	unsigned int a = 0;
	double totMus = 0;
	while (true) {
		t1 = std::chrono::high_resolution_clock::now();
		if (!GameFrame()) {
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
			long long waitMus = (long long)((((double)1000000.0) / GameGetRoomSpeed()) - mus2);
			if (waitMus <= 0) {
				break;
			}
		}
	}

	// Natural end of application
	GameTerminate();
	return 0;
}