#include <pch.h>
#include "CRInterpretation.hpp"

// Write statements to bytecode
void _CompileStatements(const std::vector<CRStatement*>* const statements, CRSOutput* output) {
	for (const CRStatement* s : *statements) {
		s->write(output);
	}
}

// All subtypes of CRStatement have a write() function which is for writing compiled bytecode into an output buffer.

// EXIT

void CRSExit::write(CRSOutput* output) const {
	output->_output.push_back(OP_EXIT);
}

// BIND VARS

void CRSBindVars::write(CRSOutput* output) const {
	// Accounts for cases where more than 255 locals are bound
	size_t i = _vars.size();
	size_t pos = 0;
	while (i > 255) {
		i -= 255;
		output->_output.push_back(OP_BIND_VARS);
		output->_output.push_back((unsigned char)0xFF);
		for (unsigned int ii = 0; ii <= 255; ii++) {
			unsigned int field = _vars[pos];
			output->_output.push_back((unsigned char)(field & 0xFF));
			output->_output.push_back((unsigned char)((field >> 8) & 0xFF));
			pos++;
		}
	}
	output->_output.push_back(OP_BIND_VARS);
	output->_output.push_back((unsigned char)i);
	while (pos < _vars.size()) {
		unsigned int field = _vars[pos];
		output->_output.push_back((unsigned char)(field & 0xFF));
		output->_output.push_back((unsigned char)((field >> 8) & 0xFF));
		pos++;
	}
}

// WHILE

void CRSWhile::write(CRSOutput* output) const {
	output->_output.push_back(OP_TEST_VAL_NOT);
	output->_output.push_back(_test[0]);
	output->_output.push_back(_test[1]);
	output->_output.push_back(_test[2]);

	CRSOutput comp;
	_CompileStatements(&_code, &comp);

	bool longJumps = comp._output.size() > 247;
	if (longJumps) {
		output->_output.push_back(OP_JUMP_LONG);
		unsigned int dist = static_cast<unsigned int>(comp._output.size()) + 4;
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP);
		output->_output.push_back((unsigned char)(comp._output.size() + 2));
	}

	for (unsigned int off : comp._continues) {
		bool longJmp = (comp._output.size() - off) > 251;
		unsigned int jd = (static_cast<unsigned int>(comp._output.size()) - off) - (longJmp ? 4 : 2);
		comp._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		comp._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			comp._output[off + 2] = ((jd >> 8) & 0xFF);
			comp._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}
	for (unsigned int off : comp._breaks) {
		bool longJmp = (comp._output.size() - off) > 253;
		unsigned int jd = (static_cast<unsigned int>(comp._output.size()) - off) + (longJumps ? 4 : 2) - (longJmp ? 4 : 2);
		comp._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		comp._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			comp._output[off + 2] = ((jd >> 8) & 0xFF);
			comp._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}

	output->_output.insert(output->_output.end(), comp._output.begin(), comp._output.end());

	if (longJumps) {
		output->_output.push_back(OP_JUMP_BACK_LONG);
		unsigned int dist = static_cast<unsigned int>(comp._output.size()) + 12;
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP_BACK);
		output->_output.push_back((unsigned char)(comp._output.size() + 8));
	}
}

// DO-UNTIL

void CRSDoUntil::write(CRSOutput* output) const {
	CRSOutput comp;
	_CompileStatements(&_code, &comp);
	bool longJmpEnd = (comp._output.size() > 249);

	for (unsigned int off : comp._continues) {
		bool longJmp = (comp._output.size() - off) > 251;
		unsigned int jd = (static_cast<unsigned int>(comp._output.size()) - off) - (longJmp ? 4 : 2);
		comp._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		comp._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			comp._output[off + 2] = ((jd >> 8) & 0xFF);
			comp._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}
	for (unsigned int off : comp._breaks) {
		bool longJmp = (comp._output.size() - off) > 253;
		unsigned int jd = (static_cast<unsigned int>(comp._output.size()) - off) + (longJmpEnd ? 4 : 2) - (longJmp ? 4 : 2);
		comp._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		comp._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			comp._output[off + 2] = ((jd >> 8) & 0xFF);
			comp._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}

	output->_output.insert(output->_output.end(), comp._output.begin(), comp._output.end());

	output->_output.push_back(OP_TEST_VAL_NOT);
	output->_output.push_back(_test[0]);
	output->_output.push_back(_test[1]);
	output->_output.push_back(_test[2]);

	if (longJmpEnd) {
		output->_output.push_back(OP_JUMP_BACK_LONG);
		unsigned int dist = static_cast<unsigned int>(comp._output.size()) + 8;
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP_BACK);
		output->_output.push_back((unsigned char)(comp._output.size() + 6));
	}
}

