#ifndef _A_GAMERENDERER_HPP_
#define _A_GAMERENDERER_HPP_
struct GameSettings;
struct GLFWwindow;

typedef unsigned int RImageIndex;
#include <vector>

class GameRenderer {
	private:
		struct RImage {
			unsigned int w;
			unsigned int h;
			unsigned char* data;
		};

		GLFWwindow* window;
		std::vector<RImage> _images;

	public:
		GameRenderer();
		~GameRenderer();

		// Creates the main game window, should be called after all loading is done. Returns true on success, otherwise false.
		bool MakeGameWindow(GameSettings* settings);

		// Resize game window
		void ResizeGameWindow(unsigned int w, unsigned int h);

		// Set window title/caption
		void SetGameWindowTitle(const char* title);


		// Registers an image in the renderer. Assumes 32-bit pixels in RGBA format (which is how it is in the EXE.) 
		RImageIndex MakeImage(unsigned int w, unsigned int h, unsigned char* bytes);

		// Draws a registered image at the given X and Y
		void DrawImage(RImageIndex ix, int x, int y);
};

#endif