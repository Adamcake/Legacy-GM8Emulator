#include "Tokenizer.hxx"
#include <algorithm>
#include <ctype.h>

// Extra functions that are not in ctype.h for obvious reasons
constexpr bool isunderscore(char& c) { return c == '_'; }
constexpr bool isquotemark(char& c) { return c == '\'' || c == '"'; }
constexpr bool isperiod(char& c) { return c == '.'; }

void GM8Emulator::Compiler::TokenList::ParseGML(const char* gml, const size_t& len) noexcept {
    tokens.clear();  // In case you re-call this
    tokens.reserve(len / 4);

    source = const_cast<char*>(gml);  // Source ptr (we don't copy the data)
    char* end = source + len;         // EOF
    char* i = source;                 // Iterator

    while (i < end) {
        /* Ignore SPC, TAB, LF, VT, FF, CR */
        if (isspace(*i)) {
            i++;
        }

        /* Identifier, Keyword or Operator Word */
        else if (isalpha(*i) || isunderscore(*i)) {
            char* id_start = i++;

            while (i < end) {
                if (isalnum(*i) || isunderscore(*i))
                    i++;

                else
                    break;
            }

            KeywordType key = KeywordType::None;
            std::string_view svw(const_cast<const char*>(id_start), static_cast<size_t>(i - id_start));

            if (svw == "var")
                key = KeywordType::Var;
            else if (svw == "if")
                key = KeywordType::If;
            else if (svw == "else")
                key = KeywordType::Else;
            else if (svw == "with")
                key = KeywordType::With;
            else if (svw == "repeat")
                key = KeywordType::Repeat;
            else if (svw == "do")
                key = KeywordType::Do;
            else if (svw == "until")
                key = KeywordType::Until;
            else if (svw == "while")
                key = KeywordType::While;
            else if (svw == "for")
                key = KeywordType::For;
            else if (svw == "switch")
                key = KeywordType::Switch;
            else if (svw == "case")
                key = KeywordType::Case;
            else if (svw == "default")
                key = KeywordType::Default;
            else if (svw == "break")
                key = KeywordType::Break;
            else if (svw == "continue")
                key = KeywordType::Continue;
            else if (svw == "return")
                key = KeywordType::Return;
            else if (svw == "exit")
                key = KeywordType::Exit;
            else if (svw == "mod") {
                tokens.push_back(Token(OperatorType::Modulo));
                continue;
            }
            else if (svw == "div") {
                tokens.push_back(Token(OperatorType::DivideAndFloor));
                continue;
            }
            else if (svw == "and") {
                tokens.push_back(Token(OperatorType::And));
                continue;
            }
            else if (svw == "or") {
                tokens.push_back(Token(OperatorType::Or));
                continue;
            }
            else if (svw == "xor") {
                tokens.push_back(Token(OperatorType::Xor));
                continue;
            }
            else if (svw == "not") {
                tokens.push_back(Token(OperatorType::Not));
                continue;
            }
            else if (svw == "then") {
                tokens.push_back(Token(SeparatorType::PascalThen));
                continue;
            }
            else if (svw == "begin") {
                tokens.push_back(Token(SeparatorType::BraceLeft));
                continue;
            }
            else if (svw == "end") {
                tokens.push_back(Token(SeparatorType::BraceRight));
                continue;
            }

            if (key == KeywordType::None)
                tokens.push_back(Token(svw));
            else
                tokens.push_back(Token(key));
        }

        /* Number Literal */
        else if (isdigit(*i) || isperiod(*i)) {
            if (isperiod(*i)) {
                if (i + 1 != end) {
                    if (!isdigit(*(i + 1))) {
                        tokens.push_back(Token(SeparatorType::Period));
                        i++;
                        continue;
                    }
                }
                else {
                    tokens.push_back(Token(SeparatorType::Period));
                    i++;
                    break;
                }
            }

            char* num_start = i++;
            while (i < end) {
                if (isdigit(*i) || isperiod(*i))
                    i++;
                else
                    break;
            }

            /* In a number, every dot after the first one is ignored. A number can't begin with '..' - this is checked before entering the loop. For example, '..1' is
            perceived as the dot operator, followed by the literal 0.1 - We make a copy with the stupid extra dots ignored, and pass it to std::stod.
            Thank you Mark Overmars! We appreciate GML, the pinnacle of programming language design! THESE ARE ALL FUCKING VALID GML NUMBERS:

            5.5.5.... (5.55), 6...2...9 (6.29), .7....3.. (0.73), 4.2...0.. (4.20, baby) */

            size_t buf_len = static_cast<size_t>(i - num_start) + 1;  // Total length (in characters) of the number matched above with extra space for a null
            size_t co = 0;                                            // How many characters we've co(pied) from num_start (so we can glue all the characters together)
            char* number = new char[buf_len];                         // Allocate as much space as the source in case it's like, a normal fucking number
            memset(number, 0, buf_len);                               // Clear out buffer with NULL to prevent errors when passing to stod
            bool foundDecimalSeparator = false;                       // Read above for explanation (tl;dr only the first . counts)
            for (size_t p = 0; p < (buf_len - 1); p++) {
                if (isperiod(num_start[p])) {
                    if (!foundDecimalSeparator) {
                        number[co++] = num_start[p];
                        foundDecimalSeparator = true;
                    }
                }
                else {
                    number[co++] = num_start[p];
                }
            }

            double result = atof(number);     // Returns 0 on error, that's fine
            tokens.push_back(Token(result));  // Store number

            delete[] number;
        }

        /* String Literal */
        else if (isquotemark(*i)) {
            char* str_start = i++;
            if (*i != *str_start) {
                while (i < end) {
                    if (*++i == *str_start) break;
                }
            }

            tokens.push_back(Token(const_cast<const char*>(str_start + 1), static_cast<size_t>(i - (str_start + 1)), true));
            i++;  // Increment past end quote
        }

        /* Hex Number Literal */
        else if (*i == '$') {
            char* hexl_start = ++i;

            while (i < end)
                if (!isxdigit(*i))
                    break;
                else
                    i++;

            size_t hexs_len = static_cast<size_t>(i - hexl_start);
            if (hexs_len == 0) {
                tokens.push_back(Token(0.0));
                continue;
            }

            std::string hex_str(std::string_view(hexl_start, hexs_len));
            try {
                unsigned long long result = std::stoull(hex_str, nullptr, 16);
                tokens.push_back(Token(static_cast<double>(result)));
            }
            catch (...) {
                tokens.push_back(Token(static_cast<double>(0xFFFFFFFFFFFFFFFF)));
            }
        }

        /* Operator or Separator or Something Invalid™ */
        else if (isprint(*i)) {

            // No match, malformed character
            if (uint8_t unknown = OperatorSeparatorLUT[(*i) - '!']; unknown == 255U) {
                tokens.push_back(Token());
                i++;
                continue;
            }

            // Separator Match
            else if ((unknown & 0b10000000) == 0b10000000) {
                tokens.push_back(Token(static_cast<SeparatorType>(unknown)));
                i++;
            }

            // Operator Match
            else {
                OperatorType op = static_cast<OperatorType>(unknown);

                // 2-char Operators
                char* i2 = i + 1;
                if (i2 != end) {
                    if (*i2 == '=') {
                        OperatorType match = OperatorComboLUT[std::min(static_cast<uint8_t>(op), static_cast<uint8_t>(11U))];
                        if (match != OperatorType::None) {
                            op = match;
                            i++;
                        }
                    }
                    else if (*i == *i2) {
                        switch (op) {
                            case OperatorType::BinaryAnd:
                                op = OperatorType::And;
                                break;
                            case OperatorType::BinaryOr:
                                op = OperatorType::Or;
                                break;
                            case OperatorType::BinaryXor:
                                op = OperatorType::Xor;
                                break;
                            case OperatorType::LessThan:
                                op = OperatorType::BinaryShiftLeft;
                                break;
                            case OperatorType::GreaterThan:
                                op = OperatorType::BinaryShiftRight;
                                break;

                            /* Single-line Comments */
                            case OperatorType::Divide: {
                                while (i < end) {
                                    if (*i == '\n' || *i == '\r')
                                        break;
                                    else
                                        i++;
                                }

                                i++;  // Increment past CR or LF
                                continue;
                            }
                            default:
                                goto dblop_no_match;  // heheheheheehe
                        }

                        i++;
                    dblop_no_match:;
                    }

                    /* Multi-line Comments */
                    else if (op == OperatorType::Divide && *i2 == '*') {
                        i += 2;  // Increment past /*
                        while (i < end) {
                            if (*i != '*') {
                                i++;  // Skip past anything that isn't the closing tag
                            }
                            else {
                                if (i + 1 < end) {
                                    if (*(++i) == '/') {  // Matched * and / pair, increment past and break
                                        i++;
                                        break;
                                    }
                                    else {
                                        i++;  // Otherwise, it won't be a * next time around, so just increment to save cycles
                                    }
                                }
                                else {
                                    goto stop;  // * into EOF - just exit, no more checks need to be performed.
                                }
                            }
                        }

                        continue;
                    }
                }

                i++;
                tokens.push_back(Token(op));
            }
        }
    }

stop:
    tokens.shrink_to_fit();
}
