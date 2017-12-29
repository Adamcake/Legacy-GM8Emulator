#include "CodeRunner.hpp"
#include "CREnums.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "GameRenderer.hpp"
#include "GlobalValues.hpp"

#define ARG_STACK_SIZE 4


int CodeRunner::_round(double d) {
	// This mimics the x86_32 "FISTP" operator which is commonly used in the GM8 runner.
	// We can't actually use that operator, because we're targeting other platforms than x86 Windows.
	int down = (int)d;
	if ((d - down) < 0.5) return down;
	if ((d - down) > 0.5) return (down + 1);
	return down + (down & 1);
}

bool CodeRunner::_parseVal(const unsigned char* val, CodeRunner::GMLType* out) {
	unsigned char type = val[0] >> 6;
	unsigned int index;
	if(type) index = ((val[0] & 0x3F) << 16) + (val[1] << 8) + val[2]; // Index is discarded if type is 0

	switch (type) {
		case 0:
			out->state = GML_TYPE_DOUBLE;
			out->dVal = (double)_stack.top();
			break;
		case 1:
			out->state = GML_TYPE_DOUBLE;
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
	//switch (index) {
		// tbd
	//}
	return true;
}

bool CodeRunner::_getGameValue(CRGameVar index, const unsigned char* arrayIndexVal, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
	switch (index) {
		case MOUSE_X:
			int mx;
			_renderer->GetCursorPos(&mx, NULL);
			out->dVal = (double)mx;
			break;
		case MOUSE_Y:
			int my;
			_renderer->GetCursorPos(NULL, &my);
			out->dVal = (double)my;
			break;
		case ROOM_SPEED:
			out->dVal = (double)_globalValues->room_speed;
			break;
		default:
			return false;
	}
	return true;
}


bool CodeRunner::_setInstanceVar(Instance* instance, CRInstanceVar index, const unsigned char * arrayIndexVal, CRSetMethod method, GMLType value) {
	// No instance vars are strings. In GML if you set an instance var to a string, it gets set to 0.
	GMLType t;
	t.state = GML_TYPE_DOUBLE;
	if (value.state != GML_TYPE_DOUBLE) {
		value.state = GML_TYPE_DOUBLE;
		value.dVal = 0;
	}

	GMLType arrayId;
	int roundedAId;
	switch (index) {
		case IV_ALARM:
			if (!_parseVal(arrayIndexVal, &arrayId)) return false;
			if (arrayId.state != GML_TYPE_DOUBLE) return false;
			roundedAId = _round(arrayId.dVal);
			if (roundedAId < 0) return false;
			instance->alarm[(unsigned int)roundedAId] = (int)(value.state == GML_TYPE_DOUBLE ? value.dVal : 0.0);
			break;
		case IV_DIRECTION:
			t.dVal = instance->direction;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->direction = (t.dVal);
			instance->hspeed = cos(instance->direction * PI / 180.0) * instance->speed;
			instance->vspeed = -sin(instance->direction * PI / 180.0) * instance->speed;
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
		case IV_IMAGE_ANGLE:
			t.dVal = instance->image_angle;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_angle = t.dVal;
			break;
		case IV_IMAGE_XSCALE:
			t.dVal = instance->image_xscale;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_xscale = t.dVal;
			break;
		case IV_IMAGE_YSCALE:
			t.dVal = instance->image_yscale;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->image_yscale = t.dVal;
			break;
		case IV_SPEED:
			t.dVal = instance->speed;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->speed = t.dVal;
			instance->hspeed = cos(instance->direction * PI / 180.0) * instance->speed;
			instance->vspeed = -sin(instance->direction * PI / 180.0) * instance->speed;
			break;
		case IV_X:
			t.dVal = instance->x;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->x = t.dVal;
			break;
		case IV_Y:
			t.dVal = instance->y;
			if (!_applySetMethod(&t, method, &value)) return false;
			instance->y = t.dVal;
			break;
		// more tbd
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_getInstanceVar(Instance* instance, CRInstanceVar index, const unsigned char* arrayIndexVal, GMLType* out) {
	out->state = GML_TYPE_DOUBLE;
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
		case IV_DIRECTION:
			out->dVal = instance->direction;
			break;
		case IV_SPEED:
			out->dVal = instance->speed;
			break;
		case IV_IMAGE_ALPHA:
			out->dVal = instance->image_alpha;
			break;
		case IV_SPRITE_WIDTH:
			if (instance->sprite_index < 0 || instance->sprite_index >= (int)_assetManager->GetSpriteCount()) {
				out->dVal = 0;
			}
			else {
				Sprite* s = _assetManager->GetSprite(instance->sprite_index);
				out->dVal = (s->exists ? s->width : 0);
			}
			break;
		case IV_SPRITE_HEIGHT:
			if (instance->sprite_index < 0 || instance->sprite_index >= (int)_assetManager->GetSpriteCount()) {
				out->dVal = 0;
			}
			else {
				Sprite* s = _assetManager->GetSprite(instance->sprite_index);
				out->dVal = (s->exists ? s->height : 0);
			}
			break;
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_isTrue(const CodeRunner::GMLType* value) {
	return (value->state == GML_TYPE_DOUBLE) && (value->dVal >= 0.5);
}

bool CodeRunner::_applySetMethod(CodeRunner::GMLType* lhs, CRSetMethod method, const CodeRunner::GMLType* const rhs) {
	if (method == SM_ASSIGN) {
		// Easiest method
		memcpy(lhs, rhs, sizeof(GMLType));
		return true;
	}
	else if (method == SM_ADD) {
		// Only other method that can be used on strings
		if ((lhs->state == GML_TYPE_STRING) != (rhs->state == GML_TYPE_STRING)) {
			// Incompatible operands
			return false;
		}
		if (lhs->state == GML_TYPE_STRING) {
			size_t lLen = strlen(lhs->sVal);
			size_t rLen = strlen(rhs->sVal);
			char* c = (char*)malloc(lLen + rLen + 1);
			memcpy(c, lhs->sVal, lLen);
			memcpy(c + lLen, rhs->sVal, rLen);
			c[lLen + rLen] = 0;
			free(lhs->sVal);
			lhs->sVal = c;
		}
		else {
			lhs->dVal += rhs->dVal;
		}
		return true;
	}
	else {
		// No other set methods can be used with strings, so we can error if either one is a string
		if ((lhs->state == GML_TYPE_STRING) || (rhs->state == GML_TYPE_STRING)) {
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



bool CodeRunner::_evalExpression(unsigned char* code, CodeRunner::GMLType* out) {
	unsigned int derefBuffer = _contexts.top().self;
	unsigned int pos = 0;
	GMLType stack[ARG_STACK_SIZE];
	
	// read a VAR
	bool negative = code[0] & 0b00010000;
	bool not = code[0] & 0b00100000;
	bool tilde = code[0] & 0b01000000;

	GMLType var;
	switch (code[0] & 0b00001111) {
		case EVTYPE_VAL: {
			if (!_parseVal(code + 1, &var)) return false;
			pos += 4;
			break;
		}
		case EVTYPE_INTERNAL_FUNC: {
			unsigned int func = code[1] | (code[2] << 8);
			unsigned int argc = code[3];
			GMLType* argv = (argc > ARG_STACK_SIZE ? new GMLType[argc] : stack);

			pos += 4;
			for (unsigned int i = 0; i < argc; i++) {
				if (!_parseVal(code + pos, argv + i)) {
					if (argc > ARG_STACK_SIZE) delete argv;
					return false;
				}
				pos += 3;
			}
			if (!(this->*_gmlFuncs[func])(argc, argv, &var)) return false;

			if (argc > ARG_STACK_SIZE) delete argv;
			break;
		}
		case EVTYPE_GAME_VALUE: {
			if (!_getGameValue((CRGameVar)code[1], code + 2, &var)) return false;
			pos += 5;
			break;
		}
		case EVTYPE_INSTANCEVAR: {
			if (!_getInstanceVar(_instances->GetInstanceByNumber(derefBuffer), (CRInstanceVar)code[1], code + 2, &var)) return false;
			pos += 5;
			break;
		}
		case EVTYPE_FIELD: {
			unsigned int fieldNum = code[1] | (code[2] << 8);
			if (_contexts.top().locals.count(fieldNum)) {
				// This is a local var
				var = _contexts.top().locals[fieldNum];
			}
			else {
				// This is a normal field
				// TODO
				return false;
			}
			pos += 3;
			break;
		}
		default: {
			return false;
		}
	}

	// Apply mods to var
	if (negative && var.state == GML_TYPE_DOUBLE) var.dVal = -var.dVal;
	if (not) {
		var.state = GML_TYPE_DOUBLE;
		var.dVal = (_isTrue(&var) ? 1.0 : 0.0);
	}
	if (tilde) {
		if (var.state == GML_TYPE_DOUBLE) var.dVal = (double)(~_round(var.dVal));
		else {
			var.state = GML_TYPE_DOUBLE;
			var.dVal = -1.0;
		}
	}

	// read an operator, then read the rest of the expression, apply it and return

	switch (code[pos]) {
		case OPERATOR_STOP: {
			(*out) = var;
			return true;
		}
		case OPERATOR_ADD: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			if (!_applySetMethod(&var, SM_ADD, &rest)) return false;
			(*out) = var;
			return true;
		}
		case OPERATOR_SUBTRACT: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			if (!_applySetMethod(&var, SM_SUBTRACT, &rest)) return false;
			(*out) = var;
			return true;
		}
		case OPERATOR_DIVIDE: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			if (!_applySetMethod(&var, SM_DIVIDE, &rest)) return false;
			(*out) = var;
			return true;
		}
		case OPERATOR_LTE: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			out->state = GML_TYPE_DOUBLE;
			out->dVal = (var.dVal <= rest.dVal ? 1.0 : 0.0);
			return true;
		}
		case OPERATOR_GTE: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			out->state = GML_TYPE_DOUBLE;
			out->dVal = (var.dVal >= rest.dVal ? 1.0 : 0.0);
			return true;
		}
		case OPERATOR_LT: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			out->state = GML_TYPE_DOUBLE;
			out->dVal = (var.dVal < rest.dVal ? 1.0 : 0.0);
			return true;
		}
		case OPERATOR_GT: {
			pos++;
			GMLType rest;
			if (!_evalExpression(code + pos, &rest)) return false;
			out->state = GML_TYPE_DOUBLE;
			out->dVal = (var.dVal > rest.dVal ? 1.0 : 0.0);
			return true;
		}
		// more tbd
		default: {
			return false;
		}
	}
}



bool CodeRunner::_runCode(const unsigned char* bytes, GMLType* out) {
	unsigned int derefBuffer = _contexts.top().self;
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
				out->state = GML_TYPE_UNINIT;
				return true;
			}
			case OP_SET_GAME_VALUE: { // Set game value
				GMLType valToSet;
				if (!_parseVal(bytes + 6, &valToSet)) return false;
				if (!_setGameValue((CRGameVar)bytes[pos + 1], bytes + pos + 2, (CRSetMethod)bytes[pos + 5], valToSet)) return false;
				pos += 9;
				break;
			}
			case OP_SET_INSTANCE_VAR: { // Set instance variable
				GMLType valToSet;
				if (!_parseVal(bytes + pos + 6, &valToSet)) return false;
				if (!_setInstanceVar(_instances->GetInstanceByNumber(derefBuffer), (CRInstanceVar)bytes[pos + 1], bytes + pos + 2, (CRSetMethod)bytes[pos + 5], valToSet)) return false;
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
					// TODO
				}

				break;
			}
			case OP_SET_ARRAY: { // Set a field array (also check if it's bound to a local)
				// TODO
				pos += 10;
				break;
			}
			case OP_BIND_VARS: { // Bind field numbers to local vars for the rest of the script
				pos++;
				unsigned char count = bytes[pos];
				pos++;
				for (unsigned char i = 0; i < count; i++) {
					unsigned int fieldNum = bytes[pos] + (bytes[pos + 1] << 8);
					_contexts.top().locals[fieldNum] = GMLType();
					_contexts.top().locals[fieldNum].state = GML_TYPE_UNINIT;
					pos += 2;
				}
				break;
			}
			case OP_DEREF: { // Dereference an expression into the deref buffer
				GMLType val;
				if (!_parseVal(bytes + pos + 1, &val)) return false;
				if (val.state != GML_TYPE_DOUBLE) return false;
				derefBuffer = _round(val.dVal);
				pos += 4;
				dereferenced = true;
				break;
			}
			case OP_RESET_DEREF: { // Put default buffer back to default "self" for this context
				derefBuffer = _contexts.top().self;
				pos++;
				dereferenced = false;
				break;
			}
			case OP_RUN_INTERNAL_FUNC: { // Run an internal function, not caring about the return value
				unsigned int func = bytes[pos + 1] | (bytes[pos + 2] << 8);
				unsigned int argc = bytes[pos + 3];
				GMLType* argv = (argc > ARG_STACK_SIZE ? new GMLType[argc] : stack);

				pos += 4;
				for (unsigned int i = 0; i < argc; i++) {
					if (!_parseVal(bytes + pos, argv + i)) {
						if (argc > ARG_STACK_SIZE) delete argv;
						return false;
					}
					pos += 3;
				}
				if (!(this->*_gmlFuncs[func])(argc, argv, NULL)) return false;
				break;
			}
			case OP_RUN_SCRIPT: { // Run a user script, not caring about the return value
				// TODO
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
			case OP_JUMP: { // short jump forward
				pos += 2 + bytes[pos + 1];
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

bool CodeRunner::Run(CodeObject code, unsigned int self, unsigned int other) {
	_contexts.push(CRContext(self, other));
	GMLType out;
	bool ret = _runCode(_codeObjects[code].compiled, &out);
	_contexts.pop();
	return ret;
}

bool CodeRunner::Query(CodeObject code, unsigned int self, unsigned int other, bool* response) {
	_contexts.push(CRContext(self, other));
	GMLType resp;
	if (!_evalExpression(_codeObjects[code].compiled, &resp)) return false;
	(*response) = _isTrue(&resp);
	_contexts.pop();
	return true;
}