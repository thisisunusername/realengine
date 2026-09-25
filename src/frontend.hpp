

#pragma once

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "keyword.hpp"
#include "token.hpp"

namespace re {

class Lexer {
public:
    explicit Lexer(std::string src) : s_(std::move(src)) {}

    std::vector<Token> run() {
        std::vector<Token> out;
        for (;;) {
            Token t = next();
            prev2Kind_ = prevKind_;
            prevKind_  = t.kind;
            out.push_back(t);
            if (t.kind == Tk::End) break;
        }
        return out;
    }

    [[noreturn]] static void fail(const std::string& msg, int line) {
        std::fprintf(stderr, "[RealEngine 语法错误] 第 %d 行: %s\n", line, msg.c_str());
        std::exit(1);
    }

private:
    std::string s_;
    size_t p_ = 0;
    int    line_ = 1;
    Tk     prevKind_  = Tk::End;
    Tk     prev2Kind_ = Tk::End;
    bool   wantRaw_   = false;
    int    rawLine_   = 0;
    int    paren_     = 0;

    char peek(int k = 0) const {
        size_t i = p_ + (size_t)k;
        return i < s_.size() ? s_[i] : 0;
    }
    char get() {
        char c = peek();
        if (c) { ++p_; if (c == '\n') ++line_; }
        return c;
    }
    static bool isHs(unsigned char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

    void skipBlank() {
        for (;;) {
            char c = peek();
            if (c && isHs((unsigned char)c)) { get(); continue; }

            if (c == '/' && peek(1) == '/') {
                while (peek() && peek() != '\n') get();
                continue;
            }

            break;
        }
    }

    std::string scanString(char quote, int ln) {
        std::string buf;
        for (;;) {
            char c = peek();
            if (!c) fail("字符串没有收尾引号", ln);
            if (c == quote) { get(); break; }
            if (c == '\\') {
                get();
                char d = peek();
                if (!d) fail("字符串末尾的反斜杠", ln);
                get();
                switch (d) {
                    case 'n': buf += '\n'; break;
                    case 't': buf += '\t'; break;
                    case 'r': buf += '\r'; break;
                    case '\\': buf += '\\'; break;
                    default:  buf += d;   break;
                }
                continue;
            }
            buf += get();
        }
        return buf;
    }

    std::string scanRawBlock(char quote, int ) {
        std::string buf;
        const bool multiline = (peek() == '\n' || peek() == '\r');

        if (multiline) {

            while (peek() == '\n' || peek() == '\r') get();
            for (;;) {
                char c = peek();
                if (!c) break;
                if (c == '\\') { get(); buf += '\\'; if (peek()) buf += get(); continue; }
                if (c == quote) {

                    bool atHead = true;
                    for (size_t k = 1; k <= buf.size(); ++k) {
                        char pc = buf[buf.size() - k];
                        if (pc == '\n') break;
                        if (pc != ' ' && pc != '\t' && pc != '\r') { atHead = false; break; }
                    }
                    if (atHead) { get(); break; }
                    buf += get();
                    continue;
                }
                buf += get();
            }
        } else {

            for (;;) {
                char c = peek();
                if (!c) break;
                if (c == '\\') { get(); buf += '\\'; if (peek()) buf += get(); continue; }
                if (c == quote) {

                    if (paren_ == 0) { get(); break; }
                    buf += get();
                    continue;
                }
                if (c == '(') ++paren_;
                else if (c == ')') { if (paren_ > 0) --paren_; }
                buf += get();
            }
            paren_ = 0;
        }

        while (!buf.empty() && (buf.back() == ' ' || buf.back() == '\t' ||
                                buf.back() == '\r' || buf.back() == '\n'))
            buf.pop_back();
        return buf;
    }

    std::string scanDollarLiteral(int ln) {

        get();
        get();
        if (peek() == '"') {
            get();
            while (peek() && peek() != '$') get();
            if (peek() == '$') get();
            return "\"";
        }

        std::string buf;
        while (peek() && peek() != '$') buf += get();
        if (peek() == '$') get();
        (void)ln;
        return buf;
    }

    Token scanDirective(int ln) {
        get();
        std::string name;
        while (std::isalnum((unsigned char)peek()) || peek() == '_') name += get();

        if (name == "import") {
            while (peek() == ' ' || peek() == '\t') get();
            std::string file;
            if (peek() == '<') {
                get();
                while (peek() && peek() != '>' && peek() != '\n') file += get();
                if (peek() == '>') get();
            } else {
                while (peek() && peek() != '\n' && peek() != ' ' && peek() != '\t')
                    file += get();
            }
            while (peek() && peek() != '\n') get();
            return {Tk::KwImport, file, ln};
        }
        if (name == "tips") {
            std::string rest;
            while (peek() == ' ' || peek() == '\t') get();
            while (peek() && peek() != '\n' && peek() != ' ') rest += get();
            while (peek() && peek() != '\n') get();
            return {Tk::KwTips, rest.empty() ? "tips" : rest, ln};
        }
        while (peek() && peek() != '\n') get();
        return {Tk::KwTips, name.empty() ? "tips" : name, ln};
    }

    Token next() {
        skipBlank();
        int  ln = line_;
        char c  = peek();
        if (!c) return {Tk::End, "", ln};

        const char q0 = '"';
        const char q1 = 39;

        if (c == '#') return scanDirective(ln);

        if (prev2Kind_ == Tk::KwDo && prevKind_ == Tk::Eq) {
            wantRaw_     = true;
            rawLine_     = ln;
        }

        if (wantRaw_ && ln != rawLine_) wantRaw_ = false;

        if (wantRaw_ && (c == q0 || c == q1)) {
            wantRaw_ = false;
            char q = get();
            return {Tk::RawBlock, scanRawBlock(q, ln), ln};
        }

        if (wantRaw_ && (c == ';' || c == '}' || c == '\n')) wantRaw_ = false;

        if (c == '$') {
            if (peek(1) == '"') {

                std::string lit = scanDollarLiteral(ln);
                return {Tk::Str, lit, ln};
            }
            get();
            std::string buf;
            while ((std::isalnum((unsigned char)peek()) || peek() == '_')) buf += get();
            if (buf.empty()) return {Tk::DollarDollar, "$", ln};
            return {Tk::Var, buf, ln};
        }

        if (c == q0 || c == q1) {
            char q = get();
            return {Tk::Str, scanString(q, ln), ln};
        }

        if (std::isdigit((unsigned char)c)) {
            std::string buf;
            while (std::isdigit((unsigned char)peek())) buf += get();
            return {Tk::Num, buf, ln};
        }

        unsigned char uc = (unsigned char)c;
        if (std::isalpha(uc) || c == '_' || uc >= 0x80) {
            std::string buf;
            for (;;) {
                char d = peek();
                unsigned char ud = (unsigned char)d;
                if (!d) break;
                if (std::isalnum(ud) || d == '_' || ud >= 0x80) { buf += get(); continue; }
                break;
            }
            auto it = keywordTable().find(buf);
            if (it != keywordTable().end()) return {it->second, buf, ln};
            return {Tk::Ident, buf, ln};
        }

        switch (c) {
            case '=': get(); return {Tk::Eq,     "=", ln};
            case ',': get(); return {Tk::Comma,  ",", ln};
            case ';': get(); return {Tk::Semi,   ";", ln};
            case '(': get(); return {Tk::LParen, "(", ln};
            case ')': get(); return {Tk::RParen, ")", ln};
            case '{': get(); return {Tk::LBrace, "{", ln};
            case '}': get(); return {Tk::RBrace, "}", ln};
            case '+': get(); return {Tk::Plus,   "+", ln};
            case '*': get(); return {Tk::Star,   "*", ln};
            default: break;
        }
        {
            char hx[16];
            std::snprintf(hx, sizeof hx, "0x%02X", uc);
            fail(std::string("不认识这个字符 '") + c + "' (" + hx + ")", ln);
        }
    }
};

struct Stmt;
using StmtPtr = std::shared_ptr<Stmt>;

enum class St {
    Func, Join, AddOther,
    LoopCreate, LoopStart, StartFunc,
    Exec, Wait, WaitInput, Kill, Print, Let, Set, Exit,
    If,
    Expr,
    Import, Tips
};

struct Stmt {
    St          kind = St::Exit;
    int         line = 0;
    std::string a, b, c;
    std::string fname;
    std::string ftype;
    std::vector<std::string> args;
    int         number = 0;
    std::string unit;
    std::vector<StmtPtr> body;
    std::string doBlock;
    std::string thenBlock, elseBlock;
    std::vector<StmtPtr> thenBody, elseBody;
};

class Parser {
public:
    explicit Parser(std::vector<Token> ts) : t_(std::move(ts)) {}

