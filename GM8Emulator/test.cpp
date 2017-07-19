#include <SDL/SDL.h>

int main(int argc, char** argv) {
	SDL_Window* test;
	SDL_Event event;

	SDL_Init(SDL_INIT_EVERYTHING);
	test = SDL_CreateWindow("Hey look, it works!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	while (1) {
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				SDL_Quit();
				return 0;
			}
		}
		SDL_Delay(1);
	}
}

// wew