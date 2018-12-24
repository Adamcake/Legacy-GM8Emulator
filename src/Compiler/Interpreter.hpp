#pragma once

#include "Tokenizer.hxx"

class CRActionList;
class CRExpression;
struct GlobalValues;

namespace GM8Emulator {
    namespace Compiler {
        bool Init(GlobalValues* globals);

        bool Interpret(const TokenList& list, CRActionList* output);
        bool InterpretExpression(const TokenList& list, CRExpression* output, unsigned int* pos = nullptr, char precedence = 5, char lowestAllowedPrec = 0);

        void FlushLocals();
    };
};
