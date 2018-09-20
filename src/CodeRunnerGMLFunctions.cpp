#include <pch.h>
#include "CodeRunner.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "RNG.hpp"
#include "CodeActionManager.hpp"
#include "InputHandler.hpp"
#include "Collision.hpp"
#include "Renderer.hpp"
#include "File.hpp"

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

bool CodeRunner::abs(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = ::abs(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::choose(unsigned int argc, GMLType* argv, GMLType* out) {
	if (!argc) {
		if (out) {
			out->state = GMLTypeState::Double;
			out->dVal = GMLFalse;
		}
		return true;
	}

	int rand = RNGIrandom(argc);
	(*out) = argv[rand];
	return true;
}

bool CodeRunner::cos(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = ::cos(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::distance_to_object(unsigned int argc, GMLType* argv, GMLType* out) {
	Instance* self = _contexts.top().self;
	Instance* other;
	InstanceList::Iterator iter(_instances, _round(argv[0].dVal));
	double lowestDist = 1000000.0; // GML default
	RefreshInstanceBbox(self);

	while (other = iter.Next()) {
		RefreshInstanceBbox(other);

		int distanceAbove = other->bbox_top - self->bbox_bottom;
		int distanceBelow = self->bbox_top - other->bbox_bottom;
		unsigned int absHeightDiff = 0;
		if (distanceAbove > 0) absHeightDiff = distanceAbove;
		else if (distanceBelow > 0) absHeightDiff = distanceBelow;

		int distanceLeft = other->bbox_left - self->bbox_right;
		int distanceRight = self->bbox_left - other->bbox_right;
		unsigned int absSideDiff = 0;
		if (distanceLeft > 0) absSideDiff = distanceLeft;
		else if (distanceRight > 0) absSideDiff = distanceRight;

		double absDist = 0.0;
		if (absSideDiff || absHeightDiff) {
			absDist = ::sqrt((absSideDiff * absSideDiff) + (absHeightDiff * absHeightDiff));
			if (absDist < lowestDist) lowestDist = absDist;
		} else {
			lowestDist = 0.0;
			break;
		}
	}

	out->state = GMLTypeState::Double;
	out->dVal = lowestDist;
	
	return true;
}

bool CodeRunner::draw_rectangle(unsigned int argc, GMLType* argv, GMLType* out) {
	// todo
	return true;
}

bool CodeRunner::draw_set_alpha(unsigned int argc, GMLType* argv, GMLType* out) {
	_drawAlpha = argv[0].dVal;
	return true;
}

bool CodeRunner::draw_set_color(unsigned int argc, GMLType* argv, GMLType* out) {
	_drawColour = _round(argv[0].dVal);
	return true;
}

bool CodeRunner::draw_set_font(unsigned int argc, GMLType* argv, GMLType* out) {
	_drawFont = _round(argv[0].dVal);
	return true;
}

bool CodeRunner::draw_set_halign(unsigned int argc, GMLType* argv, GMLType* out) {
	_drawHalign = _round(argv[0].dVal);
	return true;
}

bool CodeRunner::draw_set_valign(unsigned int argc, GMLType* argv, GMLType* out) {
	_drawValign = _round(argv[0].dVal);
	return true;
}

bool CodeRunner::draw_sprite(unsigned int argc, GMLType* argv, GMLType* out) {
	Sprite* spr = AMGetSprite(_round(argv[0].dVal));
	RDrawImage(spr->frames[_round(argv[1].dVal) % spr->frameCount], argv[2].dVal, argv[3].dVal, 1.0, 1.0, 0, 0xFFFFFFFF, 1.0);
	return true;
}

bool CodeRunner::draw_sprite_ext(unsigned int argc, GMLType* argv, GMLType* out) {
	Sprite* spr = AMGetSprite(_round(argv[0].dVal));
	RDrawImage(spr->frames[_round(argv[1].dVal) % spr->frameCount], argv[2].dVal, argv[3].dVal, argv[4].dVal, argv[5].dVal, argv[6].dVal, _round(argv[7].dVal), argv[8].dVal);
	return true;
}

bool CodeRunner::draw_text(unsigned int argc, GMLType* argv, GMLType* out) {
	const char* str = argv[2].sVal.c_str();
	std::string st;
	if (argv[2].state == GMLTypeState::Double) {
		std::stringstream ss;
		ss.precision(_round(argv[2].dVal) == argv[2].dVal ? 0 : 2);
		ss << std::fixed << argv[2].dVal;
		st = ss.str();
		str = st.c_str();
	}

	Font* font = AMGetFont(_drawFont);
	if (font && font->exists) {
		int cursorX = _round(argv[0].dVal);
		int cursorY = _round(argv[1].dVal);

		unsigned tallest = 0;
		for (const char* pC = str; (*pC) != '\0'; pC++) {
			const char c = *pC;
			if (c == '#' && (pC == str || *(pC - 1) != '\\')) continue;
			unsigned int h = font->dmap[(c * 6) + 3];
			if (h > tallest) tallest = h;
		}

		if (_drawValign == 1 || _drawValign == 2) {
			unsigned int lineCount = 1;
			for (const char* pC = str; (*pC) != '\0'; pC++) {
				const char c = *pC;
				if (c == '#' && (pC == str || *(pC - 1) != '\\')) {
					lineCount++;
				}
			}
			lineCount = lineCount * tallest;
			if (_drawValign == 1) lineCount /= 2;
			cursorY -= lineCount;
		}

		bool recalcX = true;
		for (const char* pC = str; (*pC) != '\0'; pC++) {
			const char c = *pC;
			if (c == '#' && (pC == str || *(pC - 1) != '\\')) {
				recalcX = true;
				cursorY += tallest;
				continue;
			}
			else if (c == '\\' && *(pC + 1) == '#') {
				continue;
			}

			if (recalcX) {
				cursorX = _round(argv[0].dVal);
				if (_drawHalign == 1 || _drawHalign == 2) {
					unsigned int lineWidth = 0;
					for (const char* tC = pC; (*tC) != '\0'; tC++) {
						if ((*tC) == '#' && (tC == str || *(tC - 1) != '\\')) break;
						if ((*tC) == '\\' && *(tC + 1) == '#') continue;
						lineWidth += font->dmap[((*tC) * 6) + 4];
					}
					if (_drawHalign == 1) lineWidth /= 2;
					cursorX -= lineWidth;
				}
				recalcX = false;
			}

			if (font->rangeBegin <= (unsigned int)c && font->rangeEnd >= (unsigned int)c) {
				unsigned int* dmapPos = font->dmap + (c * 6);
				unsigned int cX = *(dmapPos);
				unsigned int cY = *(dmapPos + 1);
				unsigned int cW = *(dmapPos + 2);
				unsigned int cH = *(dmapPos + 3);
				unsigned int cCW = *(dmapPos + 4);
				unsigned int cCO = *(dmapPos + 5);

				RDrawPartialImage(font->image, cursorX + (int)cCO, cursorY, 1, 1, 0.0, _drawColour, _drawAlpha, cX, cY, cW, cH);
				cursorX += cCW;
			}
		}
	}
	else {
		// Should use the default font here
	}

	return true;
}

bool CodeRunner::event_inherited(unsigned int argc, GMLType* argv, GMLType* out) {
	Object* o = AMGetObject(_contexts.top().objId);
	unsigned int id;

	while (o->parentIndex >= 0) {
		id = o->parentIndex;
		o = AMGetObject(id);

		if (_codeActions->CheckObjectEvent(_contexts.top().eventId, _contexts.top().eventNumber, o)) {
			return _codeActions->RunInstanceEvent(_contexts.top().eventId, _contexts.top().eventNumber, _contexts.top().self, _contexts.top().other, id);
		}
	}
	return true;
}

bool CodeRunner::event_perform(unsigned int argc, GMLType* argv, GMLType* out) {
	return _codeActions->RunInstanceEvent(_round(argv[0].dVal), _round(argv[1].dVal), _contexts.top().self, _contexts.top().other, _contexts.top().self->object_index);
}


// --- FILE ---


bool CodeRunner::file_bin_open(unsigned int argc, GMLType* argv, GMLType* out) {
	if (!this->_assertArgs(argc, argv, 1, true, GMLTypeState::String)) return false;
	fs::path filePath = fs::path(argv[0].sVal);
	int fileType = _round(argv[1].dVal);

	if (fs::exists(filePath)) {
		// Stream mode
		int fileMode;
		switch (fileType) {
			case 0: // Read-only
				fileMode = std::fstream::in | std::fstream::binary; break;
			case 1: // Write-only
				fileMode = std::fstream::out | std::fstream::binary; break;
			case 2: default: // Read & write
				fileMode = std::fstream::in | std::fstream::out | std::fstream::binary; break;
		}

		int i = 0;
		for (; i < maxFilesOpen; i++) {
			if (!_userFiles[i].is_open()) {
				_userFiles[i].open(filePath, fileMode);
			}
		}

		if (i == maxFilesOpen) {
			return false;
		} else if (out) {
			out->state = GMLTypeState::Double;
			out->dVal = static_cast<double>(i + 1);
		}

		return true;
	} else {
		return false; // I guess?
	}
}

bool CodeRunner::file_bin_close(unsigned int argc, GMLType* argv, GMLType* out) {
	if (!this->_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
	int idx = _round(argv[0].dVal);
	if (_userFiles[idx].is_open()) {
		_userFiles[idx].close();
		return true;
	} else {
		return false;
	}
}

bool CodeRunner::file_bin_read_byte(unsigned int argc, GMLType* argv, GMLType* out) {
	if (!this->_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
	int idx = _round(argv[0].dVal);

	if (_userFiles[idx].is_open()) {
		if (!_userFiles[idx].eof()) {
			if (out) {
				out->state == GMLTypeState::Double;
				out->dVal = static_cast<double>(_userFiles[idx].get());
			}
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool CodeRunner::file_bin_write_byte(unsigned int argc, GMLType* argv, GMLType* out) {
	if (!this->_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
	int idx = _round(argv[0].dVal);
	int oByte = _round(argv[1].dVal);

	if (_userFiles[idx].is_open()) {
		_userFiles[idx].put(static_cast<char>(oByte));
		return true;
	} else {
		return false;
	}
}

bool CodeRunner::file_delete(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GMLTypeState::String) return false;
	return std::remove(argv[0].sVal.c_str()) == 0;
}

bool CodeRunner::file_exists(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GMLTypeState::String) return false;
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (fs::exists(fs::path(argv[0].sVal)) ? GMLTrue : GMLFalse);
	}
	return true;
}


// --- FILE END ---


bool CodeRunner::instance_create(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String || argv[1].state == GMLTypeState::String || argv[2].state == GMLTypeState::String) return false;
	unsigned int objID = _round(argv[2].dVal);
	Instance* i = _instances->AddInstance(_nextInstanceID, argv[0].dVal, argv[1].dVal, objID);
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (double)_nextInstanceID;
	}
	Object* o = AMGetObject(objID);
	if (!_codeActions->RunInstanceEvent(0, 0, i, NULL, i->object_index)) return false;
	_nextInstanceID++;
	return true;
}

bool CodeRunner::instance_destroy(unsigned int argc, GMLType* argv, GMLType* out) {
	_contexts.top().self->exists = false;
	return true;
}

bool CodeRunner::instance_exists(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::Double) return false;
	int objId = _round(argv[0].dVal);
	InstanceList::Iterator it(_instances, (unsigned int)objId);
	out->state = GMLTypeState::Double;
	out->dVal = (it.Next() ? GMLTrue : GMLFalse);
	return true;
}

bool CodeRunner::instance_number(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::Double) return false;
	int objId = _round(argv[0].dVal);
	InstanceList::Iterator it(_instances, (unsigned int)objId);
	out->state = GMLTypeState::Double;
	unsigned int count = 0;
	while (it.Next()) count++;
	out->dVal = (double)count;
	return true;
}

bool CodeRunner::instance_position(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		int objId = _round(argv[2].dVal);
		int x = _round(argv[0].dVal);
		int y = _round(argv[1].dVal);
		InstanceList::Iterator it(_instances, (unsigned int)objId);
		if (objId == -3) it = InstanceList::Iterator(_instances);
		Instance* instance;
		double ret = -4.0;
		while (instance = it.Next()) {
			if (CollisionPointCheck(instance, x, y)) {
				ret = instance->id;
				break;
			}
		}
		out->state = GMLTypeState::Double;
		out->dVal = ret;
	}
	return true;
}

bool CodeRunner::irandom(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String) return false;
	int rand = RNGIrandom(_round(argv[0].dVal));
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (double)rand;
	}
	return true;
}

