#pragma once

#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

namespace GM8Emulator {
    namespace Compiler {
        enum class KeywordType : uint8_t {
            /* Keyword 'var' */
            Var,
            /* Keyword 'if' */
            If,
            /* Keyword 'else' */
            Else,
            /* Keyword 'with' */
            With,
            /* Keyword 'repeat' */
            Repeat,
            /* Keyword 'do' */
            Do,
            /* Keyword 'until' */
            Until,
            /* Keyword 'while' */
            While,
            /* Keyword 'for' */
            For,
            /* Keyword 'switch' */
            Switch,
            /* Keyword 'case' */
            Case,
            /* Keyword 'default' */
            Default,
            /* Keyword 'break' */
            Break,
            /* Keyword 'continue' */
            Continue,
            /* Keyword 'return' */
            Return,
            /* Keyword 'exit' */
            Exit,
            /* Default value (empty) */
            None
        };

        enum class OperatorType : uint8_t {
            /* Mathematical or Unary Operator '+' */
            Add,
            /* Mathematical or Unary Operator '-' */
            Subtract,
            /* Mathematical Operator '*' */
            Multiply,
            /* Mathematical Operator '/' */
            Divide,
            /* Binary Operator '&' */
            BinaryAnd,
            /* Binary Operator '|' */
            BinaryOr,
            /* Binary Operator '^' */
            BinaryXor,
            /* Assignment Operator '=' (Means == if read in an expression!) */
            Assign,
            /* Unary Operator '!' */
            Not,
            /* Boolean Operator '<' */
            LessThan,
            /* Boolean Operator '>' */
            GreaterThan,
            /* Assignment Operator '+=' */
            AssignAdd,
            /* Assignment Operator '-=' */
            AssignSubtract,
            /* Assignment Operator '*=' */
            AssignMultiply,
            /* Assignment Operator '/=' */
            AssignDivide,
            /* Binary Assignment Operator '&=' */
            AssignBinaryAnd,
            /* Binary Assignment Operator '|=' */
            AssignBinaryOr,
            /* Binary Assignment Operator '^=' */
            AssignBinaryXor,
            /* Boolean Operator '==' */
            Equal,
            /* Boolean Operator '!=' */
            NotEqual,
            /* Boolean Operator '<=' */
            LessThanOrEquals,
            /* Boolean Operator '>=' */
            GreaterThanOrEquals,
            /* Mathematical Operator 'mod' */
            Modulo,
            /* Boolean Operator '&&' */
            And,
            /* Boolean Operator '||' */
            Or,
            /* Boolean Operator '^^' */
            Xor,
            /* Binary Operator '<<' */
            BinaryShiftLeft,
            /* Binary Operator '>>' */
            BinaryShiftRight,
            /* Unary Operator '~' */
            Complement,
            /* Mathematical Operator 'div' (Thank you, Mark Overmars!) */
            DivideAndFloor,
            /* Default value (empty) */
            None
        };

        enum class SeparatorType : uint8_t {
            /* Separator '(' */
            ParenLeft = 128U,
            /* Separator ')' */
            ParenRight = 129U,
            /* Separator '{' */
            BraceLeft = 130U,
            /* Separator '}' */
            BraceRight = 131U,
            /* Separator '[' */
            SquareBracketLeft = 132U,
            /* Separator ']' */
            SquareBracketRight = 133U,
            /* Separator ';' */
            Semicolon = 134U,
            /* Separator ':' */
            Colon = 135U,
            /* Separator ',' */
            Comma = 136U,
            /* Separator '.' */
            Period = 137U,
            /* Separator 'then' (Useless relic, does nothing) */
            PascalThen = 138U,
            /* Default value (empty) */
            None = 139U
        };

        struct Token {
            /* Defines what type to read from the value union. */
            enum class token_type : uint8_t { Identifier, Keyword, Real, String, Operator, Separator, None } type;

            /* Value of the token, read Token.type to know which one to read. */
            union value_type {
                value_type() noexcept {}
                value_type(double r) noexcept : real(r) {}
                value_type(KeywordType k) noexcept : key(k) {}
                value_type(OperatorType o) noexcept : op(o) {}
                value_type(SeparatorType s) noexcept : sep(s) {}
                value_type(std::string_view svw) : str(svw) {}
                value_type(std::string_view::const_pointer&& ptr, std::string_view::size_type&& size) noexcept : str(ptr, size) {}

                double real;
                KeywordType key;
                OperatorType op;
                SeparatorType sep;
                std::string_view id;  // alias for str
                std::string_view str;
            } value;

            Token() noexcept : type(token_type::None) {}
            Token(double real) noexcept : type(token_type::Real), value(real) {}
            Token(KeywordType key) noexcept : type(token_type::Keyword), value(key) {}
            Token(OperatorType op) noexcept : type(token_type::Operator), value(op) {}
            Token(SeparatorType sep) noexcept : type(token_type::Separator), value(sep) {}
            Token(std::string_view const& svw, bool is_str_literal = false) : value(svw) {
                if (is_str_literal)
                    type = token_type::String;
                else
                    type = token_type::Identifier;
            }
            Token(std::string_view::const_pointer&& ptr, std::string_view::size_type&& size, bool is_str_literal = false) noexcept
                : value(static_cast<std::string_view::const_pointer&&>(ptr), static_cast<std::string_view::size_type&&>(size)) {
                if (is_str_literal)
                    type = token_type::String;
                else
                    type = token_type::Identifier;
            }
        };

        struct TokenList {
            /* A copy of the source code that was parsed, if necessary to store. Referenced by strings. */
            char* source;
            /* The list of interpreted tokens. */
            std::vector<Token> tokens;

            void ParseGML(const char* gml, const size_t& len) noexcept;
            TokenList(const char* gml, const size_t& len) noexcept { ParseGML(gml, len); }
            TokenList() noexcept { source = nullptr; }
        };

        extern const uint8_t OperatorSeparatorLUT[94];
        extern const OperatorType OperatorComboLUT[12];

        const char* OperatorToString(OperatorType op);
        const char* SeparatorToString(SeparatorType sep);
        const char* KeywordToString(KeywordType key);

        void TokenizerUnitTest(std::ostream& out);
    };
};
