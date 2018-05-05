#include "CRInterpretation.hpp"
#include "CREnums.hpp"
#include <map>

// Write statements to bytecode
void _CompileStatements(const std::vector<CRStatement*>* const statements, std::vector<unsigned char>* output) {
	for (const CRStatement* s : *statements) {
		s->write(output);
	}
}

// All subtypes of CRStatement have a write() function which is for writing compiled bytecode into an output buffer.

// EXIT

void CRSExit::write(std::vector<unsigned char>* output) const {
	output->push_back(OP_EXIT);
}

// BIND VARS

void CRSBindVars::write(std::vector<unsigned char>* output) const {
	// Accounts for cases where more than 255 locals are bound
	size_t i = _vars.size();
	size_t pos = 0;
	while (i > 255) {
		i -= 255;
		output->push_back(OP_BIND_VARS);
		output->push_back((unsigned char)0xFF);
		for (unsigned int ii = 0; ii <= 255; ii++) {
			unsigned int field = _vars[pos];
			output->push_back((unsigned char)(field & 0xFF));
			output->push_back((unsigned char)((field >> 8) & 0xFF));
			pos++;
		}
	}
	output->push_back(OP_BIND_VARS);
	output->push_back((unsigned char)i);
	while (pos < _vars.size()) {
		unsigned int field = _vars[pos];
		output->push_back((unsigned char)(field & 0xFF));
		output->push_back((unsigned char)((field >> 8) & 0xFF));
		pos++;
	}
}

// WHILE

void CRSWhile::write(std::vector<unsigned char>* output) const {
	output->push_back(OP_TEST_VAL_NOT);
	output->push_back(_test[0]);
	output->push_back(_test[1]);
	output->push_back(_test[2]);

	std::vector<unsigned char> comp;
	_CompileStatements(&_code, &comp);

	bool longJumps = comp.size() > 247;
	if (longJumps) {
		output->push_back(OP_JUMP_LONG);
		unsigned int dist = comp.size() + 4;
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP);
		output->push_back((unsigned char)(comp.size() + 2));
	}

	output->insert(output->end(), comp.begin(), comp.end());

	if (longJumps) {
		output->push_back(OP_JUMP_BACK_LONG);
		unsigned int dist = comp.size() + 12;
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP_BACK);
		output->push_back((unsigned char)(comp.size() + 8));
	}
}

// DO-UNTIL

void CRSDoUntil::write(std::vector<unsigned char>* output) const {
	std::vector<unsigned char> comp;
	_CompileStatements(&_code, &comp);

	output->insert(output->end(), comp.begin(), comp.end());

	output->push_back(OP_TEST_VAL);
	output->push_back(_test[0]);
	output->push_back(_test[1]);
	output->push_back(_test[2]);

	if (comp.size() > 249) {
		output->push_back(OP_JUMP_BACK_LONG);
		unsigned int dist = comp.size() + 8;
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP_BACK);
		output->push_back((unsigned char)(comp.size() + 6));
	}
}

// FOR

void CRSFor::write(std::vector<unsigned char>* output) const {
	std::vector<unsigned char> init;
	_CompileStatements(&_init, &init);
	output->insert(output->end(), init.begin(), init.end());

	std::vector<unsigned char> code;
	std::vector<unsigned char> final;
	_CompileStatements(&_code, &code);
	_CompileStatements(&_final, &final);

	output->push_back(OP_TEST_VAL);
	output->push_back(_test[0]);
	output->push_back(_test[1]);
	output->push_back(_test[2]);

	bool longJumps = (code.size() + final.size() > 247);
	if (longJumps) {
		output->push_back(OP_JUMP_LONG);
		unsigned int dist = (code.size() + final.size() + 4);
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP);
		output->push_back((unsigned char)(code.size() + final.size() + 2));
	}

	output->insert(output->end(), code.begin(), code.end());
	output->insert(output->end(), final.begin(), final.end());

	if (longJumps) {
		output->push_back(OP_JUMP_BACK_LONG);
		unsigned int dist = (code.size() + final.size() + 12);
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP_BACK);
		output->push_back((unsigned char)(code.size() + final.size() + 8));
	}
}

// IF

void CRSIf::write(std::vector<unsigned char>* output) const {
	bool useElse = (_elseCode.size() > 0);

	output->push_back(OP_TEST_VAL_NOT);
	output->push_back(_test[0]);
	output->push_back(_test[1]);
	output->push_back(_test[2]);

	std::vector<unsigned char> ifb;
	std::vector<unsigned char> elseb;
	_CompileStatements(&_code, &ifb);
	if(useElse) _CompileStatements(&_elseCode, &elseb);

	unsigned int dist = ifb.size();
	if (useElse) dist += (elseb.size() > 255 ? 4 : 2);

	if (dist > 255) {
		output->push_back(OP_JUMP_LONG);
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP);
		output->push_back((unsigned char)dist);
	}

	output->insert(output->end(), ifb.begin(), ifb.end());

	if (useElse) {
		unsigned int edist = (unsigned int)elseb.size();
		if (edist > 255) {
			output->push_back(OP_JUMP_LONG);
			output->push_back((unsigned char)(edist & 0xFF));
			output->push_back((unsigned char)((edist >> 8) & 0xFF));
			output->push_back((unsigned char)((edist >> 16) & 0xFF));
		}
		else {
			output->push_back(OP_JUMP);
			output->push_back((unsigned char)edist);
		}

		output->insert(output->end(), elseb.begin(), elseb.end());
	}
}

