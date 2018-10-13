#include "Tokenizer.hxx"

#include "Constants.hxx"
#include <sstream>

constexpr bool isASCII(char &c) noexcept { return c <= '~'; }

constexpr bool isNothing(char &c) noexcept { return c <= ' '; }

constexpr bool isNumeric(char &c) noexcept { return c >= '0' && c <= '9'; }

constexpr bool isQuote(char &c) noexcept { return c == '\'' || c == '"'; }

constexpr bool isNumericOrDot(char &c) noexcept { return isNumeric(c) || c == '.'; }

constexpr bool isAlpha(char &c) noexcept
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr bool isAlphaOrUnder(char &c) noexcept { return isAlpha(c) || c == '_'; }

constexpr bool isAlphanumericOrUnder(char &c) noexcept { return isAlphaOrUnder(c) || isNumeric(c); }

void ExcUnrecognizedOperatorCombo(char &opchr1, char &opchr2, std::string::iterator begin,
                                  std::string::iterator at)
{
    std::ostringstream err;
    err << "Unrecognized operator " << opchr1 << opchr2 << " at position "
        << static_cast<size_t>(at - begin);

    throw std::runtime_error(err.str());
}

GM8Emulator::Compiler::TokenList GM8Emulator::Compiler::Tokenize(::std::string &gml,
                                                                 MacroList *macros)
{
    TokenList tokenList;

    /* If you haven't used std::string iterators before,
    begin() is the first character, end() is the null terminator.
    std::string::iterator does not let you increment past the end. */

    ::std::string buffer;
    ::std::string::iterator i = gml.begin();
    ::std::string::iterator end = gml.end();

    buffer.reserve(TokenizerBufferSize);

    while (i != end) {
        // Whitespace
        if (isNothing(*i)) i++;

        // Constant / Identifier / Keyword / Operator
        else if (isAlphaOrUnder(*i)) {
            for (;;) {
                if (i == end)
                    break; // EOF - Stop reading!

                else if (isAlphanumericOrUnder(*i))
                    buffer += *i++; // Read any valid character into buffer

                else if (isASCII(*i))
                    break; // Invalid character in ID - Stop reading!

                else {
                    buffer += *i; // For logging!

                    ::std::ostringstream err;
                    err << "Unrecognized character " << *i << " in real literal " << buffer
                        << " at position " << static_cast<size_t>(i - gml.begin());
                    throw ::std::runtime_error(err.str());
                }
            }

            Token newToken;

            switch (buffer.size()) {
                case 2:
                    // 2 Character Keywords & OPWords
                    if (buffer == Constants::OPOrStr) {
                        newToken.Type = TokenType::Operator;
                        newToken.Value = OperatorType::Or;
                    }
                    else if (buffer == Constants::KIf) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::If;
                    }
                    else if (buffer == Constants::KDo) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Do;
                    }
                    break;

                case 3:
                    // 3 Character Keywords & OPWords
                    if (buffer == Constants::OPAndStr) {
                        newToken.Type = TokenType::Operator;
                        newToken.Value = OperatorType::And;
                    }
                    else if (buffer == Constants::OPNotStr) {
                        newToken.Type = TokenType::Operator;
                        newToken.Value = OperatorType::Not;
                    }
                    else if (buffer == Constants::OPXorStr) {
                        newToken.Type = TokenType::Operator;
                        newToken.Value = OperatorType::Xor;
                    }
                    else if (buffer == Constants::OPDivideAndFloor) {
                        newToken.Type = TokenType::Operator;
                        newToken.Value = OperatorType::DivideAndFloor;
                    }
                    else if (buffer == Constants::OPModulo) {
                        newToken.Type = TokenType::Operator;
                        newToken.Value = OperatorType::Modulo;
                    }
                    else if (buffer == Constants::KVar) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Var;
                    }
                    else if (buffer == Constants::KFor) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::For;
                    }
                    else if (buffer == Constants::SepPascalEnd) {
                        newToken.Type = TokenType::Separator;
                        newToken.Value = SeparatorType::BraceRight;
                    }
                    break;

                case 4:
                    // 4 Character Keywords
                    if (buffer == Constants::KCase) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Case;
                    }
                    else if (buffer == Constants::KWith) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::With;
                    }
                    else if (buffer == Constants::KElse) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Else;
                    }
                    else if (buffer == Constants::SepPascalThen) {
                        newToken.Type = TokenType::Separator;
                        newToken.Value = SeparatorType::PascalThen;
                    }
                    break;

                case 5:
                    // 5 Character Keywords
                    if (buffer == Constants::KWhile) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::While;
                    }
                    else if (buffer == Constants::KUntil) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Until;
                    }
                    else if (buffer == Constants::KBreak) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Break;
                    }
                    else if (buffer == Constants::SepPascalBegin) {
                        newToken.Type = TokenType::Separator;
                        newToken.Value = SeparatorType::BraceLeft;
                    }
                    break;

                case 6:
                    // 6 Character Keywords
                    if (buffer == Constants::KRepeat) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Repeat;
                    }
                    else if (buffer == Constants::KReturn) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Return;
                    }
                    else if (buffer == Constants::KSwitch) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Switch;
                    }
                    break;

                case 7:
                    // 7 Character Keywords
                    if (buffer == Constants::KDefault) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Default;
                    }
                    break;

                case 8:
                    // 8 Character Keywords
                    if (buffer == Constants::KContinue) {
                        newToken.Type = TokenType::Keyword;
                        newToken.Value = KeywordType::Continue;
                    }
                    break;

                default:
                    break;
            }

            if (macros != nullptr && newToken.Type == TokenType::None) {
                bool match = false;

                for (auto &kv : *macros) {
                    if (buffer == kv.first) {
                        match = true;

                        for (auto &tok : kv.second) {
                            tokenList.push_back(tok);
                        }

                        break;
                    }
                }

                if (match) {
                    buffer.clear();
                    continue;
                }
            }

            newToken.Type = TokenType::Identifier;
            newToken.Value = buffer;
            tokenList.push_back(newToken);
            buffer.clear();
        }

        // Literal Real
        else if (isNumericOrDot(*i)) {
            if (*i == '.') {
                if (i + 1 != end) {
                    /* If the . is followed by a non-number,
                        it's perceived as a separator, not a decimal. */
                    if (!isNumeric(*(i + 1))) {
                        Token dotOP;
                        dotOP.Type = TokenType::Separator;
                        dotOP.Value = SeparatorType::Period;
                        tokenList.push_back(dotOP);
                        i++; // Next iteration it'll catch what's next to the .
                        continue;
                    }
                }
            }

            ::std::string::iterator pStart = i; // Error Handling

            for (;;) {
                if (i == end)
                    break; // EOF - Stop reading!

                else if (isNumericOrDot(*i))
                    buffer += *i++; // Append any valid chars

                else if (isASCII(*i))
                    break; // Invalid char in real - Stop reading!

                else {
                    // bad
                }
            }

            /* In a number, every dot after the first one is
            ignored. A number can't begin with '..' - this is
            checked before entering the loop. For example, '..1' is
            perceived as the dot operator, followed by the literal
            0.1 - We make a copy with the stupid extra dots ignored,
            and pass it to std::stod. Thank you Mark Overmars! We
            appreciate GML, the pinnacle of programming language
            design! THESE ARE ALL FUCKING VALID GML NUMBERS:

            5.5.5.... (5.55)
            6...2...9 (6.29)
            .7....3.. (0.73)
            4.2...0.. (4.20) */

            ::std::string number;
            number.reserve(buffer.size());
            bool dotFound = false;
            for (char &c : buffer) {
                if (c == '.') {
                    if (!dotFound) {
                        dotFound = true;
                        number += c;
                    }
                }
                else {
                    number += c;
                }
            }

            Token numToken;
            numToken.Type = TokenType::LiteralReal;

            try {
                numToken.Value = ::std::stod(number);
            }

            catch (const std::runtime_error &) {
                if (buffer == ".") /* . at EOF */ {
                    numToken.Type = TokenType::Separator;
                    numToken.Value = SeparatorType::Period;
                }

                else /* Failed to parse! */ {
                    ::std::ostringstream err;
                    err << "Failed to parse literal real " << buffer << " at position "
                        << static_cast<size_t>(pStart - gml.begin());
                }
            }

            tokenList.push_back(numToken);
            buffer.clear();
        }

        else if (isQuote(*i)) {
            char &q = *i++; // Opening Quote

            for (;;) {
                if (i == end) /* Unexpected EOF while reading! */ {
                    ::std::ostringstream err;
                    err << "Unexpected EOF while reading string " << buffer << " at position "
                        << static_cast<size_t>(i - gml.begin());
                    throw ::std::runtime_error(err.str());
                }

                else if (*i == q) /* Closing Quote */ {
                    i++;
                    break;
                }

                else
                    buffer += *i++; // Anything else goes!
            }

            Token stringToken(buffer);
            tokenList.push_back(stringToken);
            buffer.clear();
        }

        // Comments / Operators / Separators
        else if (isASCII(*i)) {
            char &op_or_sep = *i++;
            bool nextCharExists = (i != end);

            OperatorType op = OperatorType::None;
            SeparatorType sep = SeparatorType::None;

            /* Separators come first, as they are simply more common. */
            switch (op_or_sep) {
                // Separators
                case '(':
                    sep = SeparatorType::ParenLeft;
                    break;

                case ')':
                    sep = SeparatorType::ParenRight;
                    break;

                case '{':
                    sep = SeparatorType::BraceLeft;
                    break;

                case '}':
                    sep = SeparatorType::BraceRight;
                    break;

                case ';':
                    sep = SeparatorType::Semicolon;
                    break;

                case ':':
                    sep = SeparatorType::Colon;
                    break;

                case ',':
                    sep = SeparatorType::Comma;
                    break;

                /* The dot / period operator is handled
                in the number parser! This is because
                it's the same character as a decimal. */

                /* The "then" operator is handled in
                the identifiers! This is because it's a word,
                so it's easier to parse it there. */

                // Operators
                case '+':
                    op = OperatorType::Add;
                    break;

                case '-':
                    op = OperatorType::Subtract;
                    break;

                case '*':
                    op = OperatorType::Multiply;
                    break;

                case '/':
                    op = OperatorType::Divide;
                    break;

                    /* The "div" & "mod" operators are handled in
                    the identifiers! This is because it's a word,
                    so it's easier to parse it there. */

                case '=':
                    op = OperatorType::Assign;
                    break;

                case '&':
                    op = OperatorType::BinaryAnd;
                    break;

                case '|':
                    op = OperatorType::BinaryOr;
                    break;

                case '^':
                    op = OperatorType::BinaryXor;
                    break;

                case '<':
                    op = OperatorType::LessThan;
                    break;

                case '>':
                    op = OperatorType::GreaterThan;
                    break;

                case '!':
                    op = OperatorType::Not;
                    break;

                case '~':
                    op = OperatorType::Complement;
                    break;

                default: {
                    ::std::ostringstream err;
                    err << "Unrecognized character " << *i << " at position "
                        << static_cast<size_t>(i - gml.begin());

                    throw ::std::runtime_error(err.str());
                } break;
            }

            if (nextCharExists && op != OperatorType::None) {
                OperatorType before = op;

                switch (*i) {
                    // Comments

                    // Single-line Comment
                    case '/': {
                        if (op == OperatorType::Divide) {
                            while (i != end) {
                                i++;

                                if (i == end) break;
                                if (*i == '\n' || *i == '\r') break;
                            }

                            if (i == end) continue;
                            if ((i + 1) != end) i++; // Increment past newline
                            continue;                // Break out!
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    // Multi-line Comment
                    /* Note: GM8 does not care if you leave
                        multiline comments unclosed! */
                    case '*': {
                        if (op == OperatorType::Divide) {
                            while (i != end) {
                                i++;

                                if (*i == '*') {
                                    if ((i + 1) != end) {
                                        if (*(i + 1) == '/') break;
                                    }
                                }
                            }

                            if (i == end) continue;  // Break out if @ end
                            i++;                     // Increment past */
                            if ((i + 1) != end) i++; //
                            continue;                // Break out!
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    // Assignment or LTE / GTE
                    case '=': {
                        switch (op) {
                            case OperatorType::Add:
                                op = OperatorType::AssignAdd;
                                break;

                            case OperatorType::Subtract:
                                op = OperatorType::AssignSubtract;
                                break;

                            case OperatorType::Multiply:
                                op = OperatorType::AssignMultiply;
                                break;

                            case OperatorType::Divide:
                                op = OperatorType::AssignDivide;
                                break;

                            case OperatorType::BinaryAnd:
                                op = OperatorType::AssignBinaryAnd;
                                break;

                            case OperatorType::BinaryOr:
                                op = OperatorType::AssignBinaryOr;
                                break;

                            case OperatorType::BinaryXor:
                                op = OperatorType::AssignBinaryXor;
                                break;

                            case OperatorType::Assign:
                                op = OperatorType::Equal;
                                break;

                            case OperatorType::Not:
                                op = OperatorType::NotEqual;
                                break;

                            case OperatorType::LessThan:
                                op = OperatorType::LessThanOrEquals;
                                break;

                            case OperatorType::GreaterThan:
                                op = OperatorType::GreaterThanOrEquals;
                                break;

                            default:
                                ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    case '&': {
                        if (op == OperatorType::BinaryAnd) {
                            op = OperatorType::And;
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    case '|': {
                        if (op == OperatorType::BinaryOr) {
                            op = OperatorType::Or;
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    case '^': {
                        if (op == OperatorType::BinaryXor) {
                            op = OperatorType::Xor;
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    case '<': {
                        if (op == OperatorType::LessThan) {
                            op = OperatorType::BinaryShiftLeft;
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;

                    case '>': {
                        if (op == OperatorType::GreaterThan) {
                            op = OperatorType::BinaryShiftRight;
                        }
                        else {
                            ExcUnrecognizedOperatorCombo(*(i - 1), *i, gml.begin(), i);
                        }
                    } break;
                }

                /* If it's not what it was before,
                we modified it! It's a 2chr OP match. */
                if (before != op) i++;
            }

            if (op == OperatorType::None) /* Separator! */ {
                if (sep != SeparatorType::None) {
                    Token sepToken;
                    sepToken.Type = TokenType::Separator;
                    sepToken.Value = sep;
                    tokenList.push_back(sepToken);
                    buffer.clear();
                }

                else {
                    throw ::std::runtime_error(
                        "Okay, somehow we parsed operators and separators, didn't get an error "
                        "nor a match. This is very awkward. I'm not quite sure how this can "
                        "happen. Please report it, though.");
                }
            }

            else /* Operator! */ {
                Token opToken;
                opToken.Type = TokenType::Operator;
                opToken.Value = op;
                tokenList.push_back(opToken);
                buffer.clear();
            }
        }

        else {
            ::std::ostringstream err;
            err << "Malformed character " << *i << " at position "
                << static_cast<size_t>(i - gml.begin());
            throw ::std::runtime_error(err.str());
        }
    }

    tokenList.shrink_to_fit();
    return tokenList;
}

::std::string GM8Emulator::Compiler::Token::toGMLEquivalent() noexcept
{
    try {
        switch (Type) {
            case TokenType::Identifier:
                return ::std::get<::std::string>(Value);

            case TokenType::LiteralReal:
                return ::std::to_string(std::get<double>(Value));

            case TokenType::LiteralString: {
                ::std::string stringValue = std::get<::std::string>(Value);
                char quote = Constants::DoubleQuote;

                if (stringValue.find(Constants::DoubleQuote) != stringValue.npos) {
                    quote = Constants::SingleQuote;
                }

                stringValue = quote + stringValue + quote;
                return stringValue;
            }

            case TokenType::Keyword: {
                switch (::std::get<KeywordType>(Value)) {
                    case KeywordType::Var:
                        return Constants::KVar;
                    case KeywordType::If:
                        return Constants::KIf;
                    case KeywordType::Else:
                        return Constants::KElse;
                    case KeywordType::With:
                        return Constants::KWith;
                    case KeywordType::Repeat:
                        return Constants::KRepeat;
                    case KeywordType::Do:
                        return Constants::KDo;
                    case KeywordType::Until:
                        return Constants::KUntil;
                    case KeywordType::While:
                        return Constants::KWhile;
                    case KeywordType::For:
                        return Constants::KFor;
                    case KeywordType::Switch:
                        return Constants::KSwitch;
                    case KeywordType::Case:
                        return Constants::KCase;
                    case KeywordType::Default:
                        return Constants::KDefault;
                    case KeywordType::Break:
                        return Constants::KBreak;
                    case KeywordType::Continue:
                        return Constants::KContinue;
                    case KeywordType::Return:
                        return Constants::KReturn;
                    case KeywordType::None:
                    default:
                        break;
                }
            }

            case TokenType::Operator:
                switch (::std::get<OperatorType>(Value)) {
                    case OperatorType::Add:
                        return Constants::OPAdd;
                    case OperatorType::Subtract:
                        return Constants::OPSubtract;
                    case OperatorType::Multiply:
                        return Constants::OPMultiply;
                    case OperatorType::Divide:
                        return Constants::OPDivide;
                    case OperatorType::DivideAndFloor:
                        return Constants::OPDivideAndFloor;
                    case OperatorType::Modulo:
                        return Constants::OPModulo;
                    case OperatorType::Assign:
                        return Constants::OPAssign;
                    case OperatorType::AssignAdd:
                        return Constants::OPAssignAdd;
                    case OperatorType::AssignSubtract:
                        return Constants::OPAssignSubtract;
                    case OperatorType::AssignMultiply:
                        return Constants::OPAssignMultiply;
                    case OperatorType::AssignDivide:
                        return Constants::OPAssignDivide;
                    case OperatorType::AssignBinaryAnd:
                        return Constants::OPAssignBinaryAnd;
                    case OperatorType::AssignBinaryOr:
                        return Constants::OPAssignBinaryOr;
                    case OperatorType::AssignBinaryXor:
                        return Constants::OPAssignBinaryXor;
                    case OperatorType::Equal:
                        return Constants::OPEqual;
                    case OperatorType::NotEqual:
                        return Constants::OPNotEqual;
                    case OperatorType::And:
                        return Constants::OPAnd;
                    case OperatorType::Or:
                        return Constants::OPOr;
                    case OperatorType::Xor:
                        return Constants::OPXor;
                    case OperatorType::LessThan:
                        return Constants::OPLessThan;
                    case OperatorType::LessThanOrEquals:
                        return Constants::OPLessThanOrEquals;
                    case OperatorType::GreaterThan:
                        return Constants::OPGreaterThan;
                    case OperatorType::GreaterThanOrEquals:
                        return Constants::OPGreaterThanOrEquals;
                    case OperatorType::BinaryAnd:
                        return Constants::OPBinaryAnd;
                    case OperatorType::BinaryOr:
                        return Constants::OPBinaryOr;
                    case OperatorType::BinaryXor:
                        return Constants::OPBinaryXor;
                    case OperatorType::BinaryShiftLeft:
                        return Constants::OPBinaryShiftLeft;
                    case OperatorType::BinaryShiftRight:
                        return Constants::OPBinaryShiftRight;
                    case OperatorType::Not:
                        return Constants::OPNot;
                    case OperatorType::Complement:
                        return Constants::OPComplement;

                    case OperatorType::None:
                    default:
                        break;
                }
                break; // todo

            case TokenType::Separator:
                switch (::std::get<SeparatorType>(Value)) {
                    case SeparatorType::ParenLeft:
                        return Constants::SepParenLeft;
                    case SeparatorType::ParenRight:
                        return Constants::SepParenRight;
                    case SeparatorType::BraceLeft:
                        return Constants::SepBraceLeft;
                    case SeparatorType::BraceRight:
                        return Constants::SepBraceRight;
                    case SeparatorType::Semicolon:
                        return Constants::SepSemicolon;
                    case SeparatorType::Colon:
                        return Constants::SepColon;
                    case SeparatorType::Comma:
                        return Constants::SepComma;
                    case SeparatorType::Period:
                        return Constants::SepPeriod;
                    case SeparatorType::PascalThen:
                        return Constants::SepPascalThen;

                    case SeparatorType::None:
                    default:
                        break;
                }
                break;

            case TokenType::None:
            default:
                break;
        }
    }

    catch (const ::std::bad_variant_access &) {
        return "<< Malformed Value >>";
    }

    // If it got up to here something's definitely wrong.
    return "<< Malformed Token >>";
}

::std::string GM8Emulator::Compiler::Token::toTypedIdentifier() noexcept
{
    ::std::ostringstream out;
    switch (Type) {
        case TokenType::Identifier:
            out << "id ";
            break;

        case TokenType::Keyword:
            out << "keyword ";
            break;

        case TokenType::LiteralReal:
        case TokenType::LiteralString:
            out << "literal ";
            break;

        case TokenType::Operator:
            out << "op ";
            break;

        case TokenType::Separator:
            out << "separator ";
            break;
        
        case TokenType::None:
        default:
            out << "unknown ";
            break;
    }

    out << toGMLEquivalent();

    return out.str();
}

void GM8Emulator::Compiler::UnitTest(::std::ostream &out)
{
#ifdef _DEBUG
    /* Yeah this isn't the most optimized thing ever, and the code is very copy pasted! Oh no!
    Yeah, who gives a shit? It's a unit test to see if it's working correctly. uwu; */

    // -- KEYWORDS --
    ::std::string keywords;
    keywords.reserve(128);

    for (uint8_t i = 0; i < static_cast<uint8_t>(KeywordType::None); i++) {
        Token tmp;
        tmp.Type = TokenType::Keyword;
        tmp.Value = static_cast<KeywordType>(i);

        keywords += tmp.toGMLEquivalent() + " ";
    }

    try {
        auto tokens = Tokenize(keywords);
        out << "Asserting Keywords: " << keywords.substr(0, keywords.size()) << std::endl;

        ::std::string kws_tokenized;
        kws_tokenized.reserve(128);

        for (auto &token : tokens) {
            kws_tokenized += token.toGMLEquivalent() + " ";
        }

        if (keywords == kws_tokenized) {
            out << " -> Success!" << std::endl;
        }
        else {
            out << " -> FAILED!" << std::endl
                << " Input: '" << keywords << "'" << std::endl
                << " Output '" << kws_tokenized << "'" << std::endl;
        }
    }

    catch (const std::runtime_error &err) {
        out << "Error while tokenizing operators: " << err.what() << std::endl;
    }

    // -- OPERATORS --
    ::std::string ops;
    ops.reserve(128);

    for (uint8_t i = 0; i < static_cast<uint8_t>(OperatorType::None); i++) {
        Token tmp;
        tmp.Type = TokenType::Operator;
        tmp.Value = static_cast<OperatorType>(i);

        ops += tmp.toGMLEquivalent() + " ";
    }

    try {
        auto tokens = Tokenize(ops);
        out << "Asserting Operators: " << ops.substr(0, ops.size()) << std::endl;

        ::std::string ops_tokenized;
        ops_tokenized.reserve(128);

        for (auto &token : tokens) {
            ops_tokenized += token.toGMLEquivalent() + " ";
        }

        if (ops == ops_tokenized) {
            out << " -> Success!" << std::endl;
        }
        else {
            out << " -> FAILED!" << std::endl
                << " Input: '" << ops << "'" << std::endl
                << " Output '" << ops_tokenized << "'" << std::endl;
        }
    }

    catch (const std::runtime_error &err) {
        out << "Error while tokenizing operators: " << err.what() << std::endl;
    }

    // -- STRINGS --
    /* Trail the teststring with 1 space or it won't work, obvious why. */
    ::std::string stringtest = "\"Hello GM8Emulator!\" '\" dquote' \"' squote\" ";
    try {
        auto stringTokens = Tokenize(stringtest);

        ::std::string returnValue;
        returnValue.reserve(stringtest.size());

        out << "Asserting Strings: " << stringtest << std::endl;

        for (auto &token : stringTokens) {
            returnValue += token.toGMLEquivalent() + " ";
        }

        if (stringtest == returnValue) {
            out << " -> Success!" << std::endl;
        }
        else {
            out << " -> FAILED!" << std::endl;
        }
    }

    catch (const ::std::runtime_error &err) {
        out << " -> ERROR: " << err.what() << std::endl;
    }

    // -- NUMBERS --
    ::std::string numtest = "5.5.5... 6...2...9 .7....3.. 4.2...0.. 6 .7 .8.";
    ::std::vector<::std::string> numbers = {"5.55", "6.29", ".73", "4.20", "6", ".7", ".8"};
    ::std::vector<double> numbers_d;
    for (auto &numstr : numbers) numbers_d.push_back(::std::stod(numstr));

    out << "Asserting Numbers: " << numtest << std::endl;

    try {
        auto numTokens = Tokenize(numtest);

        bool bad = false;
        int i = 0;
        for (auto &token : numTokens) {
            double val = ::std::get<double>(token.Value);
            if (val != numbers_d.at(i)) {
                bad = true;
            }

            i++;
        }

        if (!bad) {
            out << " -> Success!" << std::endl;
        }
        else {
            out << " -> FAILED!" << std::endl;
        }
    }

    catch (const ::std::runtime_error &err) {
        out << " -> ERROR: " << err.what() << std::endl;
    }

    catch (const ::std::bad_variant_access &) {
        out << " -> ERROR: Malformed type return caught while tokenizing numbers." << std::endl;
    }

    catch (const ::std::out_of_range &rangeerr) {
        out << " -> ERROR: Out of range caught while tokenizing numbers. .what(): "
            << rangeerr.what() << std::endl;
    }

    // -- SEPARATORS --
    ::std::string seps;
    seps.reserve(128);

    for (uint8_t i = 0; i < static_cast<uint8_t>(SeparatorType::None); i++) {
        Token tmp;
        tmp.Type = TokenType::Separator;
        tmp.Value = static_cast<SeparatorType>(i);

        seps += tmp.toGMLEquivalent() + " ";
    }

    try {
        auto tokens = Tokenize(seps);
        out << "Asserting Separators: " << seps.substr(0, seps.size()) << std::endl;

        ::std::string seps_tokenized;
        seps_tokenized.reserve(128);

        for (auto &token : tokens) {
            seps_tokenized += token.toGMLEquivalent() + " ";
        }

        if (seps == seps_tokenized) {
            out << " -> Success!" << std::endl;
        }
        else {
            out << " -> FAILED!" << std::endl
                << " Input: '" << seps << "'" << std::endl
                << " Output '" << seps_tokenized << "'" << std::endl;
        }
    }

    catch (const std::runtime_error &err) {
        out << "Error while tokenizing separators: " << err.what() << std::endl;
    }

    // -- MACROS --
    MacroList macros;
    ::std::string macro1 = "func(1, 2, 3)";
    ::std::string macro2 = "1 + 2 + 3";
    macros.insert({"Macro1", Tokenize(macro1)});
    macros.insert({"Macro2", Tokenize(macro2)});

    out << "Asserting Macros: "
        << "Macro1: " << macro1 << " | "
        << "Macro2: " << macro2 << std::endl;

    try {
        ::std::string testTokens = "    if (Macro1) {\r\n"
                                   "        a = Macro2 + Macro1;\r\n"
                                   "    }";

        out << "  Test case:" << std::endl << testTokens << std::endl;

        auto tokens = Tokenize(testTokens, &macros);
        ::std::string tokenized;

        for (auto &token : tokens) {
            tokenized += token.toGMLEquivalent() + " ";
        }

        if (tokenized ==
            "if ( func ( 1.000000 , 2.000000 , 3.000000 ) ) "
            "{ a = 1.000000 + 2.000000 + 3.000000 + func ( 1.000000 , 2.000000 , 3.000000 ) ; } ") {
            out << " -> Success!" << std::endl;
        }
        else {
            out << " -> Failed!" << std::endl;
        }
    }

    catch (const ::std::runtime_error &err) {
        out << " -> FAILED! Error: " << err.what();
    }

#else

    out << "Stop calling the unit test in release mode! Thanks!" << std::endl;

#endif
}