// FOR

void CRSFor::write(CRSOutput* output) const {
	CRSOutput init;
	_CompileStatements(&_init, &init);
	output->_output.insert(output->_output.end(), init._output.begin(), init._output.end());

	CRSOutput code;
	CRSOutput final;
	_CompileStatements(&_code, &code);
	_CompileStatements(&_final, &final);

	output->_output.push_back(OP_TEST_VAL_NOT);
	output->_output.push_back(_test[0]);
	output->_output.push_back(_test[1]);
	output->_output.push_back(_test[2]);

	bool longJumps = (code._output.size() + final._output.size() > 247);
	if (longJumps) {
		output->_output.push_back(OP_JUMP_LONG);
		unsigned int dist = (static_cast<unsigned int>(code._output.size()) + static_cast<unsigned int>(final._output.size()) + 4);
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP);
		output->_output.push_back((unsigned char)(code._output.size() + final._output.size() + 2));
	}

	for (unsigned int off : code._continues) {
		bool longJmp = (code._output.size() - off) > 251;
		unsigned int jd = (static_cast<unsigned int>(code._output.size()) - off) - (longJmp ? 4 : 2);
		code._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		code._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			code._output[off + 2] = ((jd >> 8)  & 0xFF);
			code._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}
	for (unsigned int off : code._breaks) {
		bool longJmp = (code._output.size() + final._output.size() - off) > 249;
		unsigned int jd = (static_cast<unsigned int>(code._output.size()) + static_cast<unsigned int>(final._output.size()) - off) + (longJumps ? 4 : 2) - (longJmp ? 4 : 2);
		code._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		code._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			code._output[off + 2] = ((jd >> 8) & 0xFF);
			code._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}

	output->_output.insert(output->_output.end(), code._output.begin(), code._output.end());
	output->_output.insert(output->_output.end(), final._output.begin(), final._output.end());

	if (longJumps) {
		output->_output.push_back(OP_JUMP_BACK_LONG);
		unsigned int dist = (static_cast<unsigned int>(code._output.size()) + static_cast<unsigned int>(final._output.size()) + 12);
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP_BACK);
		output->_output.push_back((unsigned char)(code._output.size() + final._output.size() + 8));
	}
}

// REPEAT

void CRSRepeat::write(CRSOutput* output) const {
	output->_output.push_back(OP_INTSTACK_PUSH);
	output->_output.push_back(OP_SET_INTSTACK);
	output->_output.push_back(_count[0]);
	output->_output.push_back(_count[1]);
	output->_output.push_back(_count[2]);

	CRSOutput code;
	_CompileStatements(&_code, &code);

	output->_output.push_back(OP_TEST_VAL_NOT);
	output->_output.push_back(0);
	output->_output.push_back(0);
	output->_output.push_back(0);

	unsigned int dist = static_cast<unsigned int>(code._output.size()) + 8;

	if (dist > 255) {
		output->_output.push_back(OP_JUMP_LONG);
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP);
		output->_output.push_back((unsigned char)dist);
	}

	for (unsigned int off : code._continues) {
		bool longJmp = (code._output.size() - off) > 253;
		unsigned int jd = (static_cast<unsigned int>(code._output.size()) - off) - (longJmp ? 4 : 2);
		code._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		code._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			code._output[off + 2] = ((jd >> 8) & 0xFF);
			code._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}
	for (unsigned int off : code._breaks) {
		bool longJmp = (code._output.size() - off) > 245;
		unsigned int jd = ((static_cast<unsigned int>(code._output.size()) - off) - (longJmp ? 4 : 2)) + 8;
		code._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		code._output[off + 1] = (jd & 0xFF);
		if (longJmp) {
			code._output[off + 2] = ((jd >> 8) & 0xFF);
			code._output[off + 3] = ((jd >> 16) & 0xFF);
		}
	}

	output->_output.insert(output->_output.end(), code._output.begin(), code._output.end());

	output->_output.push_back(OP_SET_INTSTACK);
	output->_output.push_back(0xc0);
	output->_output.push_back(0);
	output->_output.push_back(0);

	dist += (dist > 255 ? 8 : 6);
	output->_output.push_back(OP_JUMP_BACK_LONG);
	output->_output.push_back((unsigned char)(dist & 0xFF));
	output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
	output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));

	output->_output.push_back(OP_INTSTACK_POP);
}