bool CodeRunner::irandom_range(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String || argv[1].state == GMLTypeState::String) return false;
	int rand = RNGIrandom(::abs(_round(argv[1].dVal) - _round(argv[0].dVal)) + 1);
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (double)rand + std::fmin(argv[0].dVal, argv[1].dVal);
	}
	return true;
}

bool CodeRunner::floor(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = ::floor(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::game_end(unsigned int argc, GMLType* argv, GMLType* out) {
	return false;
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
	out->state = GMLTypeState::Double;
	int gmlKeycode = (argv[0].state == GMLTypeState::Double ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKey(gmlKeycode) ? GMLTrue : GMLFalse);
	return true;
}

bool CodeRunner::keyboard_check_direct(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GMLTypeState::Double;
	int gmlKeycode = (argv[0].state == GMLTypeState::Double ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKeyDirect(gmlKeycode) ? GMLTrue : GMLFalse);
	return true;
}

bool CodeRunner::keyboard_check_pressed(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GMLTypeState::Double;
	int gmlKeycode = (argv[0].state == GMLTypeState::Double ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKeyPressed(gmlKeycode) ? GMLTrue : GMLFalse);
	return true;
}

bool CodeRunner::keyboard_check_released(unsigned int argc, GMLType* argv, GMLType* out) {
	out->state = GMLTypeState::Double;
	int gmlKeycode = (argv[0].state == GMLTypeState::Double ? _round(argv[0].dVal) : 0);
	out->dVal = (InputCheckKeyReleased(gmlKeycode) ? GMLTrue : GMLFalse);
	return true;
}

bool CodeRunner::make_color_hsv(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String || argv[1].state == GMLTypeState::String || argv[2].state == GMLTypeState::String) return false;
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

		out->state = GMLTypeState::Double;
		out->dVal = ((unsigned int)(fR * 255)) | (((unsigned int)(fG * 255)) << 8) | (((unsigned int)(fB * 255)) << 16);
	}
	return true;
}

