#include <pch.h>
#include "InputHandler.hpp"

// As far as I know, the GM8 keycodes go up to 124 (vk_f12)
constexpr size_t NUM_KEYS = 124;
bool _current[NUM_KEYS];
bool _pressed[NUM_KEYS];
bool _released[NUM_KEYS];

GLFWwindow* win;


// Callback for when a key action gets sent to the window
// todo: this wrongly assumes numlock is on, this was fixed in a later glfw build
void key_callback(GLFWwindow* window, int k, int scancode, int action, int mods) {
	if (k != GLFW_KEY_UNKNOWN) {

		// map GLFW key to GM8 keycode
		unsigned int key;
		switch (k) {
			
			case GLFW_KEY_BACKSPACE:
				key = 8;
				break;
			case GLFW_KEY_TAB:
				key = 9;
				break;
			case GLFW_KEY_ENTER:
				key = 13;
				break;
			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_RIGHT_SHIFT:
				key = 16;
				break;
			case GLFW_KEY_LEFT_CONTROL:
			case GLFW_KEY_RIGHT_CONTROL:
				key = 17;
				break;
			case GLFW_KEY_LEFT_ALT:
			case GLFW_KEY_RIGHT_ALT:
				key = 18;
				break;
			case GLFW_KEY_PAUSE:
				key = 19;
				break;
			case GLFW_KEY_ESCAPE:
				key = 27;
				break;
			case GLFW_KEY_PAGE_UP:
				key = 33;
				break;
			case GLFW_KEY_PAGE_DOWN:
				key = 34;
				break;
			case GLFW_KEY_END:
				key = 35;
				break;
			case GLFW_KEY_HOME:
				key = 36;
				break;
			case GLFW_KEY_LEFT:
				key = 37;
				break;
			case GLFW_KEY_UP:
				key = 38;
				break;
			case GLFW_KEY_RIGHT:
				key = 39;
				break;
			case GLFW_KEY_DOWN:
				key = 40;
				break;
			case GLFW_KEY_PRINT_SCREEN:
				key = 44;
				break;
			case GLFW_KEY_INSERT:
				key = 45;
				break;
			case GLFW_KEY_DELETE:
				key = 46;
				break;
			case GLFW_KEY_KP_MULTIPLY:
				key = 106;
				break;
			case GLFW_KEY_KP_DIVIDE:
				key = 111;
				break;
			case GLFW_KEY_KP_ADD:
				key = 107;
				break;
			case GLFW_KEY_KP_SUBTRACT:
				key = 109;
				break;
			case GLFW_KEY_KP_DECIMAL:
				key = 107;
				break;
			case GLFW_KEY_KP_0:
			case GLFW_KEY_KP_1:
			case GLFW_KEY_KP_2:
			case GLFW_KEY_KP_3:
			case GLFW_KEY_KP_4:
			case GLFW_KEY_KP_5:
			case GLFW_KEY_KP_6:
			case GLFW_KEY_KP_7:
			case GLFW_KEY_KP_8:
			case GLFW_KEY_KP_9:
				key = (unsigned int)(k - 272);
				break;
			case GLFW_KEY_F1:
			case GLFW_KEY_F2:
			case GLFW_KEY_F3:
			case GLFW_KEY_F4:
			case GLFW_KEY_F5:
			case GLFW_KEY_F6:
			case GLFW_KEY_F7:
			case GLFW_KEY_F8:
			case GLFW_KEY_F9:
			case GLFW_KEY_F10:
			case GLFW_KEY_F11:
			case GLFW_KEY_F12:
				key = (unsigned int)(k - 178);
				break;
			default:
				key = (unsigned int)k;
		}
		if (key < 0 || key >= 124) return;

		if (action == GLFW_RELEASE) _released[key] = true;
		else if (!_current[key]) _pressed[key] = true;
		_current[key] = (action != GLFW_RELEASE);
	}
}

void InputInit(GLFWwindow* window) {
	memset(_current, 0, sizeof(bool) * NUM_KEYS);
	memset(_pressed, 0, sizeof(bool) * NUM_KEYS);
	memset(_released, 0, sizeof(bool) * NUM_KEYS);
	glfwSetKeyCallback(window, key_callback);
	win = window;
}

void InputUpdate() {
	memset(_pressed, 0, sizeof(bool) * NUM_KEYS);
	memset(_released, 0, sizeof(bool) * NUM_KEYS);
	glfwPollEvents();
}


bool InputCheckKey(int code) {
	if (code < 0 || code > NUM_KEYS) return false;
	return _current[code];
}

bool InputCheckKeyDirect(int code) {
	//if (code < 0 || code > NUM_KEYS) return false;
	//return glfwGetKey(win, code);
	int c = 0;
	switch (code) {
		case 37:
			c = GLFW_KEY_LEFT;
			break;
		case 38:
			c = GLFW_KEY_UP;
			break;
		case 39:
			c = GLFW_KEY_RIGHT;
			break;
		case 40:
			c = GLFW_KEY_DOWN;
			break;
		default:
			return false;
	}
	return glfwGetKey(win, c);
}

bool InputCheckKeyPressed(int code) {
	if (code < 0 || code > NUM_KEYS) return false;
	return _pressed[code];
}

bool InputCheckKeyReleased(int code) {
	if (code < 0 || code > NUM_KEYS) return false;
	return _released[code];
}

void InputClearKeys() {
	memset(_current, 0, sizeof(bool) * NUM_KEYS);
	memset(_pressed, 0, sizeof(bool) * NUM_KEYS);
	memset(_released, 0, sizeof(bool) * NUM_KEYS);
}
