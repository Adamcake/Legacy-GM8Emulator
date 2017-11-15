#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "GameRenderer.hpp"
#include "GameSettings.hpp"


GameRenderer::GameRenderer() {
	window = NULL;
	_pixels = NULL;
	_images = std::vector<RImage>();
}

GameRenderer::~GameRenderer() {
	for (RImage img : _images) {
		free(img.data);
	}
	free(_pixels);
	_images.clear();
	glfwDestroyWindow(window); // This function is allowed be called on NULL
}

bool GameRenderer::MakeGameWindow(GameSettings* settings, unsigned int w, unsigned int h) {
	// Shouldn't really be necessary, but prevents a memory leak in the event this is called more than once.
	glfwDestroyWindow(window);
	free(_pixels);

	window = glfwCreateWindow(w, h, "", NULL, NULL);
	if (!window) {
		return false;
	}
	_pixels = (unsigned char*)malloc(w * h * 4);
	width = w;
	height = h;

	// Make this the current GL context for this thread. Rendering assumes that this will never be changed.
	glfwMakeContextCurrent(window);

	return true;
}

void GameRenderer::ResizeGameWindow(unsigned int w, unsigned int h) {
	glfwSetWindowSize(window, w, h);
}

void GameRenderer::SetGameWindowTitle(const char * title) {
	glfwSetWindowTitle(window, title);
}

bool GameRenderer::ShouldClose() {
	return glfwWindowShouldClose(window);
}

RImageIndex GameRenderer::MakeImage(unsigned int w, unsigned int h, unsigned char * bytes) {
	RImage img;
	img.w = w;
	img.h = h;
	img.data = (unsigned char*)malloc(w * h * 4);
	memcpy(img.data, bytes, (w * h * 4));

	_images.push_back(img);
	return _images.size() - 1;
}

void GameRenderer::DrawImage(RImageIndex ix, int x, int y) {
	RImage* i = _images._Myfirst() + ix;
	for (unsigned int row = 0; row < i->h; row++) {
		if ((y + row < 0) || (y + row >= height)) continue;
		for (unsigned int pixel = 0; pixel < i->w; pixel++) {
			if ((x + pixel < 0) || (x + pixel >= width)) continue;

			unsigned char* sprPixelPtr = i->data + (((row * i->w) + pixel) * 4);
			unsigned char* screenPixelPtr = _pixels + ((((height - (y + row) - 1) * width) + (x + pixel)) * 4);
			if((*sprPixelPtr) & 0xFF)
				memcpy(screenPixelPtr, sprPixelPtr, 4);
		}
	}
}

void GameRenderer::RenderFrame() {
	glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, _pixels);
	glfwSwapBuffers(window);
	glfwPollEvents(); // This probably won't be here later on, but it needs to happen every frame for the window to update properly.
	memset(_pixels, 0, (width * height * 4)); // Clear screen - need to make this use the colour in game settings
}