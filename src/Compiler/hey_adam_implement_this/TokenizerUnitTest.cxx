#include "Tokenizer.hxx"
#include <iostream>

/* This is the best worst thing that has ever lived, top 10 funniest programmer moments right here :POG: */

void GM8Emulator::Compiler::TokenizerUnitTest(std::ostream& out) {
#if _DEBUG
    goto w;
w:
    const char* idut_src = "uwu owo whats this";
    out << "Asserting Sample Identifiers '" << idut_src << "' ..\n";
    TokenList idut(idut_src, 18);
    std::string idut_result;
    for (const auto& token : idut.tokens) {
        if (token.type == Token::token_type::Identifier) {
            idut_result += token.value.id;
            idut_result += ' ';
        }
        else {
            out << " -> FAILED! Wrong token type encountered.\n";
            goto next1;
        }
    }
    if (idut_result.substr(0, idut_result.size() - 1) == idut_src) {
        out << " -> Success!\n";
    }
    else {
        out << " -> FAILED! Mismatching result.\n   Source: " << idut_src << "\n   Parsed: " << idut_result;
    }
next1:
    std::string kwut_src;
    for (uint8_t i = 0; i < static_cast<uint8_t>(KeywordType::None); i++) {
        kwut_src += KeywordToString(static_cast<KeywordType>(i));
        kwut_src += ' ';
    }
    out << "Asserting Keywords '" << kwut_src.substr(0, kwut_src.length() - 1) << "' ..\n";
    std::string kwut_result;
    TokenList kwut(kwut_src.c_str(), kwut_src.length());
    for (const auto& token : kwut.tokens) {
        if (token.type == Token::token_type::Keyword) {
            kwut_result += KeywordToString(token.value.key);
            kwut_result += ' ';
        }
        else {
            out << " -> FAILED! Wrong token type encountered.\n";
            goto owoowowoowoWOWOOWOWOWOOWOOWOOOWOWOOOWOOWOWO69;
        }
    }
    if (kwut_result == kwut_src) {
        out << " -> Success!\n";
    }
    else {
        out << " -> FAILED! Mismatching result.\n   Source: " << kwut_src << "\n   Parsed: " << kwut_result;
    }
owoowowoowoWOWOOWOWOWOOWOOWOOOWOWOOOWOOWOWO69:
    std::string seput_src;
    for (uint8_t i = static_cast<uint8_t>(SeparatorType::ParenLeft); i < static_cast<uint8_t>(SeparatorType::None); i++) {
        seput_src += SeparatorToString(static_cast<SeparatorType>(i));
        seput_src += ' ';
    }
    out << "Asserting Separators '" << seput_src.substr(0, seput_src.length() - 1) << "' ..\n";
    std::string seput_result;
    TokenList seput(seput_src.c_str(), seput_src.length());
    for (const auto& token : seput.tokens) {
        if (token.type == Token::token_type::Separator) {
            seput_result += SeparatorToString(token.value.sep);
            seput_result += ' ';
        }
        else {
            out << " -> FAILED! Wrong token type encountered.\n";
            goto next2;
        }
    }
    if (seput_result == seput_src) {
        out << " -> Success!\n";
    }
    else {
        out << " -> FAILED! Mismatching result.\n   Source: " << seput_src << "\n   Parsed: " << seput_result;
    }
next2:
    std::string oput_src;
    for (uint8_t i = 0; i < static_cast<uint8_t>(OperatorType::None); i++) {
        oput_src += OperatorToString(static_cast<OperatorType>(i));
        oput_src += ' ';
    }
    out << "Asserting Operators '" << oput_src.substr(0, oput_src.length() - 1) << "' ..\n";
    std::string oput_result;
    TokenList oput(oput_src.c_str(), oput_src.length());
    for (const auto& token : oput.tokens) {
        if (token.type == Token::token_type::Operator) {
            oput_result += OperatorToString(token.value.op);
            oput_result += ' ';
        }
        else {
            out << " -> FAILED! Wrong token type encountered.\n";
            goto next3;
        }
    }
    if (oput_src == oput_result) {
        out << " -> Success!\n";
    }
    else {
        out << " -> FAILED! Mismatching result.\n   Source: " << oput_src << "\n   Parsed: " << oput_result;
    }
next3:
    const char* numut_src = "5.5.5.... 6...2...9 .7....3.. 4.2...0.. $ $a $A $aa $0d0a";
    out << "Asserting Numbers '" << numut_src << "' ..\n";
    std::string numut_expect = "5.550000 6.290000 0.730000 4.200000 0.000000 10.000000 10.000000 170.000000 3338.000000 ";
    TokenList numut(numut_src, 57);
    std::string numut_result;
    for (const auto& token : numut.tokens) {
        if (token.type == Token::token_type::Real) {
            numut_result += std::to_string(token.value.real);
            numut_result += ' ';
        }
        else {
            out << " -> FAILED! Wrong token type encountered.\n";
            goto next_;
        }
    }

    if (numut_result == numut_expect) {
        out << " -> Success!\n";
    }
    else {
        out << " -> FAILED! Mismatching result.\n   Source: " << numut_src << "\n   Parsed:   " << numut_result << "\n   Expected: " << numut_expect << "\n";
    }
next_:
    const char* stupidut_src = "begin end then and or not xor";
    out << "Asserting Stupids '" << stupidut_src << "' ..\n";
    std::string stupidut_result;
    TokenList stupidut(stupidut_src, 29);
    for (const auto& token : stupidut.tokens) {
        if (token.type == Token::token_type::Separator) {
            stupidut_result += SeparatorToString(token.value.sep);
            stupidut_result += ' ';
        }
        else if (token.type == Token::token_type::Operator) {
            stupidut_result += OperatorToString(token.value.op);
            stupidut_result += ' ';
        }
        else {
            out << " -> FAILED! Wrong token type encountered.\n";
            goto next4;
        }
    }
    if (stupidut_result != "{ } then && || ! ^^ ") {
        out << " -> FAILED! Mismatching result.\n   Source: " << stupidut_src << "\n   Parsed:   " << stupidut_result << "\n   Expected: "
            << "{ } then && || ! ^^\n";
    }
    else {
        out << " -> Success!\n";
    }
next4:;

#else
    out << "Hey, you aren't in debug mode! Shoo!\n";
#endif
}