// SWITCH

void CRSSwitch::write(std::vector<unsigned char>* output) const {
	std::map<unsigned int, unsigned int> codeOffsets;
	std::vector<unsigned char> comp;
	unsigned int i;
	for (i = 0; i < _code.size(); i++) {
		codeOffsets[i] = comp.size();
		_code[i]->write(&comp);
	}
	codeOffsets[i] = comp.size();

	unsigned int dist = comp.size() + 4;
	if (dist > 251) {
		output->push_back(OP_JUMP_LONG);
		output->push_back((unsigned char)(dist & 0xFF));
		output->push_back((unsigned char)((dist >> 8) & 0xFF));
		output->push_back((unsigned char)((dist >> 16) & 0xFF));
	}
	else {
		output->push_back(OP_JUMP);
		output->push_back((unsigned char)dist);
	}
	output->insert(output->end(), comp.begin(), comp.end());

	unsigned int finalJmpPos = (unsigned int)output->size();
	output->push_back(OP_NOP);
	output->push_back(OP_NOP);
	output->push_back(OP_NOP);
	output->push_back(OP_NOP);

	unsigned int jmpsLength = 0;
	for (const CRSwitchCase i : _cases) {
		int jdist = (jmpsLength + dist) - codeOffsets[i._offset];
		if (!i._default) {
			output->push_back(OP_TEST_VALS_EQUAL);
			output->push_back(_val[0]);
			output->push_back(_val[1]);
			output->push_back(_val[2]);
			output->push_back(i._val[0]);
			output->push_back(i._val[1]);
			output->push_back(i._val[2]);
			jmpsLength += 7;
			jdist += 7;
		}
		if (jdist > 253) {
			jdist += 4;
			output->push_back(OP_JUMP_BACK_LONG);
			output->push_back((unsigned char)(jdist & 0xFF));
			output->push_back((unsigned char)((jdist >> 8) & 0xFF));
			output->push_back((unsigned char)((jdist >> 16) & 0xFF));
			jmpsLength += 4;
		}
		else {
			jdist += 2;
			output->push_back(OP_JUMP_BACK);
			output->push_back((unsigned char)jdist);
			jmpsLength += 2;
		}
	}

	(*output)[finalJmpPos] = OP_JUMP_LONG;
	(*output)[finalJmpPos + 1] = (jmpsLength & 0xFF);
	(*output)[finalJmpPos + 2] = ((jmpsLength >> 8) & 0xFF);
	(*output)[finalJmpPos + 3] = ((jmpsLength >> 16) & 0xFF);
}

// WITH

void CRSWith::write(std::vector<unsigned char>* output) const {
	std::vector<unsigned char> comp;
	_CompileStatements(&_code, &comp);

	output->push_back(OP_CHANGE_CONTEXT);
	output->push_back(_id[0]);
	output->push_back(_id[1]);
	output->push_back(_id[2]);

	size_t len = comp.size() + 1;
	output->push_back(len & 0xFF);
	output->push_back((len >> 8) & 0xFF);
	output->push_back((len >> 16) & 0xFF);

	output->insert(output->end(), comp.begin(), comp.end());
	output->push_back(OP_REVERT_CONTEXT);
}

// RETURN

void CRSReturn::write(std::vector<unsigned char>* output) const {
	output->push_back(OP_RETURN);
	output->push_back(_val[0]);
	output->push_back(_val[1]);
	output->push_back(_val[2]);
}

// USER SCRIPT

void CRSUserScript::write(std::vector<unsigned char>* output) const {
	output->push_back(OP_RUN_SCRIPT);
	output->push_back((unsigned char)(_id & 0xFF));
	output->push_back((unsigned char)((_id >> 8) & 0xFF));
	output->push_back((unsigned char)(_argCount & 0xFF));

	size_t pos = 0;
	for (unsigned int i = 0; i < _argCount; i++) {
		output->push_back(_args[pos]);
		pos++;
		output->push_back(_args[pos]);
		pos++;
		output->push_back(_args[pos]);
		pos++;
	}
}

// INTERNAL FUNCTION

void CRSFunction::write(std::vector<unsigned char>* output) const {
	output->push_back(OP_RUN_INTERNAL_FUNC);
	output->push_back((unsigned char)(_id & 0xFF));
	output->push_back((unsigned char)((_id >> 8) & 0xFF));
	output->push_back((unsigned char)(_argCount & 0xFF));

	size_t pos = 0;
	for (unsigned int i = 0; i < _argCount; i++) {
		output->push_back(_args[pos]);
		pos++;
		output->push_back(_args[pos]);
		pos++;
		output->push_back(_args[pos]);
		pos++;
	}
}

// PRE COMPILED

void CRSPreCompiled::write(std::vector<unsigned char>* output) const {
	output->insert(output->end(), _code.begin(), _code.end());
}