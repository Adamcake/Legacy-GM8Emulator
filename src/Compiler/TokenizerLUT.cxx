#include <pch.h>

#include "Tokenizer.hxx"

/* extern */ const uint8_t GM8Emulator::Compiler::OperatorSeparatorLUT[94] = {
    /* [0] / ASCII 33 '!' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Not),
    /* [1] / ASCII 34 '"' */ 255U,
    /* [2] / ASCII 35 '#' */ 255U,
    /* [3] / ASCII 36 '$' */ 255U,
    /* [4] / ASCII 37 '%' */ 255U,
    /* [5] / ASCII 38 '&' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::BinaryAnd),
    /* [6] / ASCII 39 ''' */ 255U,
    /* [7] / ASCII 40 '(' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::ParenLeft),
    /* [8] / ASCII 41 ')' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::ParenRight),
    /* [9] / ASCII 42 '*' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Multiply),
    /* [10] / ASCII 43 '+' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Add),
    /* [11] / ASCII 44 ',' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::Comma),
    /* [12] / ASCII 45 '-' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Subtract),
    /* [13] / ASCII 46 '.' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::Period),
    /* [14] / ASCII 47 '/' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Divide),
    /* [15] / ASCII 48 '0' */ 255U,
    /* [16] / ASCII 49 '1' */ 255U,
    /* [17] / ASCII 50 '2' */ 255U,
    /* [18] / ASCII 51 '3' */ 255U,
    /* [19] / ASCII 52 '4' */ 255U,
    /* [20] / ASCII 53 '5' */ 255U,
    /* [21] / ASCII 54 '6' */ 255U,
    /* [22] / ASCII 55 '7' */ 255U,
    /* [23] / ASCII 56 '8' */ 255U,
    /* [24] / ASCII 57 '9' */ 255U,
    /* [25] / ASCII 58 ':' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::Colon),
    /* [26] / ASCII 59 ';' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::Semicolon),
    /* [27] / ASCII 60 '<' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::LessThan),
    /* [28] / ASCII 61 '=' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Assign),
    /* [29] / ASCII 62 '>' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::GreaterThan),
    /* [30] / ASCII 63 '?' */ 255U,
    /* [31] / ASCII 64 '@' */ 255U,
    /* [32] / ASCII 65 'A' */ 255U,
    /* [33] / ASCII 66 'B' */ 255U,
    /* [34] / ASCII 67 'C' */ 255U,
    /* [35] / ASCII 68 'D' */ 255U,
    /* [36] / ASCII 69 'E' */ 255U,
    /* [37] / ASCII 70 'F' */ 255U,
    /* [38] / ASCII 71 'G' */ 255U,
    /* [39] / ASCII 72 'H' */ 255U,
    /* [40] / ASCII 73 'I' */ 255U,
    /* [41] / ASCII 74 'J' */ 255U,
    /* [42] / ASCII 75 'K' */ 255U,
    /* [43] / ASCII 76 'L' */ 255U,
    /* [44] / ASCII 77 'M' */ 255U,
    /* [45] / ASCII 78 'N' */ 255U,
    /* [46] / ASCII 79 'O' */ 255U,
    /* [47] / ASCII 80 'P' */ 255U,
    /* [48] / ASCII 81 'Q' */ 255U,
    /* [49] / ASCII 82 'R' */ 255U,
    /* [50] / ASCII 83 'S' */ 255U,
    /* [51] / ASCII 84 'T' */ 255U,
    /* [52] / ASCII 85 'U' */ 255U,
    /* [53] / ASCII 86 'V' */ 255U,
    /* [54] / ASCII 87 'W' */ 255U,
    /* [55] / ASCII 88 'X' */ 255U,
    /* [56] / ASCII 89 'Y' */ 255U,
    /* [57] / ASCII 90 'Z' */ 255U,
    /* [58] / ASCII 91 '[' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::SquareBracketLeft),
    /* [59] / ASCII 92 '\' */ 255U,
    /* [60] / ASCII 93 ']' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::SquareBracketRight),
    /* [61] / ASCII 94 '^' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::BinaryXor),
    /* [62] / ASCII 95 '_' */ 255U,
    /* [63] / ASCII 96 '`' */ 255U,
    /* [64] / ASCII 97 'a' */ 255U,
    /* [65] / ASCII 98 'b' */ 255U,
    /* [66] / ASCII 99 'c' */ 255U,
    /* [67] / ASCII 100 'd' */ 255U,
    /* [68] / ASCII 101 'e' */ 255U,
    /* [69] / ASCII 102 'f' */ 255U,
    /* [70] / ASCII 103 'g' */ 255U,
    /* [71] / ASCII 104 'h' */ 255U,
    /* [72] / ASCII 105 'i' */ 255U,
    /* [73] / ASCII 106 'j' */ 255U,
    /* [74] / ASCII 107 'k' */ 255U,
    /* [75] / ASCII 108 'l' */ 255U,
    /* [76] / ASCII 109 'm' */ 255U,
    /* [77] / ASCII 110 'n' */ 255U,
    /* [78] / ASCII 111 'o' */ 255U,
    /* [79] / ASCII 112 'p' */ 255U,
    /* [80] / ASCII 113 'q' */ 255U,
    /* [81] / ASCII 114 'r' */ 255U,
    /* [82] / ASCII 115 's' */ 255U,
    /* [83] / ASCII 116 't' */ 255U,
    /* [84] / ASCII 117 'u' */ 255U,
    /* [85] / ASCII 118 'v' */ 255U,
    /* [86] / ASCII 119 'w' */ 255U,
    /* [87] / ASCII 120 'x' */ 255U,
    /* [88] / ASCII 121 'y' */ 255U,
    /* [89] / ASCII 122 'z' */ 255U,
    /* [90] / ASCII 123 '{' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::BraceLeft),
    /* [91] / ASCII 124 '|' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::BinaryOr),
    /* [92] / ASCII 125 '}' */ static_cast<uint8_t>(GM8Emulator::Compiler::SeparatorType::BraceRight),
    /* [93] / ASCII 126 '~' */ static_cast<uint8_t>(GM8Emulator::Compiler::OperatorType::Complement)

    // Big lookup table for any operator or separator. 255U (0xFF) means no match.
};