// IF

void CRSIf::write(CRSOutput* output) const {
	bool useElse = (_elseCode.size() > 0);

	output->_output.push_back(OP_TEST_VAL_NOT);
	output->_output.push_back(_test[0]);
	output->_output.push_back(_test[1]);
	output->_output.push_back(_test[2]);

	CRSOutput ifb;
	CRSOutput elseb;
	_CompileStatements(&_code, &ifb);
	if(useElse) _CompileStatements(&_elseCode, &elseb);

	unsigned int dist = static_cast<unsigned int>(ifb._output.size());
	if (useElse) dist += (elseb._output.size() > 255 ? 4 : 2);

	if (dist > 255) {
		output->_output.push_back(OP_JUMP_LONG);
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP);
		output->_output.push_back((unsigned char)dist);
	}

	for (unsigned int off : ifb._continues) {
		output->_continues.push_back(off + static_cast<unsigned int>(output->_output.size()));
	}
	for (unsigned int off : ifb._breaks) {
		output->_breaks.push_back(off + static_cast<unsigned int>(output->_output.size()));
	}

	output->_output.insert(output->_output.end(), ifb._output.begin(), ifb._output.end());

	if (useElse) {
		unsigned int edist = (unsigned int)elseb._output.size();
		if (edist > 255) {
			output->_output.push_back(OP_JUMP_LONG);
			output->_output.push_back((unsigned char)(edist & 0xFF));
			output->_output.push_back((unsigned char)((edist >> 8) & 0xFF));
			output->_output.push_back((unsigned char)((edist >> 16) & 0xFF));
		}
		else {
			output->_output.push_back(OP_JUMP);
			output->_output.push_back((unsigned char)edist);
		}

		for (unsigned int off : elseb._continues) {
			output->_continues.push_back(off + static_cast<unsigned int>(output->_output.size()));
		}
		for (unsigned int off : elseb._breaks) {
			output->_breaks.push_back(off + static_cast<unsigned int>(output->_output.size()));
		}

		output->_output.insert(output->_output.end(), elseb._output.begin(), elseb._output.end());
	}
}

// SWITCH

void CRSSwitch::write(CRSOutput* output) const {
	std::map<unsigned int, unsigned int> codeOffsets;
	CRSOutput comp;
	unsigned int i;
	for (i = 0; i < _code.size(); i++) {
		codeOffsets[i] = static_cast<unsigned int>(comp._output.size());
		_code[i]->write(&comp);
	}
	codeOffsets[i] = static_cast<unsigned int>(comp._output.size());

	unsigned int dist = static_cast<unsigned int>(comp._output.size()) + 4;
	if (dist > 251) {
		output->_output.push_back(OP_JUMP_LONG);
		output->_output.push_back((unsigned char)(dist & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 8) & 0xFF));
		output->_output.push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->_output.push_back(OP_JUMP);
		output->_output.push_back((unsigned char)dist);
	}

	for (unsigned int off : comp._breaks) {
		unsigned int jmp = (static_cast<unsigned int>(comp._output.size()) - off) - 4;
		bool longJmp = (jmp > 255);
		comp._output[off] = (longJmp ? OP_JUMP_LONG : OP_JUMP);
		comp._output[off + 1] = (jmp & 0xFF);
		if (longJmp) {
			comp._output[off + 2] = ((jmp >> 8)  & 0xFF);
			comp._output[off + 3] = ((jmp >> 16) & 0xFF);
		}
	}
	for (unsigned int off : comp._continues) {
		output->_continues.push_back(off + static_cast<unsigned int>(output->_output.size()));
	}

	output->_output.insert(output->_output.end(), comp._output.begin(), comp._output.end());

	unsigned int finalJmpPos = (unsigned int)output->_output.size();
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);

	unsigned int jmpsLength = 0;
	for (const CRSwitchCase i : _cases) {
		int jdist = (jmpsLength + dist) - codeOffsets[i._offset];
		if (!i._default) {
			output->_output.push_back(OP_TEST_VALS_EQUAL);
			output->_output.push_back(_val[0]);
			output->_output.push_back(_val[1]);
			output->_output.push_back(_val[2]);
			output->_output.push_back(i._val[0]);
			output->_output.push_back(i._val[1]);
			output->_output.push_back(i._val[2]);
			jmpsLength += 7;
			jdist += 7;
		}
		if (jdist > 253) {
			jdist += 4;
			output->_output.push_back(OP_JUMP_BACK_LONG);
			output->_output.push_back((unsigned char)(jdist & 0xFF));
			output->_output.push_back((unsigned char)((jdist >> 8) & 0xFF));
			output->_output.push_back((unsigned char)((jdist >> 16) & 0xFF));
			jmpsLength += 4;
		}
		else {
			jdist += 2;
			output->_output.push_back(OP_JUMP_BACK);
			output->_output.push_back((unsigned char)jdist);
			jmpsLength += 2;
		}
	}

	output->_output[finalJmpPos] = OP_JUMP_LONG;
	output->_output[finalJmpPos + 1] = (jmpsLength & 0xFF);
	output->_output[finalJmpPos + 2] = ((jmpsLength >> 8) & 0xFF);
	output->_output[finalJmpPos + 3] = ((jmpsLength >> 16) & 0xFF);
}

