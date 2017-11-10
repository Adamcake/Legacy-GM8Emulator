#include <GLFW/glfw3.h>
#include "GameRenderer.hpp"
#include "GameSettings.hpp"

GameRenderer::GameRenderer()
{
	window = NULL;
	_images = std::vector<RImage>();
}

GameRenderer::~GameRenderer()
{
	for (RImage img : _images) {
		free(img.data);
	}
	_images.clear();
	glfwDestroyWindow(window); // This function is allowed be called on NULL
}

bool GameRenderer::MakeGameWindow(GameSettings* settings) {
	// Shouldn't really be necessary, but prevents a memory leak in the event this is called more than once.
	glfwDestroyWindow(window);

	window = glfwCreateWindow(64, 64, "", NULL, NULL);
	if (!window) {
		return false;
	}

	return true;
}

void GameRenderer::ResizeGameWindow(unsigned int w, unsigned int h) {
	glfwSetWindowSize(window, w, h);
}

void GameRenderer::SetGameWindowTitle(const char * title) {
	glfwSetWindowTitle(window, title);
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
	
}
