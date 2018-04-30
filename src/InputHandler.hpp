#ifndef _A_INPUT_HANDLER_HPP_
#define _A_INPUT_HANDLER_HPP_

#define _CRTDBG_MAP_ALLOC
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

struct GLFWwindow;
#include <vector>

void InputInit(GLFWwindow* window);
void InputUpdate();
void InputClearKeys();

bool InputCheckKey(int code);
bool InputCheckKeyDirect(int code);
bool InputCheckKeyPressed(int code);
bool InputCheckKeyReleased(int code);

//unsigned int InputCountKeys();
//unsigned int InputCountKeysPressed();
//unsigned int InputCountKeysReleased();

#endif