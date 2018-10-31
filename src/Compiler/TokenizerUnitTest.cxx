#include <pch.h>

#include "Tokenizer.hxx"
#include <iostream>

/* Okay,
 * now
 * this
 * is
 * epic.
 */

void GM8Emulator::
    Compiler::TokenizerUnitTest(
        std::ostream
            &out )
{
#if _DEBUG
    goto w;
w:;
    ;
    const char
        *_ =
            "uwu owo whats this";
    out << "Asserting Sample Identifiers '"
        << _
        << "' ..\n";
    TokenList __(
        _,
        18 );
    std::string
        ____;
    auto _0 =
        __.tokens
            .begin( );
    auto _____sexking6974_____ =
        __.tokens
            .end( );
help_me
    :
    if (
        !( ( *_0 )
                .type ==
            Token::token_type::
                Identifier ) )
        goto _7;
    ____ +=
        ( *_0 )
            .value
            .id;
    ____ +=
        ' ';
    goto _1;
_7:
    out << " -> FAILED! Wrong token type "
           "encountered.\n";
    goto z;
_1:
    _0++;
    if (
        _0 !=
        _____sexking6974_____ )
        goto help_me;

    if (
        !( ____.substr(
               0,
               ____.size( ) -
                   1 ) ==
            _ ) )
        goto iIiiIiiill;

    out << " -> Success!\n";
    goto z;
iIiiIiiill
    :
    out << " -> FAILED! Mismatching result.\n   "
           "Source: "
        << _
        << "\n   Parsed: "
        << ____;
z:;
    std::string
        _____;
    uint8_t _w =
        0;
____O_
    :
    _____ += KeywordToString(
        static_cast<
            KeywordType >(
            _w ) );
    _____ +=
        ' ';
    _w++;
    if (
        _w <
        static_cast<
            uint8_t >(
            KeywordType::
                None ) )
        goto ____O_;
    out << "Asserting Keywords '"
        << _____
               .substr(
                   0,
                   _____.length( ) -
                       1 )
        << "' ..\n";
    std::string
        ____________________________________________________________________;
    TokenList ________(
        _____
            .c_str( ),
        _____
            .length( ) );
    auto __w___ =
        ________
            .tokens
            .begin( );
    auto __w__ =
        ________
            .tokens
            .end( );
___w__
    :
    if (
        ( *__w___ )
            .type ==
        Token::token_type::
            Keyword )
        {
        _________________________________________________________________p
            :
            ____________________________________________________________________ +=
                KeywordToString(
                    ( *__w___ )
                        .value
                        .key );
            ____________________________________________________________________ +=
                ' ';
        }
    else
        {
            out << " -> FAILED! Wrong token type "
                   "encountered.\n";
            goto owoowowoowoWOWOOWOWOWOOWOOWOOOWOWOOOWOOWOWO69;
        }
    __w___++;
    if (
        __w___ !=
        __w__ )
        goto ___w__;

    if (
        ____________________________________________________________________ ==
        _____ )
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching "
                   "result.\n   Source: "
                << _____
                << "\n   Parsed: "
                << ____________________________________________________________________;
        }
owoowowoowoWOWOOWOWOWOOWOOWOOOWOWOOOWOOWOWO69
    :
    std::string
        _________;
    uint8_t __w = static_cast<
        uint8_t >(
        SeparatorType::
            ParenLeft );
__w___
    :
    _________ += SeparatorToString(
        static_cast<
            SeparatorType >(
            __w ) );
    _________ +=
        ' ';
    __w++;
    if (
        __w <
        static_cast<
            uint8_t >(
            SeparatorType::
                None ) )
        goto __w___;
    out << "Asserting Separators '"
        << _________
               .substr(
                   0,
                   _________
                           .length( ) -
                       1 )
        << "' ..\n";
    std::string
        __________;
    TokenList ___________(
        _________
            .c_str( ),
        _________
            .length( ) );
    if (
        _[ 1 ] ==
        'z' )
        goto _________________________________________________________________p;
    auto __w_____ =
        ___________
            .tokens
            .begin( );
    auto __w____ =
        ___________
            .tokens
            .end( );
_____w__________
    :
    if (
        ( *__w_____ )
            .type ==
        Token::token_type::
            Separator )
        {
            __________ += SeparatorToString(
                ( *__w_____ )
                    .value
                    .sep );
            __________ +=
                ' ';
        }
    else
        {
            out << " -> FAILED! Wrong token type "
                   "encountered.\n";
            goto next2;
        }
    __w_____++;
    if (
        __w_____ !=
        __w____ )
        goto _____w__________;

    if (
        __________ ==
        _________ )
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching "
                   "result.\n   Source: "
                << _________
                << "\n   Parsed: "
                << __________;
        }
