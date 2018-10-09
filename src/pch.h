#pragma once

// C++ Standard
#include <algorithm>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <new>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <vector>
namespace fs = std::filesystem; // comfy alias

// GM8Emulator
#include "CREnums.hpp"
#include "File.hpp"

// Third Party

#define GLEW_STATIC // glew
#include <gl/glew.h> // glew
#include <GLFW/glfw3.h> // GLFW
#include <zlib.h> // zlib
#include <finders_interface.h> // rectpack2D

constexpr double GMLTrue = 1.0;
constexpr double GMLFalse = 0.0;
