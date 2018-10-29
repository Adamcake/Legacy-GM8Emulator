#include <pch.h>
#include "CodeRunner.hpp"
#include "AssetManager.hpp"
#include "CodeActionManager.hpp"
#include "Compiler/Tokenizer.hxx"
#include "InstanceList.hpp"
#include "RNG.hpp"
#include "CRGMLType.hpp"
#include "Compiler/CRRuntime.hpp"
#include "Compiler/Compiled.hpp"
#include "Compiler/Interpreter.hpp"
#include "CREnums.hpp"
#include "Compiler/Interpreter.hpp"

// Internal code object
struct CRCodeObject {
    std::string code;
    GM8Emulator::Compiler::TokenList _tokenized;
    bool question;
    CRActionList _actions;
    CRExpression _expression;
    CRCodeObject(const char* c, unsigned int l, bool q) : question(q) {
        std::copy(c, c + l, std::back_inserter(code));
        _tokenized = GM8Emulator::Compiler::TokenList(code.c_str(), l);
    }
};
std::vector<CRCodeObject> _codeObjects;

// Global game value settings
GlobalValues* _globalValues;

CodeRunner::CodeRunner(GlobalValues* globals) {
    _globalValues = globals;
    GM8Emulator::Compiler::Init();
    Runtime::Init(globals);
    RNGRandomize();
}

CodeRunner::~CodeRunner() {
    for (unsigned int i = 0; i < _codeObjects.size(); i++) {
        // todo
    }
    Runtime::Finalize();
}

CodeObject CodeRunner::Register(char* code, unsigned int len) {
    unsigned int ix = ( unsigned int )_codeObjects.size();
    _codeObjects.push_back(CRCodeObject(code, len, false));

    return ix;
}

CodeObject CodeRunner::RegisterQuestion(char* code, unsigned int len) {
    unsigned int ix = ( unsigned int )_codeObjects.size();
    _codeObjects.push_back(CRCodeObject(code, len, true));

    return ix;
}

bool CodeRunner::Compile(CodeObject object) {
    try {
        if (_codeObjects[object].question) {
            if(!GM8Emulator::Compiler::InterpretExpression(_codeObjects[object]._tokenized, &_codeObjects[object]._expression)) return false;
        }
        else {
            if (!GM8Emulator::Compiler::Interpret(_codeObjects[object]._tokenized, &_codeObjects[object]._actions)) return false;
        }
        return true;
    }
    catch (const std::runtime_error&) {
        return false;
	}
}

bool CodeRunner::Run(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId) {
    return Runtime::Execute(_codeObjects[code]._actions);
}

bool CodeRunner::Query(CodeObject code, Instance* self, Instance* other, bool* response) {
    GMLType t;
    if(!Runtime::EvalExpression(_codeObjects[code]._expression, &t)) return false;
    (*response) = Runtime::_isTrue(&t);
    return true;
}


void CodeRunner::SetRoomOrder(unsigned int** order, unsigned int count) {
    Runtime::SetRoomOrder(order, count);
}

bool CodeRunner::Init() {
    return true;
}
