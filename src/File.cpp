#include <pch.h>
#include <fstream>
#include "File.hpp"

bool FileExists(const char* filename) {
	std::fstream f(filename);
	bool r = f.good();
	f.close();
	return r;
}