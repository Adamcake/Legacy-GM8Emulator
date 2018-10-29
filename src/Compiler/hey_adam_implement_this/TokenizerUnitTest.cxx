#include "Tokenizer.hxx"
#include <iostream>

/* This is the best worst thing that has ever lived, top 10 funniest programmer moments right here :POG: */

void GM8Emulator::Compiler::TokenizerUnitTest(std::ostream &out)
{
#if _DEBUG
    goto w;
w:
    const char *_ = "uwu owo whats this";
    out << "Asserting Sample Identifiers '" << _ << "' ..\n";
    TokenList __(_, 18);
    std::string ____;
    for (const auto &___ : __.tokens)
        {
            if (___.type == Token::token_type::Identifier)
                {
                    ____ += ___.value.id;
                    ____ += ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong token type encountered.\n";
                    goto next1;
                }
        }
    if (____.substr(0, ____.size() - 1) == _)
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching result.\n   Source: " << _ << "\n   Parsed: " << ____;
        }
next1:
    std::string _____;
    for (uint8_t _w = 0; _w < static_cast<uint8_t>(KeywordType::None); _w++)
        {
            _____ += KeywordToString(static_cast<KeywordType>(_w));
            _____ += ' ';
        }
    out << "Asserting Keywords '" << _____.substr(0, _____.length() - 1) << "' ..\n";
    std::string ______;
    TokenList ________(_____.c_str(), _____.length());
    for (const auto &_______ : ________.tokens)
        {
            if (_______.type == Token::token_type::Keyword)
                {
                    ______ += KeywordToString(_______.value.key);
                    ______ += ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong token type encountered.\n";
                    goto owoowowoowoWOWOOWOWOWOOWOOWOOOWOWOOOWOOWOWO69;
                }
        }
    if (______ == _____)
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching result.\n   Source: " << _____ << "\n   Parsed: " << ______;
        }
owoowowoowoWOWOOWOWOWOOWOOWOOOWOWOOOWOOWOWO69:
    std::string _________;
    for (uint8_t __w = static_cast<uint8_t>(SeparatorType::ParenLeft); __w < static_cast<uint8_t>(SeparatorType::None); __w++)
        {
            _________ += SeparatorToString(static_cast<SeparatorType>(__w));
            _________ += ' ';
        }
    out << "Asserting Separators '" << _________.substr(0, _________.length() - 1) << "' ..\n";
    std::string __________;
    TokenList ___________(_________.c_str(), _________.length());
    for (const auto &____________ : ___________.tokens)
        {
            if (____________.type == Token::token_type::Separator)
                {
                    __________ += SeparatorToString(____________.value.sep);
                    __________ += ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong token type encountered.\n";
                    goto next2;
                }
        }
    if (__________ == _________)
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching result.\n   Source: " << _________ << "\n   Parsed: " << __________;
        }
next2:
    std::string _____________;
    for (uint8_t ___w = 0; ___w < static_cast<uint8_t>(OperatorType::None); ___w++)
        {
            _____________ += OperatorToString(static_cast<OperatorType>(___w));
            _____________ += ' ';
        }
    out << "Asserting Operators '" << _____________.substr(0, _____________.length() - 1) << "' ..\n";
    std::string _______________;
    TokenList ________________(_____________.c_str(), _____________.length());
    for (const auto &______________ : ________________.tokens)
        {
            if (______________.type == Token::token_type::Operator)
                {
                    _______________ += OperatorToString(______________.value.op);
                    _______________ += ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong token type encountered.\n";
                    goto next3;
                }
        }
    if (_____________ == _______________)
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching result.\n   Source: " << _____________ << "\n   Parsed: " << _______________;
        }
next3:
    const char *_________________ = "5.5.5.... 6...2...9 .7....3.. 4.2...0.. $ $a $A $aa $0d0a";
    out << "Asserting Numbers '" << _________________ << "' ..\n";
    std::string __________________ = "5.550000 6.290000 0.730000 4.200000 0.000000 10.000000 10.000000 170.000000 3338.000000 ";
    TokenList _____________________(_________________, 57);
    std::string ___________________;
    for (const auto &____________________ : _____________________.tokens)
        {
            if (____________________.type == Token::token_type::Real)
                {
                    ___________________ += std::to_string(____________________.value.real);
                    ___________________ += ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong token type encountered.\n";
                    goto next_;
                }
        }

    if (___________________ == __________________)
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching result.\n   Source: " << _________________ << "\n   Parsed:   " << ___________________
                << "\n   Expected: " << __________________ << "\n";
        }
next_:
    const char *______________________ = "begin end then and or not xor";
    out << "Asserting Stupids '" << ______________________ << "' ..\n";
    std::string _______________________;
    TokenList _________________________(______________________, 29);
    for (const auto &________________________ : _________________________.tokens)
        {
            if (________________________.type == Token::token_type::Separator)
                {
                    _______________________ += SeparatorToString(________________________.value.sep);
                    _______________________ += ' ';
                }
            else if (________________________.type == Token::token_type::Operator)
                {
                    _______________________ += OperatorToString(________________________.value.op);
                    _______________________ += ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong token type encountered.\n";
                    goto next4;
                }
        }
    if (_______________________ != "{ } then && || ! ^^ ")
        {
            out << " -> FAILED! Mismatching result.\n   Source: " << ______________________ << "\n   Parsed:   " << _______________________ << "\n   Expected: "
                << "{ } then && || ! ^^\n";
        }
    else
        {
            out << " -> Success!\n";
        }
next4:;

#else
    out << "Hey, you aren't in debug mode! Shoo!\n";
#endif
}