const GM8Emulator::Compiler::OperatorType GM8Emulator::Compiler::OperatorComboLUT[12] = {
    /* + -> += */ GM8Emulator::Compiler::OperatorType::AssignAdd,
    /* - -> -= */ GM8Emulator::Compiler::OperatorType::AssignSubtract,
    /* * -> *= */ GM8Emulator::Compiler::OperatorType::AssignMultiply,
    /* / -> /= */ GM8Emulator::Compiler::OperatorType::AssignDivide,
    /* & -> &= */ GM8Emulator::Compiler::OperatorType::AssignBinaryAnd,
    /* | -> |= */ GM8Emulator::Compiler::OperatorType::AssignBinaryOr,
    /* ^ -> ^= */ GM8Emulator::Compiler::OperatorType::AssignBinaryXor,
    /* = -> == */ GM8Emulator::Compiler::OperatorType::Equal,
    /* ! -> != */ GM8Emulator::Compiler::OperatorType::NotEqual,
    /* < -> <= */ GM8Emulator::Compiler::OperatorType::LessThanOrEquals,
    /* > -> >= */ GM8Emulator::Compiler::OperatorType::GreaterThanOrEquals,
    /* Not found (clamp to this being the max) */ GM8Emulator::Compiler::OperatorType::None

    // Lookup table matching the order of OperatorType. access with [min(n, 11U)].
};

