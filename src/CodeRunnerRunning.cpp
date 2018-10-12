#include <pch.h>
#include "CodeRunner.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "Renderer.hpp"
#include "GlobalValues.hpp"
#include "Collision.hpp"
#include "Alarm.hpp"

constexpr unsigned int ARG_STACK_SIZE = 4; // SHOUTY


Instance global;


int CodeRunner::_round(double d) {
	// This mimics the x86_32 "FISTP" operator which is commonly used in the GM8 runner.
	// We can't actually use that operator, because we're targeting other platforms than x86 Windows.
	int down = (int)d;
	if ((d - down) < 0.5) return down;
	if ((d - down) > 0.5) return (down + 1);
	return down + (down & 1);
}

bool CodeRunner::_equal(double d1, double d2) {
	// I have no idea why GM8 does this, but it does.
	double difference = fabs(d2 - d1);
	double cut_digits = ::floor(difference * 1e13) / 1e13;
	return cut_digits == 0.0;
}

bool CodeRunner::_parseVal(const unsigned char* val, CodeRunner::GMLType* out) {
	unsigned char type = val[0] >> 6;
	unsigned int index;
	if(type) index = ((val[0] & 0x3F) << 16) + (val[1] << 8) + val[2]; // Index is discarded if type is 0

	switch (type) {
		case 0:
			out->state = GMLTypeState::Double;
			out->dVal = (double)_stack.top();
			break;
		case 1:
			out->state = GMLTypeState::Double;
			out->dVal = (double)index;
			break;
		case 2:
			(*out) = _constants[index];
			break;
		case 3:
			if (!_evalExpression(_codeObjects[index].compiled, out)) return false;
			break;
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_setGameValue(CRGameVar index, const unsigned char* arrayIndexVal, CRSetMethod method, GMLType value) {
	switch (index) {
		case ROOM:
			_globalValues->roomTarget = (unsigned int)_round(value.dVal);
			break;
		case ROOM_SPEED:
			_globalValues->room_speed = (unsigned int)_round(value.dVal);
			break;
		case ROOM_CAPTION: {
			GMLType tCaption{ GMLTypeState::String, 0.0, _globalValues->room_caption };
			_applySetMethod(&tCaption, method, &value);
			_globalValues->room_caption = tCaption.sVal;
		}
			break;
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_getGameValue(CRGameVar index, const unsigned char* arrayIndexVal, GMLType* out) {
	out->state = GMLTypeState::Double;
	GMLType arr;
	int aIndex;
	switch (index) {
		case ARGUMENT:
			if (!_parseVal(arrayIndexVal, &arr)) return false;
			aIndex = _round(arr.dVal);
			if (_contexts.top().argc > aIndex) {
				(*out) = _contexts.top().argv[aIndex];
			}
			break;
		case ARGUMENT0:
			if (_contexts.top().argc > 0) {
				(*out) = _contexts.top().argv[0];
			}
			break;
		case ARGUMENT1:
			if (_contexts.top().argc > 1) {
				(*out) = _contexts.top().argv[1];
			}
			break;
		case ARGUMENT2:
			if (_contexts.top().argc > 2) {
				(*out) = _contexts.top().argv[2];
			}
			break;
		case ARGUMENT3:
			if (_contexts.top().argc > 3) {
				(*out) = _contexts.top().argv[3];
			}
			break;
		case ARGUMENT4:
			if (_contexts.top().argc > 4) {
				(*out) = _contexts.top().argv[4];
			}
			break;
		case ARGUMENT5:
			if (_contexts.top().argc > 5) {
				(*out) = _contexts.top().argv[5];
			}
			break;
		case ARGUMENT6:
			if (_contexts.top().argc > 6) {
				(*out) = _contexts.top().argv[6];
			}
			break;
		case ARGUMENT7:
			if (_contexts.top().argc > 7) {
				(*out) = _contexts.top().argv[7];
			}
			break;
		case ARGUMENT8:
			if (_contexts.top().argc > 8) {
				(*out) = _contexts.top().argv[8];
			}
			break;
		case ARGUMENT9:
			if (_contexts.top().argc > 9) {
				(*out) = _contexts.top().argv[9];
			}
			break;
		case ARGUMENT10:
			if (_contexts.top().argc > 10) {
				(*out) = _contexts.top().argv[10];
			}
			break;
		case ARGUMENT11:
			if (_contexts.top().argc > 11) {
				(*out) = _contexts.top().argv[11];
			}
			break;
		case ARGUMENT12:
			if (_contexts.top().argc > 12) {
				(*out) = _contexts.top().argv[12];
			}
			break;
		case ARGUMENT13:
			if (_contexts.top().argc > 13) {
				(*out) = _contexts.top().argv[13];
			}
			break;
		case ARGUMENT14:
			if (_contexts.top().argc > 14) {
				(*out) = _contexts.top().argv[14];
			}
			break;
		case ARGUMENT15:
			if (_contexts.top().argc > 15) {
				(*out) = _contexts.top().argv[15];
			}
			break;
		case INSTANCE_COUNT:
			out->dVal = (double)_instances->Count();
			break;
		case MOUSE_X:
			int mx;
			RGetCursorPos(&mx, NULL);
			out->dVal = (double)mx;
			break;
		case MOUSE_Y:
			int my;
			RGetCursorPos(NULL, &my);
			out->dVal = (double)my;
			break;
		case ROOM:
			out->dVal = (double)_globalValues->room;
			break;
		case ROOM_SPEED:
			out->dVal = (double)_globalValues->room_speed;
			break;
		case ROOM_WIDTH:
			out->dVal = (double)_globalValues->room_width;
			break;
		case ROOM_HEIGHT:
			out->dVal = (double)_globalValues->room_height;
			break;
		default:
			return false;
	}
	return true;
}


bool CodeRunner::_setInstanceVar(Instance* instance, CRInstanceVar index, const unsigned char * arrayIndexVal, CRSetMethod method, GMLType value) {
	// No instance vars are strings. In GML if you set an instance var to a string, it gets set to 0.
	GMLType t;
	t.state = GMLTypeState::Double;
	if (value.state != GMLTypeState::Double) {
		value.state = GMLTypeState::Double;
		value.dVal = 0;
	}

	GMLType arrayId;
	switch (index) {
		case IV_ALARM: {
			if (!_parseVal(arrayIndexVal, &arrayId)) return false;
			if (arrayId.state != GMLTypeState::Double) return false;
			int roundedAId = _round(arrayId.dVal);
			if (roundedAId < 0) return false;
			int alarmValue = (int)(value.state == GMLTypeState::Double ? value.dVal : 0.0);
			if (alarmValue) AlarmSet(instance->id, (unsigned int)roundedAId, alarmValue);
			else AlarmDelete(instance->id, (unsigned int)roundedAId);
			break;
		}
		case IV_DIRECTION:
			t.dVal = instance->direction;
			if (!_applySetMethod(&t, method, &value)) return false;
			while (t.dVal >= 360.0) t.dVal -= 360.0;
			while (t.dVal < 0.0) t.dVal += 360.0;
			instance->direction = (t.dVal);
			instance->hspeed = ::cos(instance->direction * PI / 180.0) * instance->speed;
			instance->vspeed = -::sin(instance->direction * PI / 180.0) * instance->speed;
			break;
		case IV_IMAGE_SPEED:
			t.dVal = instance->image_speed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_speed = t.dVal;
			break;
		case IV_FRICTION:
			t.dVal = instance->friction;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->friction = t.dVal;
			break;
		case IV_SPRITE_INDEX:
			t.dVal = instance->sprite_index;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->sprite_index = _round(t.dVal);
			break;
		case IV_MASK_INDEX:
			t.dVal = instance->mask_index;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->mask_index = _round(t.dVal);
			break;
		case IV_IMAGE_BLEND:
			t.dVal = instance->image_blend;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_blend = _round(t.dVal);
			break;
		case IV_IMAGE_ALPHA:
			t.dVal = instance->image_alpha;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_alpha = t.dVal;
			break;
		case IV_IMAGE_INDEX:
			t.dVal = instance->image_index;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_index = t.dVal;
			break;
		case IV_IMAGE_ANGLE:
			t.dVal = instance->image_angle;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_angle = t.dVal;
			instance->bboxIsStale = true;
			break;
		case IV_IMAGE_XSCALE:
			t.dVal = instance->image_xscale;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_xscale = t.dVal;
			instance->bboxIsStale = true;
			break;
		case IV_IMAGE_YSCALE:
			t.dVal = instance->image_yscale;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_yscale = t.dVal;
			instance->bboxIsStale = true;
			break;
		case IV_SOLID:
			t.dVal = (instance->solid ? GMLTrue : GMLFalse);
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->solid = _isTrue(&t);
			break;
		case IV_VISIBLE:
			t.dVal = (instance->visible ? GMLTrue : GMLFalse);
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->visible = _isTrue(&t);
			break;
		case IV_PERSISTENT:
			t.dVal = (instance->persistent ? GMLTrue : GMLFalse);
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->persistent = _isTrue(&t);
			break;
		case IV_DEPTH:
			t.dVal = instance->depth;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->depth = _round(t.dVal);
			break;
		case IV_SPEED:
			t.dVal = instance->speed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->speed = t.dVal;
			instance->hspeed = ::cos(instance->direction * PI / 180.0) * instance->speed;
			instance->vspeed = -::sin(instance->direction * PI / 180.0) * instance->speed;
			break;
		case IV_VSPEED:
			t.dVal = instance->vspeed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->vspeed = t.dVal;
			instance->direction = ::atan(-instance->vspeed / instance->hspeed) * 180.0 / PI;
			instance->speed = ::sqrt(pow(instance->hspeed, 2) + pow(instance->vspeed, 2));
			break;
		case IV_HSPEED:
			t.dVal = instance->hspeed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->hspeed = t.dVal;
			instance->direction = ::atan(-instance->vspeed / instance->hspeed) * 180.0 / PI;
			instance->speed = ::sqrt(pow(instance->hspeed, 2) + pow(instance->vspeed, 2));
			break;
		case IV_GRAVITY:
			t.dVal = instance->gravity;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->gravity = t.dVal;
			break;
		case IV_GRAVITY_DIRECTION:
			t.dVal = instance->gravity_direction;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->gravity_direction = t.dVal;
			break;
		case IV_X:
			t.dVal = instance->x;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->x = t.dVal;
			instance->bboxIsStale = true;
			break;
		case IV_Y:
			t.dVal = instance->y;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->y = t.dVal;
			instance->bboxIsStale = true;
			break;
		case IV_PATH_INDEX:
			t.dVal = instance->path_index;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->path_index = _round(t.dVal);
			break;
		case IV_PATH_POSITION:
			t.dVal = instance->path_position;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->path_position = t.dVal;
			break;
		case IV_PATH_SPEED:
			t.dVal = instance->path_speed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->path_speed = t.dVal;
			break;
		case IV_PATH_SCALE:
			t.dVal = instance->path_scale;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->path_scale = t.dVal;
			break;
		case IV_PATH_ORIENTATION:
			t.dVal = instance->path_orientation;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->path_orientation = t.dVal;
			break;
		case IV_PATH_ENDACTION:
			t.dVal = instance->path_endaction;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->path_endaction = _round(t.dVal);
			break;
		case IV_TIMELINE_INDEX:
			t.dVal = instance->timeline_index;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->timeline_index = _round(t.dVal);
			break;
		case IV_TIMELINE_RUNNING:
			t.dVal = (instance->timeline_running ? GMLTrue : GMLFalse);
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->timeline_running = _isTrue(&t);
			break;
		case IV_TIMELINE_LOOP:
			t.dVal = (instance->timeline_loop ? GMLTrue : GMLFalse);
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->timeline_loop = _isTrue(&t);
			break;
		case IV_TIMELINE_SPEED:
			t.dVal = instance->timeline_speed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->timeline_speed = t.dVal;
			break;
		case IV_TIMELINE_POSITION:
			t.dVal = instance->timeline_position;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->timeline_position = t.dVal;
			break;
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_getInstanceVar(Instance* instance, CRInstanceVar index, const unsigned char* arrayIndexVal, GMLType* out) {
	out->state = GMLTypeState::Double;
	switch (index) {
		case IV_INSTANCE_ID:
			out->dVal = instance->id;
			break;
		case IV_X:
			out->dVal = instance->x;
			break;
		case IV_Y:
			out->dVal = instance->y;
			break;
		case IV_SOLID:
			out->dVal = (instance->solid ? GMLTrue : GMLFalse);
			break;
		case IV_DIRECTION:
			out->dVal = instance->direction;
			break;
		case IV_SPEED:
			out->dVal = instance->speed;
			break;
		case IV_VSPEED:
			out->dVal = instance->vspeed;
			break;
		case IV_HSPEED:
			out->dVal = instance->hspeed;
			break;
		case IV_GRAVITY:
			out->dVal = instance->gravity;
			break;
		case IV_GRAVITY_DIRECTION:
			out->dVal = instance->gravity_direction;
			break;
		case IV_IMAGE_ALPHA:
			out->dVal = instance->image_alpha;
			break;
		case IV_IMAGE_INDEX:
			out->dVal = instance->image_index;
			break;
		case IV_SPRITE_INDEX:
			out->dVal = instance->sprite_index;
			break;
		case IV_SPRITE_WIDTH:
			if (instance->sprite_index < 0 || instance->sprite_index >= (int)AMGetSpriteCount()) {
				out->dVal = 0;
			}
			else {
				Sprite* s = AMGetSprite(instance->sprite_index);
				out->dVal = (s->exists ? s->width : 0);
			}
			break;
		case IV_SPRITE_HEIGHT:
			if (instance->sprite_index < 0 || instance->sprite_index >= (int)AMGetSpriteCount()) {
				out->dVal = 0;
			}
			else {
				Sprite* s = AMGetSprite(instance->sprite_index);
				out->dVal = (s->exists ? s->height : 0);
			}
			break;
		case IV_IMAGE_XSCALE:
			out->dVal = instance->image_xscale;
			break;
		case IV_IMAGE_YSCALE:
			out->dVal = instance->image_yscale;
			break;
		case IV_BBOX_LEFT:
			RefreshInstanceBbox(instance);
			out->dVal = instance->bbox_left;
			break;
		case IV_BBOX_RIGHT:
			RefreshInstanceBbox(instance);
			out->dVal = instance->bbox_right;
			break;
		case IV_BBOX_BOTTOM:
			RefreshInstanceBbox(instance);
			out->dVal = instance->bbox_bottom;
			break;
		case IV_BBOX_TOP:
			RefreshInstanceBbox(instance);
			out->dVal = instance->bbox_top;
			break;
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_isTrue(const CodeRunner::GMLType* value) {
	return (value->state == GMLTypeState::Double) && (value->dVal >= 0.5);
}

bool CodeRunner::_applySetMethod(CodeRunner::GMLType* lhs, CRSetMethod method, const CodeRunner::GMLType* const rhs) {
	if (method == SM_ASSIGN) {
		// Easiest method
		(*lhs) = (*rhs);
		return true;
	}
	else if (method == SM_ADD) {
		// Only other method that can be used on strings
		if ((lhs->state == GMLTypeState::String) != (rhs->state == GMLTypeState::String)) {
			// Incompatible operands
			return false;
		}
		if (lhs->state == GMLTypeState::String) {
			lhs->sVal += rhs->sVal;
		}
		else {
			lhs->dVal += rhs->dVal;
		}
		return true;
	}
	else {
		// No other set methods can be used with strings, so we can error if either one is a string
		if ((lhs->state == GMLTypeState::String) || (rhs->state == GMLTypeState::String)) {
			return false;
		}
		switch (method) {
			case SM_SUBTRACT:
				lhs->dVal -= rhs->dVal;
				break;
			case SM_MULTIPLY:
				lhs->dVal *= rhs->dVal;
				break;
			case SM_DIVIDE:
				lhs->dVal /= rhs->dVal;
				break;
			case SM_BITWISE_AND:
				lhs->dVal = (double)(_round(lhs->dVal) & _round(rhs->dVal));
				break;
			case SM_BITWISE_OR:
				lhs->dVal = (double)(_round(lhs->dVal) | _round(rhs->dVal));
				break;
			case SM_BITWISE_XOR:
				lhs->dVal = (double)(_round(lhs->dVal) ^ _round(rhs->dVal));
				break;
		}
		return true;
	}
}



// Helper function that reads a var and its unary operators from a compiled expression
bool CodeRunner::_readExpVal(unsigned char* code, unsigned int* pos, Instance* derefBuffer, GMLType* argStack, GMLType* out) {
	GMLType var;
	switch (code[*pos]) {
		case EVTYPE_VAL: {
			if (!_parseVal(code + (*pos) + 1, &var)) return false;
			(*pos) += 4;
			break;
		}
		case EVTYPE_INTERNAL_FUNC: {
			unsigned int func = code[(*pos) + 1] | (code[(*pos) + 2] << 8);
			unsigned int argc = code[(*pos) + 3];
			GMLType* argv = (argc > ARG_STACK_SIZE ? new GMLType[argc] : argStack);

			(*pos) += 4;
			for (unsigned int i = 0; i < argc; i++) {
				if (!_parseVal(code + (*pos), argv + i)) {
					if (argc > ARG_STACK_SIZE) delete[] argv;
					return false;
				}
				(*pos) += 3;
			}
			if (!(this->*_gmlFuncs[func])(argc, argv, &var)) {
				if (argc > ARG_STACK_SIZE) delete[] argv;
				return false;
			}

			if (argc > ARG_STACK_SIZE) delete[] argv;
			break;
		}
		case EVTYPE_GAME_VALUE: {
			if (!_getGameValue((CRGameVar)code[(*pos) + 1], code + (*pos) + 2, &var)) return false;
			(*pos) += 5;
			break;
		}
		case EVTYPE_INSTANCEVAR: {
			if (derefBuffer) {
				if (!_getInstanceVar(derefBuffer, (CRInstanceVar)code[(*pos) + 1], code + (*pos) + 2, &var)) return false;
			}
			(*pos) += 5;
			break;
		}
		case EVTYPE_FIELD: {
			unsigned int fieldNum = code[(*pos) + 1] | (code[(*pos) + 2] << 8);
			if (_contexts.top().locals.count(fieldNum)) {
				// This is a local var
				var = _contexts.top().locals[fieldNum];
			}
			else {
				// This is a normal field
				if (derefBuffer) {
					var = _fields[derefBuffer->id][fieldNum];
				}
			}
			(*pos) += 3;
			break;
		}
		case EVTYPE_ARRAY: {
			unsigned int fieldNum = code[(*pos) + 1] | (code[(*pos) + 2] << 8);
			GMLType i1, i2;
			if (!_parseVal(code + (*pos) + 3, &i1)) return false;
			if (!_parseVal(code + (*pos) + 6, &i2)) return false;
			(*pos) += 9;

			if ((i1.state != GMLTypeState::Double) || (i2.state != GMLTypeState::Double)) return false;
			int index1 = _round(i1.dVal);
			int index2 = _round(i2.dVal);
			if (index1 < 0 || index2 < 0) return false;
			if (_contexts.top().locals.count(fieldNum)) {
				// TODO - This is a local var
				return false;
			}
			else {
				// This is a normal field
				if (derefBuffer) {
					var = _arrays[derefBuffer->id][fieldNum][index1][index2];
				}
			}
			break;
		}
		default: {
			return false;
		}
	}

	// Get and apply any unary operators
	while (code[*pos] == EVMOD_NOT || code[*pos] == EVMOD_NEGATIVE || code[*pos] == EVMOD_TILDE) {
		if (var.state == GMLTypeState::String) return false;
		if (code[*pos] == EVMOD_NOT) var.dVal = (_isTrue(&var) ? GMLFalse : GMLTrue);
		else if (code[*pos] == EVMOD_NEGATIVE) var.dVal = -var.dVal;
		else var.dVal = ~_round(var.dVal);
		(*pos)++;
	}

	(*out) = var;
	return true;
}

bool CodeRunner::_evalExpression(unsigned char* code, CodeRunner::GMLType* out) {
	Instance* derefBuffer = _contexts.top().self;
	unsigned int pos = 0;
	GMLType stack[ARG_STACK_SIZE];
	
	// read our first VAR
	GMLType var;
	GMLType rhs;
	if (!_readExpVal(code, &pos, derefBuffer, stack, &var)) return false;

	while (code[pos] == OPERATOR_DEREF) {
		pos++;
		if (var.state == GMLTypeState::String) return false;
		int i = _round(var.dVal);
		switch (i) {
			case -1:
				break;
			case -2:
				derefBuffer = _contexts.top().other;
				break;
			case -3:
				derefBuffer = NULL;
				break;
			case -4:
				derefBuffer = InstanceList::Iterator(_instances).Next();
				break;
			case -5:
				derefBuffer = &global;
				break;
			default:
				derefBuffer = _instances->GetInstanceByNumber(i);
		}
		if (!_readExpVal(code, &pos, derefBuffer, stack, &var)) return false;
	}

	while(true) {
		CROperator op = (CROperator)code[pos];
		if (op == OPERATOR_STOP) {
			(*out) = var;
			return true;
		}
		pos++;

		if (!_readExpVal(code, &pos, derefBuffer, stack, &rhs)) return false;
		if (var.state != rhs.state) return false;

		while (code[pos] == OPERATOR_DEREF) {
			if (rhs.state == GMLTypeState::String) return false;
			pos++;
			int i = _round(rhs.dVal);
			switch (i) {
				case -1:
					break;
				case -2:
					derefBuffer = _contexts.top().other;
					break;
				case -3:
					derefBuffer = NULL;
					break;
				case -4:
					derefBuffer = InstanceList::Iterator(_instances).Next();
					break;
				case -5:
					derefBuffer = &global;
					break;
				default:
					derefBuffer = _instances->GetInstanceByNumber(i);
			}
			if (!_readExpVal(code, &pos, derefBuffer, stack, &rhs)) return false;
		}

		switch (op) {
			case OPERATOR_ADD: {
				if (!_applySetMethod(&var, SM_ADD, &rhs)) return false;
				break;
			}
			case OPERATOR_SUBTRACT: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal -= rhs.dVal;
				break;
			}
			case OPERATOR_MULTIPLY: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal *= rhs.dVal;
				break;
			}
			case OPERATOR_DIVIDE: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal /= rhs.dVal;
				break;
			}
			case OPERATOR_MOD: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = std::fmod(var.dVal, rhs.dVal);
				break;
			}
			case OPERATOR_DIV: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = ::floor(var.dVal / rhs.dVal);
				break;
			}
			case OPERATOR_LTE: {
				if (var.state == GMLTypeState::Double) {
					var.dVal = ((var.dVal < rhs.dVal || _equal(var.dVal, rhs.dVal)) ? GMLTrue : GMLFalse);
				}
				else {
					var.dVal = (var.sVal.length() <= rhs.sVal.length() ? GMLTrue : GMLFalse);
				}
				var.state = GMLTypeState::Double;
				break;
			}
			case OPERATOR_GTE: {
				if (var.state == GMLTypeState::Double) {
					var.dVal = ((var.dVal > rhs.dVal || _equal(var.dVal, rhs.dVal)) ? GMLTrue : GMLFalse);
				}
				else {
					var.dVal = (var.sVal.length() >= rhs.sVal.length() ? GMLTrue : GMLFalse);
				}
				var.state = GMLTypeState::Double;
				break;
			}
			case OPERATOR_LT: {
				if (var.state == GMLTypeState::Double) {
					var.dVal = (var.dVal < rhs.dVal ? GMLTrue : GMLFalse);
				}
				else {
					var.dVal = (var.sVal.length() < rhs.sVal.length() ? GMLTrue : GMLFalse);
				}
				var.state = GMLTypeState::Double;
				break;
			}
			case OPERATOR_GT: {
				if (var.state == GMLTypeState::Double) {
					var.dVal = (var.dVal > rhs.dVal ? GMLTrue : GMLFalse);
				}
				else {
					var.dVal = (var.sVal.length() > rhs.sVal.length() ? GMLTrue : GMLFalse);
				}
				var.state = GMLTypeState::Double;
				break;
			}
			case OPERATOR_EQUALS: {
				if(var.state == GMLTypeState::Double) var.dVal = (_equal(var.dVal, rhs.dVal) ? GMLTrue : GMLFalse);
				else var.dVal = (var.sVal.compare(rhs.sVal) ? GMLFalse : GMLTrue);
				var.state = GMLTypeState::Double;
				break;
			}
			case OPERATOR_NOT_EQUAL: {
				if (var.state == GMLTypeState::Double) var.dVal = (_equal(var.dVal, rhs.dVal) ? GMLFalse : GMLTrue);
				else var.dVal = (var.sVal.compare(rhs.sVal) ? GMLTrue : GMLFalse);
				var.state = GMLTypeState::Double;
				break;
			}
			case OPERATOR_BOOLEAN_AND: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (_isTrue(&var) && _isTrue(&rhs) ? GMLTrue : GMLFalse);
				break;
			}
			case OPERATOR_BOOLEAN_OR: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (_isTrue(&var) || _isTrue(&rhs) ? GMLTrue : GMLFalse);
				break;
			}
			case OPERATOR_BOOLEAN_XOR: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (_isTrue(&var) != _isTrue(&rhs) ? GMLTrue : GMLFalse);
				break;
			}
			case OPERATOR_BITWISE_AND: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (double)(_round(var.dVal) & _round(rhs.dVal));
				break;
			}
			case OPERATOR_BITWISE_OR: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (double)(_round(var.dVal) | _round(rhs.dVal));
				break;
			}
			case OPERATOR_BITWISE_XOR: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (double)(_round(var.dVal) ^ _round(rhs.dVal));
				break;
			}
			case OPERATOR_LSHIFT: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (double)(_round(var.dVal) << _round(rhs.dVal));
				break;
			}
			case OPERATOR_RSHIFT: {
				if (var.state == GMLTypeState::String) return false;
				var.dVal = (double)(_round(var.dVal) >> _round(rhs.dVal));
				break;
			}
			default: {
				return false;
			}
		}
	}
}



