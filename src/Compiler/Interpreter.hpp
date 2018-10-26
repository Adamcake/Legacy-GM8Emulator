#ifndef _A_INTERPRETER_HPP_
#define _A_INTERPRETER_HPP_
#include <pch.h>
#include "Tokenizer.hxx"
class CRActionList;
class CRExpression;

namespace GM8Emulator {
    namespace Compiler {
        bool Interpret(TokenList list, CRActionList* output);
        bool InterpretExpression(TokenList list, CRExpression* output);
    }
}

#endif