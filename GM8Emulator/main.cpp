#define CHECK_MEMORY_LEAKS 0

#include "SDL/SDL.h"
#include "Game.hpp"

#if CHECK_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

int main(int argc, char** argv) {

	Game* game = new Game();

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window;
	window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 332, 92, SDL_WINDOW_OPENGL);
	SDL_SetWindowBordered(window, SDL_FALSE);
	SDL_SetWindowResizable(window, SDL_FALSE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// If you want the runner to load the data from itself (like how normal gm8 games do it), set the game name to argv[0].
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		return 1;
	}

	game->loadFirstRoom(window);

	while (true) {
		if (!game->Frame(window)) {
			break;
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(20);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	delete game;
	SDL_Quit();

#if CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}