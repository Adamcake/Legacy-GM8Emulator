#pragma once

// C++ Standard
#include <algorithm>
#include <ctime>
#include <fstream>
#include <filesystem>
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
#define GLEW_STATIC
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <zlib.h>

constexpr double GMLTrue = 1.0;
constexpr double GMLFalse = 0.0;