bool CodeRunner::_runCode(const unsigned char* bytes, GMLType* out) {
	Instance* derefBuffer = _contexts.top().self;

	bool dereferenced = false;
	unsigned int pos = 0;
	GMLType stack[ARG_STACK_SIZE];

	while (true) {
		switch (bytes[pos]) {
			case OP_NOP: { // Do nothing
				pos++;
				break;
			}
			case OP_EXIT: { // Exit
				return true;
			}
			case OP_SET_GAME_VALUE: { // Set game value
				GMLType valToSet;
				if (!_parseVal(bytes + pos + 6, &valToSet)) return false;
				if (!_setGameValue((CRGameVar)bytes[pos + 1], bytes + pos + 2, (CRSetMethod)bytes[pos + 5], valToSet)) return false;
				pos += 9;
				break;
			}
			case OP_SET_INSTANCE_VAR: { // Set instance variable
				GMLType valToSet;
				if (!_parseVal(bytes + pos + 6, &valToSet)) return false;
				if (!_setInstanceVar(derefBuffer, (CRInstanceVar)bytes[pos + 1], bytes + pos + 2, (CRSetMethod)bytes[pos + 5], valToSet)) return false;
				pos += 9;
				break;
			}
			case OP_SET_FIELD: { // Set a field (also check if it's bound to a local)
				unsigned int fieldNum = bytes[pos + 1] + (bytes[pos + 2] << 8);
				CRSetMethod setMethod = (CRSetMethod)bytes[pos + 3];
				GMLType val;
				if (!_parseVal(bytes + pos + 4, &val)) return false;
				pos += 7;

				if (_contexts.top().locals.count(fieldNum) && !dereferenced) {
					// This is a local
					if (!_applySetMethod(&_contexts.top().locals[fieldNum], setMethod, &val)) return false;
				}
				else {
					// Not a local, so set the actual field
					if (!_applySetMethod(&_fields[derefBuffer->id][fieldNum], setMethod, &val)) return false;
				}

				break;
			}
			case OP_SET_ARRAY: { // Set a field array (also check if it's bound to a local)
				unsigned int fieldNum = bytes[pos + 1] + (bytes[pos + 2] << 8);
				GMLType i1, i2, val;
				if (!_parseVal(bytes + pos + 3, &i1)) return false;
				if (!_parseVal(bytes + pos + 6, &i2)) return false;
				if (!_parseVal(bytes + pos + 10, &val)) return false;
				CRSetMethod setMethod = (CRSetMethod)bytes[pos + 9];
				pos += 13;

				if ((i1.state != GMLTypeState::Double) || (i2.state != GMLTypeState::Double)) return false;
				int index1 = _round(i1.dVal);
				int index2 = _round(i2.dVal);
				if (index1 < 0 || index2 < 0) return false;

				if (_contexts.top().locals.count(fieldNum) && !dereferenced) {
					// This is a local
					// TODO
				}
				else {
					// Not a local, so set the actual field
					if (!_applySetMethod(&_arrays[derefBuffer->id][fieldNum][index1][index2], setMethod, &val)) return false;
				}
				break;
			}
			case OP_BIND_VARS: { // Bind field numbers to local vars for the rest of the script
				pos++;
				unsigned char count = bytes[pos];
				pos++;
				for (unsigned char i = 0; i < count; i++) {
					unsigned int fieldNum = bytes[pos] + (bytes[pos + 1] << 8);
					_contexts.top().locals[fieldNum] = GMLType();
					pos += 2;
				}
				break;
			}
			case OP_DEREF: { // Dereference an expression into the deref buffer
				GMLType val;
				if (!_parseVal(bytes + pos + 1, &val)) return false;
				if (val.state != GMLTypeState::Double) return false;
				pos += 4;
				_stack.push((int)pos);
				dereferenced = true;
				int ii = _round(val.dVal);

				switch (ii) {
					case -1:
						_iterators.push(InstanceList::Iterator(_instances, -1));
						derefBuffer = _contexts.top().self;
						break;
					case -2:
						_iterators.push(InstanceList::Iterator(_instances, -1));
						derefBuffer = _contexts.top().other;
						break;
					case -3:
						_iterators.push(InstanceList::Iterator(_instances, -1));
						derefBuffer = NULL;
						break;
					case -4:
						_iterators.push(InstanceList::Iterator(_instances));
						derefBuffer = _iterators.top().Next();
						break;
					case -5:
						_iterators.push(InstanceList::Iterator(_instances, -1));
						derefBuffer = &global;
						break;
					default:
						_iterators.push(InstanceList::Iterator(_instances, ii));
						derefBuffer = _iterators.top().Next();
				}

				if (derefBuffer) {
					break;
				}

				// Nothing to iterate - this is a crash scenario in GM8
				return false;
			}
			case OP_RESET_DEREF: { // Put default buffer back to default "self" for this context
				Instance* in = _iterators.top().Next();
				if (in) {
					derefBuffer = in;
					pos = _stack.top();
				}
				else {
					_iterators.pop();
					_stack.pop();
					if (_iterators.empty()) {
						derefBuffer = _contexts.top().self;
						pos++;
						dereferenced = false;
					}
					else {
						derefBuffer = _iterators.top().Next();
						pos = _stack.top();
					}
				}
				break;
			}
			case OP_CHANGE_CONTEXT: { // Push new values onto the context stack
				GMLType val;
				if (!_parseVal(bytes + pos + 1, &val)) return false;
				if (val.state != GMLTypeState::Double) return false;
				int id = _round(val.dVal);
				pos += 7;

				if (id >= 0) {
					_contexts.push(CRContext(_contexts.top().self, pos, _instances, id, _contexts.top().argc, _contexts.top().argv));
				}
				else if (id == -1) {
					_contexts.push(CRContext(_contexts.top().self, pos, _instances, _contexts.top().self->id, _contexts.top().argc, _contexts.top().argv));
				}
				else if (id == -2) {
					_contexts.push(CRContext(_contexts.top().self, pos, _instances, _contexts.top().other->id, _contexts.top().argc, _contexts.top().argv));
				}
				else if (id == -3) {
					_contexts.push(CRContext(_contexts.top().self, pos, _instances, _contexts.top().argc, _contexts.top().argv));
					//return false;
				}
				else {
					pos += bytes[pos - 3] + ((unsigned int)bytes[pos - 2] << 8) + ((unsigned int)bytes[pos - 1] << 16);
				}

				derefBuffer = _contexts.top().self;

				if (_contexts.top().self == NULL) {
					// No instances to use in this context
					pos += bytes[pos - 3] + ((unsigned int)bytes[pos - 2] << 8) + ((unsigned int)bytes[pos - 1] << 16);
					_contexts.pop();
				}
				break;
			}
			case OP_REVERT_CONTEXT: { // End of context - get the next instance to run that context on, and continue from here if there are no more.
				Instance* next = _contexts.top().iterator.Next();
				if (next) {
					_contexts.top().self = next;
					pos = _contexts.top().startpos;
				}
				else {
					_contexts.pop();
					pos++;
				}
				derefBuffer = _contexts.top().self;
				break;
			}
			case OP_RUN_INTERNAL_FUNC: { // Run an internal function, not caring about the return value
				unsigned int func = bytes[pos + 1] | (bytes[pos + 2] << 8);
				unsigned int argc = bytes[pos + 3];
				GMLType* argv = (argc > ARG_STACK_SIZE ? new GMLType[argc] : stack);

				pos += 4;
				for (unsigned int i = 0; i < argc; i++) {
					if (!_parseVal(bytes + pos, argv + i)) {
						if (argc > ARG_STACK_SIZE) delete[] argv;
						return false;
					}
					pos += 3;
				}
				if (!(this->*_gmlFuncs[func])(argc, argv, NULL)) return false;
				if (argc > ARG_STACK_SIZE) delete[] argv;
				break;
			}
			case OP_RUN_SCRIPT: { // Run a user script, not caring about the return value
				unsigned int scr = bytes[pos + 1] | (bytes[pos + 2] << 8);
				unsigned int argc = bytes[pos + 3];
				GMLType* argv = (argc > ARG_STACK_SIZE ? new GMLType[argc] : stack);

				pos += 4;
				for (unsigned int i = 0; i < argc; i++) {
					if (!_parseVal(bytes + pos, argv + i)) {
						if (argc > ARG_STACK_SIZE) delete[] argv;
						return false;
					}
					pos += 3;
				}

				Script* script = AMGetScript(scr);
				if (!script->exists) return false;
				if (!this->Run(script->codeObj, _contexts.top().self, _contexts.top().other, _contexts.top().eventId, _contexts.top().eventNumber, _contexts.top().objId, argc, argv)) return false;
				if (argc > ARG_STACK_SIZE) delete[] argv;
				break;
			}
			case OP_TEST_VAL: { // Test if a VAL evaluates to true
				pos++;
				GMLType v;
				if (!_parseVal(bytes + pos, &v)) return false;
				pos += 3;

				if (!_isTrue(&v)) {
					if (bytes[pos] == OP_JUMP || bytes[pos] == OP_JUMP_BACK)
						pos += 2;
					else if (bytes[pos] == OP_JUMP_LONG || bytes[pos] == OP_JUMP_BACK_LONG)
						pos += 4;
				}
				break;
			}
			case OP_TEST_VAL_NOT: { // Test if a VAL evaluates to false
				pos++;
				GMLType v;
				if (!_parseVal(bytes + pos, &v)) return false;
				pos += 3;

				if (_isTrue(&v)) {
					if (bytes[pos] == OP_JUMP || bytes[pos] == OP_JUMP_BACK)
						pos += 2;
					else if (bytes[pos] == OP_JUMP_LONG || bytes[pos] == OP_JUMP_BACK_LONG)
						pos += 4;
				}
				break;
			}
			case OP_TEST_VALS_EQUAL: { // Test if two VALs are equal
				pos++;
				GMLType v1, v2;
				if (!_parseVal(bytes + pos, &v1)) return false;
				pos += 3;
				if (!_parseVal(bytes + pos, &v2)) return false;
				pos += 3;

				if (!((v1.state == v2.state) && (v1.state == GMLTypeState::Double ? (v1.dVal == v2.dVal) : (!v1.sVal.compare(v2.sVal))))) { // when did LISP get here?
					if (bytes[pos] == OP_JUMP || bytes[pos] == OP_JUMP_BACK)
						pos += 2;
					else if (bytes[pos] == OP_JUMP_LONG || bytes[pos] == OP_JUMP_BACK_LONG)
						pos += 4;
				}
			}
			case OP_JUMP: { // short jump forward
				pos += 2 + bytes[pos + 1];
				break;
			}
			case OP_JUMP_BACK: { // short jump forward
				pos -= bytes[pos + 1] - 2;
				break;
			}
			case OP_JUMP_LONG: { // long jump forward
				pos += 4 + (bytes[pos + 1]) + (bytes[pos + 2] << 8) + (bytes[pos + 3] << 16);
				break;
			}
			case OP_JUMP_BACK_LONG: { // long jump backward
				pos -= (bytes[pos + 1]) + (bytes[pos + 2] << 8) + (bytes[pos + 3] << 16) - 4;
				break;
			}
			case OP_SET_INTSTACK: { // Set top value of int stack
				GMLType v;
				if (!_parseVal(bytes + pos + 1, &v)) return false;
				_stack.top() = _round(v.dVal);
				pos += 4;
				break;
			}
			case OP_SET_VARSTACK: { // Set top value of var stack
				GMLType v;
				if (!_parseVal(bytes + pos + 1, &v)) return false;
				_varstack.top() = v;
				pos += 4;
				break;
			}
			case OP_INTSTACK_PUSH: { // Push onto int stack
				_stack.push(0);
				pos++;
				break;
			}
			case OP_INTSTACK_POP: { // Pop from int stack
				_stack.pop();
				pos++;
				break;
			}
			case OP_VARSTACK_PUSH: { // Push onto var stack
				_varstack.push(GMLType());
				pos++;
				break;
			}
			case OP_VARSTACK_POP: { // Pop from var stack
				_varstack.pop();
				pos++;
				break;
			}
			default: {
				// Unknown operator
				return false;
			}
		}
	}

	return false;
}

bool CodeRunner::Run(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc, GMLType* argv) {
	_contexts.push(CRContext(self, other, ev, sub, asObjId, argc, argv));
	GMLType out;
	bool ret = _runCode(_codeObjects[code].compiled, &out);
	_contexts.pop();
	return ret;
}

bool CodeRunner::Query(CodeObject code, Instance* self, Instance* other, bool* response) {
	_contexts.push(CRContext(self, other));
	GMLType resp;
	if (!_evalExpression(_codeObjects[code].compiled, &resp)) return false;
	(*response) = _isTrue(&resp);
	_contexts.pop();
	return true;
}