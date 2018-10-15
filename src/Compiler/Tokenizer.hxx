#ifndef TOKENIZER_HXX
#define TOKENIZER_HXX

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace GM8Emulator {
    namespace Compiler {
        struct Token;
        enum class TokenType : uint8_t;
        enum class KeywordType : uint8_t;
        enum class OperatorType : uint8_t;
        enum class SeparatorType : uint8_t;
        typedef ::std::vector<Token> TokenList;
        typedef ::std::variant<KeywordType, OperatorType, SeparatorType, double, ::std::string>
            TokenValue;
        typedef ::std::map<::std::string, TokenList> MacroList;

        enum class TokenType : uint8_t {
            Identifier,
            Keyword,
            LiteralReal,
            LiteralString,
            Operator,
            Separator,
            None
        };

        enum class KeywordType : uint8_t {
            Var,      // var
            If,       // if
            Else,     // else
            With,     // with
            Repeat,   // repeat
            Do,       // do
            Until,    // until
            While,    // while
            For,      // for
            Switch,   // switch
            Case,     // case
            Default,  // default
            Break,    // break
            Continue, // continue
            Return,   // return
            None
        };

        enum class OperatorType : uint8_t {
            Add,                 // + (Can be unary positive (useless))
            Subtract,            // - (Can be unary negative (flips sign))
            Multiply,            // *
            Divide,              // /
            DivideAndFloor,      // div (Thank you, Mark Overmars!)
            Modulo,              // mod
            Assign,              // = (Can mean equal if read in an expression)
            AssignAdd,           // +=
            AssignSubtract,      // -=
            AssignMultiply,      // *=
            AssignDivide,        // /=
            AssignBinaryAnd,     // &=
            AssignBinaryOr,      // |=
            AssignBinaryXor,     // ^=
            Equal,               // ==
            NotEqual,            // !=
            And,                 // &&
            Or,                  // ||
            Xor,                 // ^^
            LessThan,            // <
            LessThanOrEquals,    // <=
            GreaterThan,         // >
            GreaterThanOrEquals, // >=
            BinaryAnd,           // &
            BinaryOr,            // |
            BinaryXor,           // ^
            BinaryShiftLeft,     // <<
            BinaryShiftRight,    // >>
            Not,                 // !
            Complement,          // ~
            None
        };

        enum class SeparatorType : uint8_t {
            ParenLeft,          // (
            ParenRight,         // )
            BraceLeft,          // {
            BraceRight,         // }
            SquareBracketLeft,  // [
            SquareBracketRight, // ]
            Semicolon,          // ;
            Colon,              // :
            Comma,              // ,
            Period,             // .
            PascalThen,         // then (Useless relic, does nothing)
            None
        };

        struct Token {
            TokenType Type;
            TokenValue Value;

            Token() noexcept : Type(TokenType::None), Value() {}

            Token(double literalReal) noexcept : Type(TokenType::LiteralReal), Value(literalReal) {}

            Token(::std::string literalString) noexcept
                : Type(TokenType::LiteralString), Value(literalString)
            {
            }

            ::std::string toGMLEquivalent() noexcept;
            ::std::string toTypedIdentifier() noexcept;
        };

        constexpr size_t TokenizerBufferSize = 256;
        TokenList Tokenize(::std::string &gml, MacroList *macros = nullptr);
        void UnitTest(::std::ostream &out);
    }; // namespace Compiler
};     // namespace GM8Emulator

#endif // !TOKENIZER_HXX
