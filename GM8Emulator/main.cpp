#include <SDL/SDL.h>
#include "Game.hpp"

int main(int argc, char** argv) {

	Game* game = new Game();



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
}