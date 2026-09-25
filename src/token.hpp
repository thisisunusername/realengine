#pragma once

#include <string>

namespace re {

enum class Tk {

    Ident, Var, Str, Num, RawBlock, KeyName,

    KwFunc, KwJoin, KwAddOther, KwCreateLoop, KwStartLoop, KwStartFunc,
    KwExec, KwWait, KwWaitInput, KwIf, KwThen, KwElse, KwDo, KwIs,
    KwKill, KwPrint, KwLet, KwExit, KwSecond, KwEndIf, KwSet,
    KwGetKeyboard, KwFormFunction, KwTips, KwImport, KwEnd,

    Eq, Comma, Semi, LParen, RParen, LBrace, RBrace,
    Plus, Star, DollarDollar, Hash, End
};

struct Token {
    Tk          kind = Tk::End;
    std::string text;
    int         line = 0;
};

} // namespace re