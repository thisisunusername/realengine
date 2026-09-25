#pragma once

#include <map>
#include <string>

#include "token.hpp"

namespace re {

inline const std::map<std::string, Tk>& keywordTable() {
    static const std::map<std::string, Tk> kw = {
        {"function_add",       Tk::KwFunc},
        {"join_function",      Tk::KwJoin},
        {"add_other_function", Tk::KwAddOther},
        {"create_loop",        Tk::KwCreateLoop},
        {"start_loop",         Tk::KwStartLoop},
        {"start_function",     Tk::KwStartFunc},
        {"execve",             Tk::KwExec},
        {"wait",               Tk::KwWait},
        {"wait_input",         Tk::KwWaitInput},
        {"if",                 Tk::KwIf},
        {"then",               Tk::KwThen},
        {"else",               Tk::KwElse},
        {"do",                 Tk::KwDo},
        {"is",                 Tk::KwIs},
        {"kill",               Tk::KwKill},
        {"printf",             Tk::KwPrint},
        {"let",                Tk::KwLet},
        {"set",                Tk::KwSet},
        {"exit",               Tk::KwExit},
        {"end_if",             Tk::KwEndIf},
        {"second",             Tk::KwSecond},
        {"seconds",            Tk::KwSecond},
        {"ms",                 Tk::KwSecond},
        {"get_keyboard",       Tk::KwGetKeyboard},
        {"form_function",      Tk::KwFormFunction},
    };
    return kw;
}

inline bool isKeyword(const std::string& word) {
    return keywordTable().count(word) > 0;
}

} // namespace re