    std::vector<StmtPtr> parseProgram() {
        std::vector<StmtPtr> out;
        while (!at(Tk::End)) out.push_back(parseStmt());
        return out;
    }

private:
    std::vector<Token> t_;
    size_t i_ = 0;

    const Token& cur() const { return t_[i_]; }
    bool  at(Tk k) const { return cur().kind == k; }
    const Token& take() { return t_[i_++]; }
    bool  accept(Tk k) { if (at(k)) { ++i_; return true; } return false; }

    void expect(Tk k, const char* what) {
        if (!at(k)) Lexer::fail(std::string("这里应该出现 ") + what, cur().line);
        ++i_;
    }
    std::string expectAny(const char* what) {
        Tk k = cur().kind;
        if (k != Tk::Str && k != Tk::RawBlock && k != Tk::Ident && k != Tk::Num)
            Lexer::fail(std::string("这里应该出现 ") + what, cur().line);
        return take().text;
    }

    void eatSemis() { while (accept(Tk::Semi)) {} }

    bool tryDoBlock(std::string& dst) {
        size_t save = i_;
        eatSemis();
        if (!accept(Tk::KwDo)) { i_ = save; return false; }
        accept(Tk::Eq);

        auto isStmtStart = [](Tk k) {
            switch (k) {
                case Tk::KwFunc: case Tk::KwJoin: case Tk::KwAddOther:
                case Tk::KwCreateLoop: case Tk::KwStartLoop: case Tk::KwStartFunc:
                case Tk::KwExec: case Tk::KwWait: case Tk::KwWaitInput:
                case Tk::KwIf: case Tk::KwKill: case Tk::KwPrint:
                case Tk::KwLet: case Tk::KwSet: case Tk::KwExit:
                case Tk::KwGetKeyboard: case Tk::KwFormFunction:
                case Tk::KwTips: case Tk::KwImport:
                case Tk::End: case Tk::RBrace:
                    return true;
                default: return false;
            }
        };

        if (at(Tk::Str) || at(Tk::RawBlock)) {

            Tk nxt = (i_ + 1 < t_.size()) ? t_[i_ + 1].kind : Tk::End;
            if (nxt == Tk::Semi || nxt == Tk::KwElse || nxt == Tk::KwEndIf ||
                nxt == Tk::RBrace || nxt == Tk::End || isStmtStart(nxt)) {
                dst = take().text;
                return true;
            }
        }

        std::string buf;
        while (!at(Tk::End) && !at(Tk::RBrace) && !at(Tk::Semi) &&
               !at(Tk::KwElse) && !at(Tk::KwEndIf)) {
            if (!buf.empty() && isStmtStart(cur().kind)) break;
            const Token& tk = cur();
            if (tk.kind == Tk::Str || tk.kind == Tk::RawBlock) {
                if (!tk.text.empty()) {
                    if (!buf.empty() && buf.back() != ' ') buf += ' ';
                    buf += tk.text;
                }
                take();
            } else {
                if (!buf.empty() && buf.back() != ' ') buf += ' ';
                if (tk.kind == Tk::Var) buf += '$';
                buf += take().text;
            }
        }
        if (buf.empty()) { i_ = save; return false; }
        dst = buf;
        return true;
    }

