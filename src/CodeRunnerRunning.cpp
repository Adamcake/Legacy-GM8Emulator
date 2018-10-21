#include <pch.h>
#include "CRGMLType.hpp"
#include "CodeRunner.hpp"

/*
int CodeRunner::_round(double d) {
    // This mimics the x86_32 "FISTP" operator which is commonly used in the GM8 runner.
    // We can't actually use that operator, because we're targeting other platforms than x86 Windows.
    int down = ( int )d;
    if ((d - down) < 0.5) return down;
    if ((d - down) > 0.5) return (down + 1);
    return down + (down & 1);
}

bool Runtime::_equal(double d1, double d2) {
    // I have no idea why GM8 does this, but it does.
    double difference = fabs(d2 - d1);
    double cut_digits = ::floor(difference * 1e13) / 1e13;
    return cut_digits == 0.0;
}

bool Runtime::_isTrue(const GMLType* value) {
	return (value->state == GMLTypeState::Double) && (value->dVal >= 0.5);
}
*/


bool CodeRunner::Run(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc, GMLType* argv) {
    return false;
}

bool CodeRunner::Query(CodeObject code, Instance* self, Instance* other, bool* response) {
	return false;
}