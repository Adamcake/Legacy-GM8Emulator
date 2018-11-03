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
    char* _code;
    GM8Emulator::Compiler::TokenList _tokenized;
    bool question;
    CRActionList _actions;
    CRExpression _expression;
    CRCodeObject(const char* c, unsigned int l, bool q) : question(q) {
        _code = (char*)malloc(l);
        memcpy(_code, c, l);
        _tokenized = GM8Emulator::Compiler::TokenList(_code, l);
    }
};
std::vector<CRCodeObject> _codeObjects;

// Global game value settings
GlobalValues* _crGlobalValues;

bool CodeManager::Init(GlobalValues* globals) {
    _crGlobalValues = globals;
    RNGRandomize();
    return GM8Emulator::Compiler::Init(globals);
}

void CodeManager::Finalize() {
    for (unsigned int i = 0; i < _codeObjects.size(); i++) {
        // todo
    }
    Runtime::Finalize();
}

CodeObject CodeManager::Register(char* code, unsigned int len) {
    unsigned int ix = ( unsigned int )_codeObjects.size();
    _codeObjects.push_back(CRCodeObject(code, len, false));

    return ix;
}

CodeObject CodeManager::RegisterQuestion(char* code, unsigned int len) {
    unsigned int ix = ( unsigned int )_codeObjects.size();
    _codeObjects.push_back(CRCodeObject(code, len, true));

    return ix;
}

bool CodeManager::Compile(CodeObject object) {
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

bool CodeManager::Run(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc, GMLType* argv) {
    return Runtime::Execute(_codeObjects[code]._actions, self, other, ev, sub, asObjId, argc, argv);
}

bool CodeManager::Query(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, bool* response) {
    GMLType t;
    if (!Runtime::EvalExpression(_codeObjects[code]._expression, self, other, ev, sub, asObjId, &t)) return false;
    (*response) = Runtime::_isTrue(&t);
    return true;
}


void CodeManager::SetRoomOrder(unsigned int** order, unsigned int count) {
    Runtime::SetRoomOrder(order, count);
}

bool CodeManager::Init() {
    return true;
}

bool CodeManager::GetError(const char** err) {
    if(Runtime::GetReturnCause() == Runtime::ReturnCause::ExitError) {
        (*err) = Runtime::GetErrorMessage();
        return true;
    }
    else {
        return false;
    }
}