bool CodeRunner::max(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		GMLType ret;
		if (argc == 0) {
			(*out) = ret;
			return true;
		}

		ret = *argv;
		for (GMLType* arg = argv + 1; arg < (argv + argc); arg++) {
			if (arg->state == GMLTypeState::String && ret.state == GMLTypeState::String) {
				unsigned int c = 0;
				while (true) {
					if ((*(arg->sVal.c_str() + c)) > (*(ret.sVal.c_str() + c))) {
						ret = *arg;
						break;
					}
					if ((*(arg->sVal.c_str() + c)) == '\0') {
						break;
					}
					c++;
				}
			}
			else if (arg->state == GMLTypeState::Double && ret.state == GMLTypeState::Double) {
				if (arg->dVal > ret.dVal) ret = *arg;
			}
		}
		(*out) = ret;
	}
	return true;
}

bool CodeRunner::min(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		GMLType ret;
		if (argc == 0) {
			(*out) = ret;
			return true;
		}

		ret = *argv;
		for (GMLType* arg = argv + 1; arg < (argv + argc); arg++) {
			if (arg->state == GMLTypeState::String && ret.state == GMLTypeState::String) {
				unsigned int c = 0;
				while (true) {
					if ((*(arg->sVal.c_str() + c)) < (*(ret.sVal.c_str() + c))) {
						ret = *arg;
						break;
					}
					if ((*(ret.sVal.c_str() + c)) == '\0') {
						break;
					}
					c++;
				}
			}
			else if (arg->state == GMLTypeState::Double && ret.state == GMLTypeState::Double) {
				if (arg->dVal < ret.dVal) ret = *arg;
			}
			else {
				ret = *arg;
			}
		}
		(*out) = ret;
	}
	return true;
}

