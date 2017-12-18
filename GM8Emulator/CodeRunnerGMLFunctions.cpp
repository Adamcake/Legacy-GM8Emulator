#include "CodeRunner.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "RNG.hpp"

/*
All GML functions have this format:
bool <gml_function_name>(unsigned int argc, GMLType* argv, GMLType* out)

argc is the number of arguments being passed. argv is the array of arguments.
out is where you should return the GML value being returned by the function.
The function should return true on success, false on failure (ie. game should close.)

Note that argc hasn't been verified as correct when passed in. So, for example, instance_create isn't guaranteed to have
exactly 3 arguments, even though it's always meant to. So it should return false if it doesn't have the correct argc.
That said, it SHOULD always be correct unless some gamedata has been manufactured otherwise, so we'll omit checks for efficiency.

Your implementation must account for "out" being NULL. For example, instance_create() should output the new instance id,
but if out is NULL then it doesn't output anything, as the runner has indicated that the output isn't required.
*/

bool CodeRunner::execute_string(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::instance_create(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_STRING || argv[1].state == GML_TYPE_STRING || argv[2].state == GML_TYPE_STRING) return false;
	_instances->AddInstance(_nextInstanceID, argv[0].dVal, argv[1].dVal, _round(argv[2].dVal));
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)_nextInstanceID;
	}
	_nextInstanceID++;
	// TODO: call new instance's create event here
	return true;
}

bool CodeRunner::instance_destroy(unsigned int argc, GMLType* argv, GMLType* out) {
	_instances->GetInstanceByNumber(_contexts.top().self)->exists = false;
	return true;
}

bool CodeRunner::irandom(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::irandom_range(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::make_color_hsv(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::move_wrap(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[2].state == GML_TYPE_STRING) return false;
	bool hor = _isTrue(argv + 0);
	bool ver = _isTrue(argv + 1);
	int margin = argv[2].dVal;
	Instance* instance = _instances->GetInstanceByNumber(_contexts.top().self);

	if (hor) {
		unsigned int roomW = _assetManager->GetRoom(_globalValues->room)->width;
		if (instance->x < -margin) {
			instance->x += roomW;
		}
		else if (instance->x >= (roomW + margin)) {
			instance->x -= roomW;
		}
	}

	if (ver) {
		unsigned int roomH = _assetManager->GetRoom(_globalValues->room)->height;
		if (instance->y < -margin) {
			instance->y += roomH;
		}
		else if (instance->y >= (roomH + margin)) {
			instance->y -= roomH;
		}
	}

	return true;
}

bool CodeRunner::random(unsigned int argc, GMLType* argv, GMLType* out) {
	double rand = _rng->Random();
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = argv[0].dVal * rand;
	}
	return true;
}

bool CodeRunner::random_range(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::room_goto(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::room_goto_next(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}

bool CodeRunner::room_goto_previous(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return false;
}