#include "CodeRunner.hpp"
#include "CREnums.hpp"

bool CodeRunner::_parseVal(const unsigned char* val, Instance* self, Instance* other, CodeRunner::GMLType* out) {
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
			if (!_evalExpression(index, self, other, out)) return false;
			break;
		default:
			return false;
	}
	return true;
}

bool CodeRunner::_setGameValue(CRGameVar index, const unsigned char* arrayIndexVal, CRSetMethod method, GMLType value) {
	switch (index) {
		// tbd
	}
	return true;
}

bool CodeRunner::_setInstanceVar(CRGameVar index, const unsigned char * arrayIndexVal, CRSetMethod method, GMLType value) {
	switch (index) {
		case IV_DIRECTION:
			
			break;
	}
	return true;
}

bool CodeRunner::_isTrue(const CodeRunner::GMLType* value) {
	return (value->state == GML_TYPE_DOUBLE) && (value->dVal >= 0.5);
}



bool CodeRunner::_evalExpression(CodeObject obj, Instance * self, Instance * other, CodeRunner::GMLType* out) {
	unsigned char* code;
	// tbd
	return true;
}



bool CodeRunner::Run(CodeObject code, Instance* self, Instance* other) {
	const unsigned char* bytes = _codeObjects[code].compiled;
	unsigned int pos = 0;

	while (true) {
		switch (bytes[pos]) {
			case OP_NOP: // Do nothing
				pos++;
				break;
			case OP_EXIT: // Exit
				return true;
			case OP_SET_GAME_VALUE: // Set game value
				GMLType valToSet;
				_parseVal(bytes + 6, self, other, &valToSet);
				if (!_setGameValue((CRGameVar)bytes[pos + 1], bytes + 2, (CRSetMethod)bytes[5], valToSet)) return false;
				pos += 9;
				break;
			case OP_SET_INSTANCE_VAR: // Set instance variable
				break;
		}
	}

	return false;
}

bool CodeRunner::Query(CodeObject code, Instance * self, Instance * other, bool* response) {
	GMLType resp;
	if (!_evalExpression(code, self, other, &resp)) return false;
	(*response) = _isTrue(&resp);
	return true;
}