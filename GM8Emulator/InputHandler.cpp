#include "InputHandler.hpp"
#include <GLFW/glfw3.h>

// GLFW_KEY_LAST tells us what the largest key code is - at time of writing it's 348, so we need 349 entries.
bool _current[GLFW_KEY_LAST + 1];
bool _pressed[GLFW_KEY_LAST + 1];
bool _released[GLFW_KEY_LAST + 1];

GLFWwindow* win;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key != GLFW_KEY_UNKNOWN) {
		if (action == GLFW_RELEASE) _released[key] = true;
		else if (!_current[key]) _pressed[key] = true;
		_current[key] = (action != GLFW_RELEASE);
	}
}

void InputInit(GLFWwindow* window) {
	memset(_current, 0, sizeof(bool) * GLFW_KEY_LAST);
	memset(_pressed, 0, sizeof(bool) * GLFW_KEY_LAST);
	memset(_released, 0, sizeof(bool) * GLFW_KEY_LAST);
	glfwSetKeyCallback(window, key_callback);
	win = window;
}

void InputUpdate() {
	memset(_pressed, 0, sizeof(bool) * GLFW_KEY_LAST);
	memset(_released, 0, sizeof(bool) * GLFW_KEY_LAST);
	glfwPollEvents();
}


bool InputCheckKey(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return _current[code];
}

bool InputCheckKeyDirect(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return glfwGetKey(win, code);
}

bool InputCheckKeyPressed(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return _pressed[code];
}

bool InputCheckKeyReleased(int code) {
	if (code < 0 || code > GLFW_KEY_LAST) return false;
	return _released[code];
}

unsigned int InputCountKeys() {
	unsigned int count = 0;
	for (unsigned int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (_pressed[i]) count++;
	}
	return count;
}

void InputClearKeys() {
	memset(_current, 0, sizeof(bool) * GLFW_KEY_LAST);
	memset(_pressed, 0, sizeof(bool) * GLFW_KEY_LAST);
	memset(_released, 0, sizeof(bool) * GLFW_KEY_LAST);
}

unsigned int InputCountKeysPressed() {
	unsigned int count = 0;
	for (unsigned int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (_pressed[i]) count++;
	}
	return count;
}

unsigned int InputCountKeysReleased() {
	unsigned int count = 0;
	for (unsigned int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (_released[i]) count++;
	}
	return count;
}