next2
    :
    std::string
        _____________;
    for (
        uint8_t ___w =
            0;
        ___w <
        static_cast<
            uint8_t >(
            OperatorType::
                None );
        ___w++ )
        {
            _____________ += OperatorToString(
                static_cast<
                    OperatorType >(
                    ___w ) );
            _____________ +=
                ' ';
        }
    out << "Asserting Operators '"
        << _____________
               .substr(
                   0,
                   _____________
                           .length( ) -
                       1 )
        << "' ..\n";
    std::string
        _______________;
    TokenList ________________(
        _____________
            .c_str( ),
        _____________
            .length( ) );
    for (
        const auto
            &______________ :
        ________________
            .tokens )
        {
            if (
                ______________
                    .type ==
                Token::token_type::
                    Operator )
                {
                    _______________ +=
                        OperatorToString(
                            ______________
                                .value
                                .op );
                    _______________ +=
                        ' ';
                }
            else
                {
                    out << " -> FAILED! Wrong "
                           "token type "
                           "encountered.\n";
                    goto next3;
                }
        }
    if (
        _____________ ==
        _______________ )
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching "
                   "result.\n   Source: "
                << _____________
                << "\n   Parsed: "
                << _______________;
        }
next3
    :
    const char
        *_________________ =
            "5.5.5.... 6...2...9 .7....3.. 4.2...0.. "
            "$ $a $A $aa $0d0a";
    out << "Asserting Numbers '"
        << _________________
        << "' ..\n";
    std::string __________________ =
        "5.550000 6.290000 0.730000 4.200000 "
        "0.000000 10.000000 10.000000 170.000000 "
        "3338.000000 ";
    TokenList _____________________(
        _________________,
        57 );
    std::string
        ___________________;
    auto std_string =
        _____________________
            .tokens
            .begin( );
    auto std_string_view =
        _____________________
            .tokens
            .end( );
TokenList
    :
    if (
        ( *std_string )
            .type ==
        Token::token_type::
            Real )
        {
            ___________________ +=
                std::to_string(
                    ( *std_string )
                        .value
                        .real );
            ___________________ +=
                ' ';
        }
    else
        {
            out << " -> FAILED! Wrong token type "
                   "encountered.\n";
            goto next_;
        }
    std_string++;
    if (
        std_string_view !=
        std_string )
        goto TokenList;

    if (
        ___________________ ==
        __________________ )
        {
            out << " -> Success!\n";
        }
    else
        {
            out << " -> FAILED! Mismatching "
                   "result.\n   Source: "
                << _________________
                << "\n   Parsed:   "
                << ___________________
                << "\n   Expected: "
                << __________________
                << "\n";
        }
next_
    :
    const char
        *______________________ =
            "begin end then and or not xor";
    out << "Asserting Stupids '"
        << ______________________
        << "' ..\n";
    std::string
        _______________________;
    TokenList _________________________(
        ______________________,
        29 );
    auto _________________________________________________________________________ =
        _________________________
            .tokens
            .begin( );
    auto ________________________________________________________________________ =
        _________________________
            .tokens
            .end( );
_q:
    if (
        ( *_________________________________________________________________________ )
            .type ==
        Token::token_type::
            Separator )
        {
            _______________________ +=
                SeparatorToString(
                    ( *_________________________________________________________________________ )
                        .value
                        .sep );
            _______________________ +=
                ' ';
        }
    else if (
        ( *_________________________________________________________________________ )
            .type ==
        Token::token_type::
            Operator )
        {
            _______________________ +=
                OperatorToString(
                    ( *_________________________________________________________________________ )
                        .value
                        .op );
            _______________________ +=
                ' ';
        }
    else
        {
            out << " -> FAILED! Wrong token type "
                   "encountered.\n";
            goto next4;
        }
    _________________________________________________________________________++;
    if (
        _________________________________________________________________________ !=
        ________________________________________________________________________ )
        goto _q;
    if (
        _______________________ !=
        "{ } then && || ! ^^ " )
        {
            out << " -> FAILED! Mismatching "
                   "result.\n   Source: "
                << ______________________
                << "\n   Parsed:   "
                << _______________________
                << "\n   Expected: "
                << "{ } then && || ! ^^\n";
        }
    else
        {
            out << " -> Success!\n";
        }
next4
    :;

#else
    out << "Hey, you aren't in debug mode! "
           "Shoo!\n";
#endif
}
