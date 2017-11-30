#ifndef _A_GAMERENDERER_HPP_
#define _A_GAMERENDERER_HPP_
struct GameSettings;
struct GLFWwindow;
typedef unsigned int GLuint;
typedef int GLint;
typedef float GLfloat;

typedef unsigned int RImageIndex;
#include <vector>

class GameRenderer {
	private:
		struct RImage {
			unsigned int w;
			unsigned int h;
			unsigned int originX;
			unsigned int originY;
			unsigned char* data;
			GLuint glTexObject;
			unsigned int glIndex;
			bool registered;
		};

		void _RegImg(unsigned int ix, RImage* i);
		void _DrawTex(unsigned int ix, GLuint obj);

		std::vector<RImage> _images;

		GLFWwindow* window;
		bool contextSet;
		unsigned int windowW;
		unsigned int windowH;

		unsigned int _maxGpuTextures;
		unsigned int _gpuTextures;

		GLuint _glProgram;
		GLuint _vao;
		GLuint _vbo;

	public:
		GameRenderer();
		~GameRenderer();

		// Creates the main game window, should be called after all loading is done. Returns true on success, otherwise false.
		bool MakeGameWindow(GameSettings* settings, unsigned int w, unsigned int h);

		// Resize game window
		void ResizeGameWindow(unsigned int w, unsigned int h);

		// Set window title/caption
		void SetGameWindowTitle(const char* title);

		// Get cursor pos
		void GetCursorPos(double* xpos, double* ypos);

		// Our GL window will tell us when it's supposed to close, then we have to close it. This is for getting that value.
		bool ShouldClose();


		// Registers an image in the renderer. Assumes 32-bit pixels in RGBA format (which is how it is in the EXE.) 
		RImageIndex MakeImage(unsigned int w, unsigned int h, unsigned int originX, unsigned int originY, unsigned char* bytes);

		// Draws a registered image at the given X and Y. Tries to imitate draw_sprite_ext() from GML.
		void DrawImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha);

		// Render the current frame. This should be done exactly once per cycle.
		void RenderFrame();

};

#endif