// ([A-z]+),[ ]*\/\/ (.+)\n
const char* GM8Emulator::Compiler::OperatorToString(GM8Emulator::Compiler::OperatorType op) {
    switch (op) {
        case GM8Emulator::Compiler::OperatorType::Add:
            return "+";
        case GM8Emulator::Compiler::OperatorType::Subtract:
            return "-";
        case GM8Emulator::Compiler::OperatorType::Multiply:
            return "*";
        case GM8Emulator::Compiler::OperatorType::Divide:
            return "/";
        case GM8Emulator::Compiler::OperatorType::DivideAndFloor:
            return "div";
        case GM8Emulator::Compiler::OperatorType::Modulo:
            return "mod";
        case GM8Emulator::Compiler::OperatorType::Assign:
            return "=";
        case GM8Emulator::Compiler::OperatorType::AssignAdd:
            return "+=";
        case GM8Emulator::Compiler::OperatorType::AssignSubtract:
            return "-=";
        case GM8Emulator::Compiler::OperatorType::AssignMultiply:
            return "*=";
        case GM8Emulator::Compiler::OperatorType::AssignDivide:
            return "/=";
        case GM8Emulator::Compiler::OperatorType::AssignBinaryAnd:
            return "&=";
        case GM8Emulator::Compiler::OperatorType::AssignBinaryOr:
            return "|=";
        case GM8Emulator::Compiler::OperatorType::AssignBinaryXor:
            return "^=";
        case GM8Emulator::Compiler::OperatorType::Equal:
            return "==";
        case GM8Emulator::Compiler::OperatorType::NotEqual:
            return "!=";
        case GM8Emulator::Compiler::OperatorType::And:
            return "&&";
        case GM8Emulator::Compiler::OperatorType::Or:
            return "||";
        case GM8Emulator::Compiler::OperatorType::Xor:
            return "^^";
        case GM8Emulator::Compiler::OperatorType::LessThan:
            return "<";
        case GM8Emulator::Compiler::OperatorType::LessThanOrEquals:
            return "<=";
        case GM8Emulator::Compiler::OperatorType::GreaterThan:
            return ">";
        case GM8Emulator::Compiler::OperatorType::GreaterThanOrEquals:
            return ">=";
        case GM8Emulator::Compiler::OperatorType::BinaryAnd:
            return "&";
        case GM8Emulator::Compiler::OperatorType::BinaryOr:
            return "|";
        case GM8Emulator::Compiler::OperatorType::BinaryXor:
            return "^";
        case GM8Emulator::Compiler::OperatorType::BinaryShiftLeft:
            return "<<";
        case GM8Emulator::Compiler::OperatorType::BinaryShiftRight:
            return ">>";
        case GM8Emulator::Compiler::OperatorType::Not:
            return "!";
        case GM8Emulator::Compiler::OperatorType::Complement:
            return "~";
        case GM8Emulator::Compiler::OperatorType::None:
        default:
            return "/* malformed operator */";
    }
}

const char* GM8Emulator::Compiler::SeparatorToString(GM8Emulator::Compiler::SeparatorType sep) {
    switch (sep) {
        case GM8Emulator::Compiler::SeparatorType::ParenLeft:
            return "(";
        case GM8Emulator::Compiler::SeparatorType::ParenRight:
            return ")";
        case GM8Emulator::Compiler::SeparatorType::BraceLeft:
            return "{";
        case GM8Emulator::Compiler::SeparatorType::BraceRight:
            return "}";
        case GM8Emulator::Compiler::SeparatorType::SquareBracketLeft:
            return "[";
        case GM8Emulator::Compiler::SeparatorType::SquareBracketRight:
            return "]";
        case GM8Emulator::Compiler::SeparatorType::Semicolon:
            return ";";
        case GM8Emulator::Compiler::SeparatorType::Colon:
            return ":";
        case GM8Emulator::Compiler::SeparatorType::Comma:
            return ",";
        case GM8Emulator::Compiler::SeparatorType::Period:
            return ".";
        case GM8Emulator::Compiler::SeparatorType::PascalThen:
            return "then";
        default:
            return "/* malformed separator */";
    }
}

const char* GM8Emulator::Compiler::KeywordToString(GM8Emulator::Compiler::KeywordType key) {
    switch (key) {
        case GM8Emulator::Compiler::KeywordType::Var:
            return "var";
        case GM8Emulator::Compiler::KeywordType::If:
            return "if";
        case GM8Emulator::Compiler::KeywordType::Else:
            return "else";
        case GM8Emulator::Compiler::KeywordType::With:
            return "with";
        case GM8Emulator::Compiler::KeywordType::Repeat:
            return "repeat";
        case GM8Emulator::Compiler::KeywordType::Do:
            return "do";
        case GM8Emulator::Compiler::KeywordType::Until:
            return "until";
        case GM8Emulator::Compiler::KeywordType::While:
            return "while";
        case GM8Emulator::Compiler::KeywordType::For:
            return "for";
        case GM8Emulator::Compiler::KeywordType::Switch:
            return "switch";
        case GM8Emulator::Compiler::KeywordType::Case:
            return "case";
        case GM8Emulator::Compiler::KeywordType::Default:
            return "default";
        case GM8Emulator::Compiler::KeywordType::Break:
            return "break";
        case GM8Emulator::Compiler::KeywordType::Continue:
            return "continue";
        case GM8Emulator::Compiler::KeywordType::Exit:
            return "exit";
        case GM8Emulator::Compiler::KeywordType::Return:
            return "return";
        case GM8Emulator::Compiler::KeywordType::None:
        default:
            return "/* malformed keyword */";
    }
}