#pragma once

// C++ Standard
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <fstream>
// #include <filesystem>
#include <new>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <set>
// namespace fs = std::filesystem; // comfy alias

// Third Party
#include <glad/glad.h> // GLAD
#include <GLFW/glfw3.h> // GLFW
#include <zlib.h> // zlib
#include <finders_interface.h> // rectpack2D

constexpr double GMLTrue = 1.0;
constexpr double GMLFalse = 0.0;
constexpr double GML_PI = 3.141592654;  // Actual value of PI used by the official runner. Please don't make it more accurate.