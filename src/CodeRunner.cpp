#include "CodeRunner.hpp"
#include "AssetManager.hpp"
#include "CREnums.hpp"
#include "CRGMLType.hpp"
#include "CodeActionManager.hpp"
#include "Compiler/CRRuntime.hpp"
#include "Compiler/Compiled.hpp"
#include "Compiler/Interpreter.hpp"
#include "Compiler/Tokenizer.hxx"
#include "InstanceList.hpp"
#include "RNG.hpp"
#include <cstring>
#include <stdexcept>

// Internal code object
struct CRCodeObject {
    char* _code;
    GM8Emulator::Compiler::TokenList _tokenized;
    bool question;
    CRActionList _actions;
    CRExpression _expression;
    CRCodeObject(const char* c, unsigned int l, bool q) : question(q) {
        _code = ( char* )malloc(l);
        memcpy(_code, c, l);
        _tokenized = GM8Emulator::Compiler::TokenList(_code, l);
    }
};
std::vector<CRCodeObject> _codeObjects;

// Global game value settings
GlobalValues* _crGlobalValues;

bool CodeManager::Init(GlobalValues* globals) {
    _crGlobalValues = globals;
    RNG::Randomize();
    return GM8Emulator::Compiler::Init(globals);
}

void CodeManager::Finalize() {
    for (CRCodeObject& obj : _codeObjects) {
        if (obj.question) {
            obj._expression.Finalize();
        }
        else {
            obj._actions.Finalize();
        }
        free(obj._code);
    }
    Runtime::Finalize();
}

CodeObject CodeManager::Register(const char* code, unsigned int len) {
    unsigned int ix = ( unsigned int )_codeObjects.size();
    _codeObjects.push_back(CRCodeObject(code, len, false));

    return ix;
}

CodeObject CodeManager::RegisterQuestion(const char* code, unsigned int len) {
    unsigned int ix = ( unsigned int )_codeObjects.size();
    _codeObjects.push_back(CRCodeObject(code, len, true));

    return ix;
}

bool CodeManager::Compile(CodeObject object) {
    try {
        if (_codeObjects[object].question) {
            if (!GM8Emulator::Compiler::InterpretExpression(_codeObjects[object]._tokenized, &_codeObjects[object]._expression)) return false;
        }
        else {
            if (!GM8Emulator::Compiler::Interpret(_codeObjects[object]._tokenized, &_codeObjects[object]._actions)) return false;
        }
        GM8Emulator::Compiler::FlushLocals();
        return true;
    }
    catch (const std::runtime_error&) {
        return false;
    }
}

bool CodeManager::Run(CodeObject code, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, unsigned int argc, GMLType* argv) {
    return Runtime::Execute(_codeObjects[code]._actions, self, other, ev, sub, asObjId, argc, argv);
}

bool CodeManager::Query(CodeObject code, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, bool* response, unsigned int argc, GMLType* argv) {
    GMLType t;
    if (!Runtime::EvalExpression(_codeObjects[code]._expression, self, other, ev, sub, asObjId, &t, argc, argv)) return false;
    (*response) = Runtime::_isTrue(&t);
    return true;
}

bool CodeManager::Query(CodeObject code, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, GMLType* response) {
    GMLType t;
    if (!Runtime::EvalExpression(_codeObjects[code]._expression, self, other, ev, sub, asObjId, &t)) return false;
    (*response) = t;
    return true;
}


void CodeManager::SetRoomOrder(unsigned int** order, unsigned int count) { Runtime::SetRoomOrder(order, count); }

bool CodeManager::Init() { return true; }

bool CodeManager::GetError(const char** err) {
    if (Runtime::GetReturnCause() == Runtime::ReturnCause::ExitError) {
        (*err) = Runtime::GetErrorMessage();
        return true;
    }
    else {
        return false;
    }
}
