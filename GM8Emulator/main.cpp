#include "SDL/SDL.h"
#include "Game.hpp"

int main(int argc, char** argv) {

	Game* game = new Game();

	// If you want the runner to load the data from itself (like how normal gm8 games do it), set the game name to argv[0].
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		return 1;
	}

	SDL_Window* window;
	SDL_Event event;

	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	unsigned int frame = 0;

	while (true) {
		if (!game->Frame(SDL_GetWindowSurface(window), frame)) {
			break;
		}

		frame++;

		SDL_UpdateWindowSurface(window);
		SDL_Delay(1);
	}

	SDL_DestroyWindow(window);
	delete game;
	SDL_Quit();
	return 0;
}