bool CodeRunner::move_contact_solid(unsigned int argc, GMLType* argv, GMLType* out) {
	int maxdist = _round(argv[1].dVal);
	if (maxdist <= 0) maxdist = 1000; // GML default
	double hspeed = ::cos(argv[0].dVal * PI / 180.0);
	double vspeed = -::sin(argv[0].dVal * PI / 180.0);
	Instance* self = _contexts.top().self;
	bool moved = false;

	for (int i = 0; i < maxdist; i++) {
		InstanceList::Iterator iter(_instances);
		bool collision = false;
		
		Instance* target;
		while (target = iter.Next()) {
			if ((target != self) && target->solid) {
				if (CollisionCheck(self, target)) {
					collision = true;
					break;
				}
			}
		}

		if (collision) {
			if (moved) {
				self->x -= hspeed;
				self->y -= vspeed;
			}
			break;
		}
		else {
			self->x += hspeed;
			self->y += vspeed;
			self->bboxIsStale = true;
			moved = true;
		}
	}
	return true;
}

bool CodeRunner::move_wrap(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[2].state == GMLTypeState::String) return false;
	bool hor = _isTrue(argv + 0);
	bool ver = _isTrue(argv + 1);
	double margin = argv[2].dVal;
	Instance* instance = _contexts.top().self;

	if (hor) {
		unsigned int roomW = AMGetRoom(_globalValues->room)->width;
		if (instance->x < -margin) {
			instance->x += roomW;
		}
		else if (instance->x >= (roomW + margin)) {
			instance->x -= roomW;
		}
	}

	if (ver) {
		unsigned int roomH = AMGetRoom(_globalValues->room)->height;
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
	if (argv[0].state != GMLTypeState::String) return false;
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (double)argv[0].sVal[0];
	}
	return true;
}