// BREAK

void CRSBreak::write(CRSOutput* output) const {
	output->_breaks.push_back(static_cast<unsigned int>(output->_output.size()));
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
}

// CONTINUE

void CRSContinue::write(CRSOutput* output) const {
	output->_continues.push_back(static_cast<unsigned int>(output->_output.size()));
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
	output->_output.push_back(OP_NOP);
}

// WITH

void CRSWith::write(CRSOutput* output) const {
	CRSOutput comp;
	_CompileStatements(&_code, &comp);

	output->_output.push_back(OP_CHANGE_CONTEXT);
	output->_output.push_back(_id[0]);
	output->_output.push_back(_id[1]);
	output->_output.push_back(_id[2]);

	size_t len = comp._output.size() + 1;
	output->_output.push_back(len & 0xFF);
	output->_output.push_back((len >> 8) & 0xFF);
	output->_output.push_back((len >> 16) & 0xFF);

	output->_output.insert(output->_output.end(), comp._output.begin(), comp._output.end());
	output->_output.push_back(OP_REVERT_CONTEXT);
}

// RETURN

void CRSReturn::write(CRSOutput* output) const {
	output->_output.push_back(OP_RETURN);
	output->_output.push_back(_val[0]);
	output->_output.push_back(_val[1]);
	output->_output.push_back(_val[2]);
}

// USER SCRIPT

void CRSUserScript::write(CRSOutput* output) const {
	output->_output.push_back(OP_RUN_SCRIPT);
	output->_output.push_back((unsigned char)(_id & 0xFF));
	output->_output.push_back((unsigned char)((_id >> 8) & 0xFF));
	output->_output.push_back((unsigned char)(_argCount & 0xFF));

	size_t pos = 0;
	for (unsigned int i = 0; i < _argCount; i++) {
		output->_output.push_back(_args[pos]);
		pos++;
		output->_output.push_back(_args[pos]);
		pos++;
		output->_output.push_back(_args[pos]);
		pos++;
	}
}

// INTERNAL FUNCTION

void CRSFunction::write(CRSOutput* output) const {
	output->_output.push_back(OP_RUN_INTERNAL_FUNC);
	output->_output.push_back((unsigned char)(_id & 0xFF));
	output->_output.push_back((unsigned char)((_id >> 8) & 0xFF));
	output->_output.push_back((unsigned char)(_argCount & 0xFF));

	size_t pos = 0;
	for (unsigned int i = 0; i < _argCount; i++) {
		output->_output.push_back(_args[pos]);
		pos++;
		output->_output.push_back(_args[pos]);
		pos++;
		output->_output.push_back(_args[pos]);
		pos++;
	}
}

// PRE COMPILED

void CRSPreCompiled::write(CRSOutput* output) const {
	output->_output.insert(output->_output.end(), _code.begin(), _code.end());
}