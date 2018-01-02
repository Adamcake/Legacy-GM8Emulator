#include "CodeRunner.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "RNG.hpp"
#include "CodeActionManager.hpp"
#include "InputHandler.hpp"

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
	Instance* i = _instances->AddInstance(_nextInstanceID, argv[0].dVal, argv[1].dVal, objID);
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)_nextInstanceID;
	}
	Object* o = _assetManager->GetObject(objID);
	if (!_codeActions->Run(o->evCreate, o->evCreateActionCount, i, NULL)) return false;
	_nextInstanceID++;
	return true;
}

bool CodeRunner::instance_destroy(unsigned int argc, GMLType* argv, GMLType* out) {
	_contexts.top().self->exists = false;
	return true;
}

bool CodeRunner::irandom(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_STRING) return false;
	int rand = RNGIrandom(_round(argv[0].dVal));
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)rand;
	}
	return true;
}

bool CodeRunner::irandom_range(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_STRING || argv[1].state == GML_TYPE_STRING) return false;
	int rand = RNGIrandom(_round(argv[0].dVal));
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)rand;
	}
	return true;
}

bool CodeRunner::keyboard_check(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
	int gmlKeycode = (argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);

	if (gmlKeycode == 0) {
		// vk_nokey
		out->dVal = (InputCountKeys() == 0 ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 1) {
		// vk_anykey
		out->dVal = (InputCountKeys() == 0 ? 0.0 : 1.0);
	}
	else if (gmlKeycode == 39) {
		out->dVal = (InputCheckKey(262) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 37) {
		out->dVal = (InputCheckKey(263) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 40) {
		out->dVal = (InputCheckKey(264) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 38) {
		out->dVal = (InputCheckKey(265) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 13) {
		out->dVal = (InputCheckKey(257) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 27) {
		out->dVal = (InputCheckKey(256) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 32) {
		out->dVal = (InputCheckKey(32) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 16) {
		out->dVal = ((InputCheckKey(340) || InputCheckKey(344)) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 17) {
		out->dVal = ((InputCheckKey(341) && InputCheckKey(345)) ? 1.0 : 0.0);
	}
	else {
		out->dVal = (InputCheckKey(gmlKeycode) ? 1.0 : 0.0);
	}
	return true;
}

bool CodeRunner::keyboard_check_direct(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
	int gmlKeycode = (argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKeyDirect(gmlKeycode) ? 1.0 : 0.0);
	return true;
}

bool CodeRunner::keyboard_check_pressed(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
	int gmlKeycode = (argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);

	if (gmlKeycode == 0) {
		// vk_nokey
		out->dVal = (InputCountKeysPressed() == 0 ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 1) {
		// vk_anykey
		out->dVal = (InputCountKeysPressed() == 0 ? 0.0 : 1.0);
	}
	else if (gmlKeycode == 16) {
		out->dVal = ((InputCheckKeyPressed(340) || InputCheckKeyPressed(344)) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 17) {
		out->dVal = ((InputCheckKeyPressed(341) && InputCheckKeyPressed(345)) ? 1.0 : 0.0);
	}
	else {
		out->dVal = (InputCheckKeyPressed(gmlKeycode) ? 1.0 : 0.0);
	}
	return true;
}

bool CodeRunner::keyboard_check_released(unsigned int argc, GMLType* argv, GMLType* out) {
	(*out).state = GML_TYPE_DOUBLE;
	int gmlKeycode = (argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);

	if (gmlKeycode == 0) {
		// vk_nokey
		out->dVal = (InputCountKeysReleased() == 0 ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 1) {
		// vk_anykey
		out->dVal = (InputCountKeysReleased() == 0 ? 0.0 : 1.0);
	}
	else if (gmlKeycode == 16) {
		out->dVal = ((InputCheckKeyReleased(340) || InputCheckKeyReleased(344)) ? 1.0 : 0.0);
	}
	else if (gmlKeycode == 17) {
		out->dVal = ((InputCheckKeyReleased(341) && InputCheckKeyReleased(345)) ? 1.0 : 0.0);
	}
	else {
		out->dVal = (InputCheckKeyReleased(gmlKeycode) ? 1.0 : 0.0);
	}
	return true;
}

bool CodeRunner::make_color_hsv(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_STRING || argv[1].state == GML_TYPE_STRING || argv[2].state == GML_TYPE_STRING) return false;
	if (out) {
		float fH = (float)((argv[0].dVal / 255.0) * 360.0);
		float fS = (float)(argv[1].dVal / 255.0);
		float fV = (float)(argv[2].dVal / 255.0);
		float fR, fG, fB;
		float fC = fV * fS; // Chroma
		float fHPrime = (float)fmod(fH / 60.0, 6);
		float fX = fC * (float)(1 - fabs(fmod(fHPrime, 2) - 1));
		float fM = fV - fC;

		if (0 <= fHPrime && fHPrime < 1) {
			fR = fC;
			fG = fX;
			fB = 0;
		}
		else if (1 <= fHPrime && fHPrime < 2) {
			fR = fX;
			fG = fC;
			fB = 0;
		}
		else if (2 <= fHPrime && fHPrime < 3) {
			fR = 0;
			fG = fC;
			fB = fX;
		}
		else if (3 <= fHPrime && fHPrime < 4) {
			fR = 0;
			fG = fX;
			fB = fC;
		}
		else if (4 <= fHPrime && fHPrime < 5) {
			fR = fX;
			fG = 0;
			fB = fC;
		}
		else if (5 <= fHPrime && fHPrime < 6) {
			fR = fC;
			fG = 0;
			fB = fX;
		}
		else {
			fR = 0;
			fG = 0;
			fB = 0;
		}

		fR += fM;
		fG += fM;
		fB += fM;

		out->state = GML_TYPE_DOUBLE;
		out->dVal = ((unsigned int)(fR * 255)) | (((unsigned int)(fG * 255)) << 8) | (((unsigned int)(fB * 255)) << 16);
	}
	return true;
}

bool CodeRunner::move_wrap(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[2].state == GML_TYPE_STRING) return false;
	bool hor = _isTrue(argv + 0);
	bool ver = _isTrue(argv + 1);
	double margin = argv[2].dVal;
	Instance* instance = _contexts.top().self;

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
	if (argv[0].state == GML_TYPE_STRING) return false;
	double rand = RNGRandom(argv[0].dVal);
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = rand;
	}
	return true;
}

bool CodeRunner::random_range(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_STRING || argv[1].state == GML_TYPE_STRING) return false;
	double rand = RNGRandom(argv[1].dVal - argv[0].dVal);
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = rand + argv[0].dVal;
	}
	return true;
}

bool CodeRunner::random_get_seed(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)RNGGetSeed();
	}
	return true;
}

bool CodeRunner::random_set_seed(unsigned int argc, GMLType* argv, GMLType* out) {
	RNGSetSeed(argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);
	return true;
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