bool CodeRunner::place_free(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = GMLTrue;

		InstanceList::Iterator iter(_instances);
		Instance* self = _contexts.top().self;
		double oldX = self->x;
		double oldY = self->y;
		self->x = argv[0].dVal;
		self->y = argv[1].dVal;
		self->bboxIsStale = true;

		Instance* target;
		while (target = iter.Next()) {
			if ((target != self) && target->solid) {
				if (CollisionCheck(self, target)) {
					out->dVal = GMLFalse;
					break;
				}
			}
		}

		self->x = oldX;
		self->y = oldY;
		self->bboxIsStale = true;
	}
	return true;
}

bool CodeRunner::place_meeting(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = GMLFalse;
		int obj = _round(argv[2].dVal);
		InstanceList::Iterator iter(_instances, (unsigned int)obj);
		if (obj == -3) iter = InstanceList::Iterator(_instances);

		Instance* self = _contexts.top().self;
		double oldX = self->x;
		double oldY = self->y;
		self->x = argv[0].dVal;
		self->y = argv[1].dVal;
		self->bboxIsStale = true;

		Instance* target;
		while (target = iter.Next()) {
			if (target != self) {
				if (CollisionCheck(self, target)) {
					out->dVal = GMLTrue;
					break;
				}
			}
		}

		self->x = oldX;
		self->y = oldY;
		self->bboxIsStale = true;
	}
	return true;
}

bool CodeRunner::point_direction(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (::atan2((argv[1].dVal - argv[3].dVal), (argv[2].dVal - argv[0].dVal))) * 180.0 / PI;
	}
	return true;
}

bool CodeRunner::power(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		(*out).state = GMLTypeState::Double;
		(*out).dVal = ::pow(argv[0].dVal, argv[1].dVal);
	}
	return true;
}

