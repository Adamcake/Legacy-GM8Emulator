#ifndef CONSTANTS_HXX
#define CONSTANTS_HXX

namespace GM8Emulator {
    namespace Compiler {
        namespace Constants {
            constexpr const char SingleQuote = '\'';
            constexpr const char DoubleQuote = '"';

            // Keywords
            constexpr const char *KVar = "var";
            constexpr const char *KIf = "if";
            constexpr const char *KElse = "else";
            constexpr const char *KWith = "with";
            constexpr const char *KRepeat = "repeat";
            constexpr const char *KDo = "do";
            constexpr const char *KUntil = "until";
            constexpr const char *KWhile = "while";
            constexpr const char *KFor = "for";
            constexpr const char *KSwitch = "switch";
            constexpr const char *KCase = "case";
            constexpr const char *KDefault = "default";
            constexpr const char *KBreak = "break";
            constexpr const char *KContinue = "continue";
            constexpr const char *KReturn = "return";

            // Operators
            constexpr const char *OPAdd = "+";
            constexpr const char *OPSubtract = "-";
            constexpr const char *OPMultiply = "*";
            constexpr const char *OPDivide = "/";
            constexpr const char *OPDivideAndFloor = "div";
            constexpr const char *OPModulo = "mod";
            constexpr const char *OPAssign = "=";
            constexpr const char *OPAssignAdd = "+=";
            constexpr const char *OPAssignSubtract = "-=";
            constexpr const char *OPAssignMultiply = "*=";
            constexpr const char *OPAssignDivide = "/=";
            constexpr const char *OPAssignBinaryAnd = "&=";
            constexpr const char *OPAssignBinaryOr = "|=";
            constexpr const char *OPAssignBinaryXor = "^=";
            constexpr const char *OPEqual = "==";
            constexpr const char *OPNotEqual = "!=";
            constexpr const char *OPAnd = "&&";
            constexpr const char *OPAndStr = "and";
            constexpr const char *OPOr = "||";
            constexpr const char *OPOrStr = "or";
            constexpr const char *OPXor = "^^";
            constexpr const char *OPXorStr = "xor";
            constexpr const char *OPLessThan = "<";
            constexpr const char *OPLessThanOrEquals = "<=";
            constexpr const char *OPGreaterThan = ">";
            constexpr const char *OPGreaterThanOrEquals = ">=";
            constexpr const char *OPBinaryAnd = "&";
            constexpr const char *OPBinaryOr = "|";
            constexpr const char *OPBinaryXor = "^";
            constexpr const char *OPBinaryShiftLeft = "<<";
            constexpr const char *OPBinaryShiftRight = ">>";
            constexpr const char *OPNot = "!";
            constexpr const char *OPNotStr = "not";
            constexpr const char *OPComplement = "~";

            // Separators
            constexpr const char *SepParenLeft = "(";
            constexpr const char *SepParenRight = ")";
            constexpr const char *SepBraceLeft = "{";
            constexpr const char *SepBraceRight = "}";
            constexpr const char *SepSquareBracketLeft = "[";
            constexpr const char *SepSquareBracketRight = "]";
            constexpr const char *SepSemicolon = ";";
            constexpr const char *SepColon = ":";
            constexpr const char *SepComma = ",";
            constexpr const char *SepPeriod = ".";
            constexpr const char *SepPascalThen = "then";
            constexpr const char *SepPascalBegin = "begin";
            constexpr const char *SepPascalEnd = "end";
        }; // namespace Constants
    };     // namespace Compiler
};         // namespace GM8Emulator

#endif // !CONSTANTS_HXX
