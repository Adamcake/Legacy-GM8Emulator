#include <SDL/SDL.h>
#include "Game.hpp"

int main(int argc, char** argv) {

	Game* game = new Game();

	// If you want the runner to load the data from itself (like how normal gm8 games do it), set the game name to argv[0].
	if (!game->Load("game.exe")) {
		// Load failed
		delete game;
		return 1;
	}

	SDL_Window* test;
	SDL_Event event;

	SDL_Init(SDL_INIT_EVERYTHING);
	test = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	while (1) {
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				SDL_Quit();
				return 0;
			}
		}
		SDL_Delay(1);
	}

	SDL_DestroyWindow(test);
	delete game;
	return 0;
}