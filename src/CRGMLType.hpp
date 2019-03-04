#pragma once
#include <string>

// States a GMLType can be in
enum struct GMLTypeState { Double, String };

// The universal data type in GML
struct GMLType {
    GMLTypeState state = GMLTypeState::Double;
    double dVal = 0.0;
    std::string sVal;
};
