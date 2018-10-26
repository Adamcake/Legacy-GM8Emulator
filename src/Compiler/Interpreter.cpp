#include <pch.h>
#include "Interpreter.hpp"
#include "Compiled.hpp"

bool _InterpretLine(GM8Emulator::Compiler::TokenList list, CRAction** output, unsigned int* pos) {
    return false;
}

bool GM8Emulator::Compiler::Interpret(TokenList list, CRActionList* output) {
    unsigned int pos = 0;
    CRAction* action;
    while(pos < list.size()) {
        if(!_InterpretLine(list, &action, &pos)) return false;
        output->Append(action);
    }
    return true;
}

bool GM8Emulator::Compiler::InterpretExpression(TokenList list, CRExpression* output) {
    // todo
    return false;
}
