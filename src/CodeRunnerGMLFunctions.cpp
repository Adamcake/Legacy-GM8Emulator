#include "CodeRunner.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "RNG.hpp"
#include "CodeActionManager.hpp"
#include "InputHandler.hpp"
#include "Collision.hpp"

#include <iomanip> // for string()
#include <sstream> // for string()

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

bool CodeRunner::cos(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = ::cos(argv[0].dVal);
	}
	return true;
}

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

bool CodeRunner::instance_exists(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_DOUBLE) return false;
	int objId = _round(argv[0].dVal);
	InstanceList::Iterator it(_instances, (unsigned int)objId);
	out->state = GML_TYPE_DOUBLE;
	out->dVal = (it.Next() ? 1.0 : 0.0);
	return true;
}

bool CodeRunner::instance_number(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GML_TYPE_DOUBLE) return false;
	int objId = _round(argv[0].dVal);
	InstanceList::Iterator it(_instances, (unsigned int)objId);
	out->state = GML_TYPE_DOUBLE;
	unsigned int count = 0;
	while (it.Next()) count++;
	out->dVal = (double)count;
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

bool CodeRunner::file_bin_open(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GML_TYPE_STRING) return false;
	int ftype = _round(argv[1].dVal);
	FILE* f;

	switch (ftype) {
		case 0:
			if (!fopen_s(&f, argv[0].sVal, "rb")) {
				fopen_s(&f, argv[0].sVal, "wb");
				fclose(f);
				fopen_s(&f, argv[0].sVal, "rb");
			}
			break;
		case 1:
			fopen_s(&f, argv[0].sVal, "wb");
			break;
		default:
			fopen_s(&f, argv[0].sVal, "ab"); // Not sure if "ab" is correct for this setting, but I assume so.
			break;
	}

	int i = 0;
	for (i = 0; i < 32; i++) {
		if (!_userFiles[i]) {
			_userFiles[i] = f;
		}
	}

	if (i == 32) return false;
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)(i + 1);
	}

	return true;
}

bool CodeRunner::file_bin_close(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GML_TYPE_DOUBLE) return false;
	int index = _round(argv[0].dVal) - 1;
	if (index < 0 || index >= 32) return false;
	if (!_userFiles[index]) return false;

	fclose(_userFiles[index]);
	_userFiles[index] = NULL;
	return true;
}

bool CodeRunner::file_bin_read_byte(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GML_TYPE_DOUBLE) return false;
	int index = _round(argv[0].dVal) - 1;
	if (index < 0 || index >= 32) return false;
	if (!_userFiles[index]) return false;

	unsigned char s;
	fread_s(&s, 1, 1, 1, _userFiles[index]);
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)s;
	}
	return true;
}

bool CodeRunner::file_bin_write_byte(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GML_TYPE_DOUBLE || argv[1].state != GML_TYPE_DOUBLE) return false;
	int index = _round(argv[0].dVal) - 1;
	if (index < 0 || index >= 32) return false;
	if (!_userFiles[index]) return false;

	unsigned char c = _round(argv[1].dVal);
	fwrite(&c, 1, 1, _userFiles[index]);
	return true;
}

bool CodeRunner::file_exists(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GML_TYPE_STRING) return false;
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (stat(argv[0].sVal, NULL) == 0 ? 1.0 : 0.0);
	}
	return true;
}

bool CodeRunner::floor(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = ::floor(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::game_restart(unsigned int argc, GMLType* argv, GMLType* out) {
	_globalValues->changeRoom = true;
	_globalValues->roomTarget = (*_roomOrder)[0];
	InstanceList::Iterator iter(_instances);
	Instance* i;
	while (i = iter.Next()) {
		i->exists = false;
	}
	return true;
}

bool CodeRunner::keyboard_check(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
	int gmlKeycode = (argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKey(gmlKeycode) ? 1.0 : 0.0);
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
	out->dVal = (InputCheckKeyPressed(gmlKeycode) ? 1.0 : 0.0);
	return true;
}

bool CodeRunner::keyboard_check_released(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
	int gmlKeycode = (argv[0].state == GML_TYPE_DOUBLE ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKeyReleased(gmlKeycode) ? 1.0 : 0.0);
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

bool CodeRunner::ord(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GML_TYPE_STRING) return false;
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (double)argv[0].sVal[0];
	}
	return true;
}

bool CodeRunner::place_meeting(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = 0.0;
		int obj = _round(argv[2].dVal);
		InstanceList::Iterator iter(_instances, (unsigned int)obj);
		if (obj == -3) iter = InstanceList::Iterator(_instances);

		Instance* self = _contexts.top().self;
		double oldX = self->x;
		double oldY = self->y;
		self->x = argv[0].dVal;
		self->y = argv[1].dVal;

		Instance* target;
		while (target = iter.Next()) {
			if (target != self) {
				if (CollisionCheck(self, target, _assetManager)) {
					out->dVal = 1.0;
					break;
				}
			}
		}

		self->x = oldX;
		self->y = oldY;
	}
	return true;
}

bool CodeRunner::point_direction(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = (::atan2((argv[1].dVal - argv[3].dVal), (argv[2].dVal - argv[0].dVal))) * 180.0 / PI;
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
	if (argv[0].state == GML_TYPE_STRING) return false;
	_globalValues->changeRoom = true;
	_globalValues->roomTarget = _round(argv[0].dVal);

	return true;
}

bool CodeRunner::room_goto_next(unsigned int argc, GMLType* argv, GMLType* out) {
	_globalValues->changeRoom = true;
	if ((*_roomOrder)[_roomOrderCount - 1] == _globalValues->room) return false; // Trying to go to next room from last room
	for (unsigned int i = 0; i < _roomOrderCount; i++) {
		if ((*_roomOrder)[i] == _globalValues->room) {
			_globalValues->roomTarget = (*_roomOrder)[i + 1];
			return true;
		}
	}
	return false;
}

bool CodeRunner::room_goto_previous(unsigned int argc, GMLType* argv, GMLType* out) {
	_globalValues->changeRoom = true;
	if ((*_roomOrder)[0] == _globalValues->room) return false; // Trying to go to next room from last room
	for (unsigned int i = 0; i < _roomOrderCount; i++) {
		if ((*_roomOrder)[i] == _globalValues->room) {
			_globalValues->roomTarget = (*_roomOrder)[i - 1];
			return true;
		}
	}
	return false;
}

bool CodeRunner::sin(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_DOUBLE;
		out->dVal = ::sin(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::sound_play(unsigned int argc, GMLType* argv, GMLType* out) {
	// tbd
	return true;
}

bool CodeRunner::string(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GML_TYPE_STRING;
		if (argv[0].state == GML_TYPE_STRING) {
			out->sVal = argv[0].sVal;
		}
		else {
			std::stringstream s;
			s << std::fixed << std::setprecision(2) << argv[0].dVal;
			const char* c = s.str().c_str();
			size_t len = s.str().size();
			out->sVal = (char*)malloc(len + 1);
			memcpy(out->sVal, c, len + 1);
		}
	}
	return true;
}