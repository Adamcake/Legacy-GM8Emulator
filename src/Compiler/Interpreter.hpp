#ifndef _A_INTERPRETER_HPP_
#define _A_INTERPRETER_HPP_
#include <pch.h>
#include "Tokenizer.hxx"
class CRActionList;
class CRExpression;

namespace GM8Emulator {
    namespace Compiler {
        bool Init();

        bool Interpret(const TokenList& list, CRActionList* output);
        bool InterpretExpression(const TokenList& list, CRExpression* output, unsigned int* pos = nullptr);
    }
}

#endif