    StmtPtr parseIf() {
        auto s = std::make_shared<Stmt>();
        s->kind = St::If;
        s->line = cur().line;
        take();

        std::string cond;
        auto pushTok = [&](const Token& tk) {
            if (tk.kind == Tk::Str || tk.kind == Tk::RawBlock) {
                if (!cond.empty() && cond.back() != ' ') cond += ' ';
                cond += '"'; cond += tk.text; cond += '"';
            } else {
                if (!cond.empty() && cond.back() != ' ' && cond.back() != '*') cond += ' ';
                if (tk.kind == Tk::Var) cond += '$';
                cond += tk.text;
            }
        };
        while (!at(Tk::End) && !at(Tk::KwThen) && !at(Tk::RBrace)) {
            pushTok(take());
        }

        while (!cond.empty() && cond.back() == ';') cond.pop_back();
        while (!cond.empty() && cond.back() == ' ')  cond.pop_back();
        s->a = cond;

        if (accept(Tk::KwThen)) {
            eatSemis();
            tryDoBlock(s->thenBlock);
        }

        eatSemis();
        if (accept(Tk::KwElse)) {
            eatSemis();
            tryDoBlock(s->elseBlock);
        }
        eatSemis();
        accept(Tk::KwEndIf);
        eatSemis();
        return s;
    }

