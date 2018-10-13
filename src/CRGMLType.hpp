#ifndef _A_CRGMLTYPE_HPP_
#define _A_CRGMLTYPE_HPP_
#include <pch.h>

// States a GMLType can be in
enum struct GMLTypeState {
	Double,
	String
};

// The universal data type in GML
struct GMLType {
	GMLTypeState state = GMLTypeState::Double;
	double dVal = 0.0;
	std::string sVal;
};

#endif