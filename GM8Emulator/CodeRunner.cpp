#include "CodeRunner.hpp"
#include "AssetManager.hpp"
#include "InstanceList.hpp"


CodeRunner::CodeRunner(AssetManager* assets, InstanceList* instances) {
	_assetManager = assets;
	_instances = instances;
}

CodeRunner::~CodeRunner() {
	for (unsigned int i = 0; i < _codeObjects.size(); i++) {
		free(_codeObjects[i].code);
	}
}

CodeObject CodeRunner::Register(char* code, unsigned int len) {
	unsigned int ix = (unsigned int)_codeObjects.size();
	_codeObjects.push_back(CRCodeObject());

	_codeObjects[ix].code = (char*)malloc(len);
	memcpy(_codeObjects[ix].code, code, len);
	_codeObjects[ix].codeLength = len;
	_codeObjects[ix].question = false;
	
	return ix;
}

CodeObject CodeRunner::RegisterQuestion(char * code, unsigned int len) {
	unsigned int ix = (unsigned int)_codeObjects.size();
	_codeObjects.push_back(CRCodeObject());

	_codeObjects[ix].code = (char*)malloc(len);
	memcpy(_codeObjects[ix].code, code, len);
	_codeObjects[ix].codeLength = len;
	_codeObjects[ix].question = true;

	return ix;
}

bool CodeRunner::Compile(CodeObject object) {
	// tbd
	free(_codeObjects[object].code);
	_codeObjects[object].code = NULL;
	return true;
}

bool CodeRunner::Run(CodeObject code, Instance* self, Instance* other) {
	// tbd
	return true;
}

bool CodeRunner::Query(CodeObject code, Instance * self, Instance * other, bool* response) {
	// tbd
	return true;
}
