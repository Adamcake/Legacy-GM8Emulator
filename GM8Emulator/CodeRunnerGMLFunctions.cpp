#include "CodeRunner.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "RNG.hpp"
#include "CodeActionManager.hpp"

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
	unsigned int objID = _round(argv[2].dVal);
	_instances->AddInstance(_nextInstanceID, argv[0].dVal, argv[1].dVal, objID);
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)_nextInstanceID;
	}
	Object* o = _assetManager->GetObject(objID);
	if (!_codeActions->Run(o->evCreate, o->evCreateActionCount, _nextInstanceID, NULL)) return false;
	_nextInstanceID++;
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
	if (argv[0].state == GML_TYPE_STRING || argv[1].state == GML_TYPE_STRING || argv[2].state == GML_TYPE_STRING) return false;
	if (out) {
		double h = argv[0].dVal;
		double s = argv[1].dVal;
		double v = argv[2].dVal;
		unsigned char r, g, b;

		if (s == 0) {
			r = v;
			g = v;
			b = v;
		}
		else {
			int i;
			double f, p, q, t;
			h /= (256.0/6.0);
			s /= 255.0;
			v /= 255.0;

			i = (int)trunc(h);
			f = h - i;
			p = v * (1.0 - s);
			q = v * (1.0 - (s * f));
			t = v * (1.0 - (s * (1.0 - f)));

			switch (i)
			{
			case 0:
				r = (unsigned char)(v * 255);
				g = (unsigned char)(t * 255);
				b = (unsigned char)(p * 255);
				break;

			case 1:
				r = (unsigned char)(q * 255);
				g = (unsigned char)(v * 255);
				b = (unsigned char)(p * 255);
				break;

			case 2:
				r = (unsigned char)(p * 255);
				g = (unsigned char)(v * 255);
				b = (unsigned char)(t * 255);
				break;

			case 3:
				r = (unsigned char)(p * 255);
				g = (unsigned char)(q * 255);
				b = (unsigned char)(v * 255);
				break;

			case 4:
				r = (unsigned char)(t * 255);
				g = (unsigned char)(p * 255);
				b = (unsigned char)(v * 255);
				break;

			default:
				r = (unsigned char)(v * 255);
				g = (unsigned char)(p * 255);
				b = (unsigned char)(q * 255);
				break;
			}
		}

		out->state = GML_TYPE_DOUBLE;
		out->dVal = ((unsigned int)r) | (((unsigned int)g) << 8) | (((unsigned int)b) << 16);
	}
	return true;
}

bool CodeRunner::move_wrap(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[2].state == GML_TYPE_STRING) return false;
	bool hor = _isTrue(argv + 0);
	bool ver = _isTrue(argv + 1);
	double margin = argv[2].dVal;
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