bool CodeRunner::random(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String) return false;
	double rand = RNGRandom(argv[0].dVal);
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = rand;
	}
	return true;
}

bool CodeRunner::random_range(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String || argv[1].state == GMLTypeState::String) return false;
	double rand = RNGRandom(argv[1].dVal - argv[0].dVal);
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = rand + argv[0].dVal;
	}
	return true;
}

bool CodeRunner::random_get_seed(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (double)RNGGetSeed();
	}
	return true;
}

bool CodeRunner::random_set_seed(unsigned int argc, GMLType* argv, GMLType* out) {
	RNGSetSeed(argv[0].state == GMLTypeState::Double ? _round(argv[0].dVal) : 0);
	return true;
}

bool CodeRunner::room_goto(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state == GMLTypeState::String) return false;
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

bool CodeRunner::sign(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (argv[0].dVal == 0 ? 0 : (argv[0].dVal < 0 ? -1 : 1));
	}
	return true;
}

bool CodeRunner::sin(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = ::sin(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::sqr(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = argv[0].dVal * argv[0].dVal;
	}
	return true;
}

bool CodeRunner::sqrt(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].dVal < 0) return false;
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = ::sqrt(argv[0].dVal);
	}
	return true;
}

bool CodeRunner::string(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::String;
		if (argv[0].state == GMLTypeState::String) {
			out->sVal = argv[0].sVal;
		}
		else {
			std::stringstream ss;
			ss.precision(_round(argv[0].dVal) == argv[0].dVal ? 0 : 2);
			ss << std::fixed << argv[0].dVal;
			out->sVal = ss.str();
			//const char* c = ss.str().c_str();
			//size_t len = ss.str().size();
			//out->sVal = _constants[_RegConstantString(c, (unsigned int)len)].sVal;
		}
	}
	return true;
}

bool CodeRunner::string_width(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		if (argv[0].state != GMLTypeState::String) {
			out->dVal = GMLTrue; // GML default
			return true;
		}
		
		Font* font = AMGetFont(_drawFont);
		if (!font->exists) {
			// Default font not sure what to do here
			out->dVal = GMLFalse;
			return true;
		}

		unsigned int longestLine = 1;
		unsigned int curLength = 0;
		for (const char* pC = argv[0].sVal.c_str(); (*pC) != '\0'; pC++) {
			const char c = *pC;
			if (c == '#' && (pC == argv[0].sVal || *(pC - 1) != '\\')) {
				if (curLength > longestLine) longestLine = curLength;
				curLength = 0;
				continue;
			}
			curLength += font->dmap[(c * 6) + 4];
		}

		if (curLength > longestLine) longestLine = curLength;
		out->dVal = longestLine;
	}
	return true;
}

bool CodeRunner::string_height(unsigned int argc, GMLType* argv, GMLType* out) {
	if (out) {
		out->state = GMLTypeState::Double;
		if (argv[0].state != GMLTypeState::String) {
			out->dVal = GMLTrue; // GML default
			return true;
		}

		Font* font = AMGetFont(_drawFont);
		if (!font->exists) {
			// Default font not sure what to do here
			out->dVal = GMLFalse;
			return true;
		}
		
		unsigned tallest = 1;
		unsigned int lines = 1;
		for (const char* pC = argv[0].sVal.c_str(); (*pC) != '\0'; pC++) {
			const char c = *pC;
			if (c == '#' && (pC == argv[0].sVal || *(pC - 1) != '\\')) {
				lines++;
				continue;
			}
			unsigned int h = font->dmap[(c * 6) + 3];
			if (h > tallest) tallest = h;
		}

		out->dVal = tallest * lines;
	}
	return true;
}

bool CodeRunner::unimplemented(unsigned int argc, GMLType * argv, GMLType * out) {
	if (!CRErrorOnUnimplemented) {
		if (out) {
			out->state = GMLTypeState::Double;
			out->dVal = GMLFalse;
		}
		return true;
	}
	return false;
}
