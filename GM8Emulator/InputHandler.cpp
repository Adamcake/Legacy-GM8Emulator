#include "InputHandler.hpp"
#include <GLFW/glfw3.h>

// GLFW_KEY_LAST tells us what the largest key code is - at time of writing it's 348, so we need 349 entries.
bool _pressed[GLFW_KEY_LAST + 1];
bool _pressedPrevious[GLFW_KEY_LAST + 1];
GLFWwindow* win;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key != GLFW_KEY_UNKNOWN) {
		_pressed[key] = (action != GLFW_RELEASE);
	}
}

void InputInit(GLFWwindow* window) {
	glfwSetKeyCallback(window, key_callback);
	win = window;
}

void InputUpdate() {
	memcpy(_pressedPrevious, _pressed, sizeof(bool) * (GLFW_KEY_LAST + 1));
	glfwPollEvents();
}


bool InputCheckKey(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return _pressed[code];
}

bool InputCheckKeyDirect(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return glfwGetKey(win, code);
}

bool InputCheckKeyPressed(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return _pressed[code] && !_pressedPrevious[code];
}

bool InputCheckKeyReleased(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return _pressedPrevious[code] && !_pressed[code];
}

unsigned int InputCountKeys() {
	unsigned int count = 0;
	for (unsigned int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (_pressed[i]) count++;
	}
	return count;
}

unsigned int InputCountKeysPressed() {
	unsigned int count = 0;
	for (unsigned int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (_pressed[i] && !_pressedPrevious[i]) count++;
	}
	return count;
}

unsigned int InputCountKeysReleased() {
	unsigned int count = 0;
	for (unsigned int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (_pressedPrevious[i] && !_pressed[i]) count++;
	}
	return count;
}