    StmtPtr parseStmt() {
        auto s = std::make_shared<Stmt>();
        s->line = cur().line;

        switch (cur().kind) {

        case Tk::KwFunc: {
            take();
            s->kind = St::Func;
            s->fname = expectAny("函数名");
            if (accept(Tk::LParen)) {

                while (!at(Tk::RParen) && !at(Tk::End)) {
                    if (at(Tk::Eq)) { take(); s->ftype = expectAny("type 的值"); }
                    else if (at(Tk::RParen)) break;
                    else take();
                }
                expect(Tk::RParen, ")");
            }
            if (accept(Tk::LBrace)) s->body = parseBlock();
            else if (accept(Tk::KwDo)) { accept(Tk::Eq); s->doBlock = expectAny("do= 的块"); }
            break;
        }

        case Tk::KwJoin: {
            take();
            s->kind = St::Join;
            s->a = expectAny("函数名");
            break;
        }

        case Tk::KwAddOther: {
            take();
            s->kind = St::AddOther;
            s->a = expectAny("函数名列表");
            break;
        }

        case Tk::KwCreateLoop: {
            take(); s->kind = St::LoopCreate;
            s->a = expectAny("循环名");
            eatSemis();
            while (!at(Tk::KwDo) && !at(Tk::End) && !at(Tk::Semi)) take();
            tryDoBlock(s->doBlock);
            break;
        }
        case Tk::KwStartLoop: {
            take(); s->kind = St::LoopStart; s->a = expectAny("循环名"); break;
        }
        case Tk::KwStartFunc: {
            take(); s->kind = St::StartFunc; s->a = expectAny("函数名"); break;
        }

        case Tk::KwExec: {
            take(); s->kind = St::Exec;
            expect(Tk::LParen, "(");
            if (!at(Tk::RParen)) {
                s->a = expectAny("要执行的东西");
                while (accept(Tk::Comma)) s->args.push_back(expectAny("参数"));
            }
            expect(Tk::RParen, ")");
            break;
        }

        case Tk::KwWait: {
            take(); s->kind = St::Wait;
            if (at(Tk::Num)) s->number = std::atoi(take().text.c_str());
            else if (at(Tk::Var)) { s->b = take().text; }
            if (at(Tk::KwSecond)) s->unit = take().text;
            break;
        }

        case Tk::KwWaitInput: {
            take(); s->kind = St::WaitInput;
            if (at(Tk::Str) || at(Tk::Var) || at(Tk::Ident)) s->a = take().text;
            break;
        }

        case Tk::KwGetKeyboard: {
            take(); s->kind = St::Set;
            s->a = "get_keyboard";

            std::string kb;
            while (!at(Tk::End) && !at(Tk::RBrace) && !at(Tk::Semi)) {
                if (!kb.empty()) kb += ' ';
                kb += take().text;
            }
            s->b = kb;
            break;
        }

        case Tk::KwFormFunction: {
            take(); s->kind = St::Set;
            s->a = "form_function";
            accept(Tk::Eq);
            s->b = expectAny("函数名");
            break;
        }

        case Tk::KwSet: {
            take(); s->kind = St::Set;
            s->a = expectAny("变量名");
            accept(Tk::Eq);

            std::string rhs;
            while (!at(Tk::End) && !at(Tk::RBrace) && !at(Tk::Semi)) {
                const Token& tk = cur();
                if (tk.kind == Tk::Str || tk.kind == Tk::RawBlock) {
                    if (!rhs.empty()) rhs += ' ';
                    rhs += '"'; rhs += take().text; rhs += '"';
                } else {
                    if (!rhs.empty() && rhs.back() != ' ') rhs += ' ';
                    if (tk.kind == Tk::Var) rhs += '$';
                    rhs += take().text;
                }
            }
            s->b = rhs;
            break;
        }

        case Tk::KwIf:
            return parseIf();

        case Tk::KwKill: {
            take(); s->kind = St::Kill;
            if (at(Tk::Str) || at(Tk::Ident) || at(Tk::RawBlock)) s->a = take().text;
            break;
        }

        case Tk::KwPrint: {
            take(); s->kind = St::Print;
            bool paren = accept(Tk::LParen);
            std::string acc;
            while (!at(Tk::End) && !at(Tk::RBrace) && !at(Tk::Semi) &&
                   !(paren && at(Tk::RParen))) {
                const Token& tk = cur();
                if (tk.kind == Tk::Str || tk.kind == Tk::RawBlock) {
                    if (!acc.empty() && acc.back() != ' ') acc += ' ';
                    acc += take().text;
                } else {
                    if (!acc.empty() && acc.back() != ' ') acc += ' ';
                    if (tk.kind == Tk::Var) acc += '$';
                    acc += take().text;
                }
            }
            if (paren) expect(Tk::RParen, ")");
            s->a = acc;
            break;
        }

        case Tk::KwLet: {
            take(); s->kind = St::Let;
            if (at(Tk::Var) || at(Tk::Ident)) s->a = take().text;
            accept(Tk::Eq);
            if (at(Tk::Num)) s->number = std::atoi(take().text.c_str());
            else if (at(Tk::Str) || at(Tk::RawBlock)) s->b = take().text;
            break;
        }

        case Tk::KwExit: {
            take(); s->kind = St::Exit; break;
        }

        case Tk::KwTips: {
            take(); s->kind = St::Tips; s->a = t_[i_ - 1].text; break;
        }
        case Tk::KwImport: {
            s->a = take().text;
            s->kind = St::Import;
            break;
        }

        case Tk::KwThen:
        case Tk::KwElse:
        case Tk::KwEndIf:
        case Tk::KwDo: {
            take();
            s->kind = St::Tips;
            s->a = "(孤立关键字)";
            break;
        }

        case Tk::Str:
        case Tk::RawBlock:
        case Tk::Num: {
            s->kind = St::Expr;
            s->a = take().text;
            break;
        }

        case Tk::Var:
        case Tk::Ident:
        case Tk::DollarDollar: {
            s->kind = St::Expr;
            std::string e;
            while (!at(Tk::End) && !at(Tk::RBrace) && !at(Tk::Semi) &&
                   !at(Tk::KwElse) && !at(Tk::KwEndIf)) {
                const Token& tk = cur();
                if (tk.kind == Tk::Str || tk.kind == Tk::RawBlock) {
                    if (!e.empty() && e.back() != ' ') e += ' ';
                    e += '"'; e += take().text; e += '"';
                } else {
                    if (!e.empty() && e.back() != ' ') e += ' ';
                    if (tk.kind == Tk::Var) e += '$';
                    e += take().text;
                }
            }
            s->a = e;
            break;
        }

        default:
            Lexer::fail("这一行看不懂，开头是: " + cur().text, cur().line);
        }

        eatSemis();
        return s;
    }

    std::vector<StmtPtr> parseBlock() {
        std::vector<StmtPtr> out;
        while (!at(Tk::RBrace) && !at(Tk::End)) out.push_back(parseStmt());
        expect(Tk::RBrace, "}");
        return out;
    }
};

inline std::vector<Token> lexAll(const std::string& src) {
    Lexer lx(src);
    return lx.run();
}

inline std::string readWholeFile(const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (!f) { std::fprintf(stderr, "打不开文件: %s\n", path); std::exit(1); }
    std::string out;
    char buf[8192];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
    std::fclose(f);
    return out;
}

}
