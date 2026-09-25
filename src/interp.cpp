

#include "frontend.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace re;

struct FuncDef {
    std::string          name;
    std::string          type;
    std::vector<StmtPtr> body;
    std::string          doBlock;
};

struct Runtime {
    std::map<std::string, std::string> vars;
    std::map<std::string, FuncDef>     funcs;
    std::vector<std::string>           otherFuncs;
    std::map<std::string, std::string> loops;
    std::string mainFunc;
    bool        hasMain   = false;
    bool        shouldExit = false;
    int         lastExit  = 0;
    int         depth     = 0;

    std::string runCommand(const std::string& cmd) {
        std::string out;
        FILE* f = popen((cmd + " 2>/dev/null").c_str(), "r");
        if (!f) return "";
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
        pclose(f);
        while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
        return out;
    }

    std::string expand(const std::string& in) {
        std::string out;
        for (size_t i = 0; i < in.size();) {
            if (in[i] != '$') { out += in[i++]; continue; }

            static const std::string kExecCmd = "$execute_command";
            if (in.compare(i, kExecCmd.size(), kExecCmd) == 0) {
                size_t j = i + kExecCmd.size();
                while (j < in.size() && (in[j] == ' ' || in[j] == '\t')) ++j;

                size_t start = j;
                int    par   = 0;
                while (j < in.size()) {
                    char c = in[j];
                    if (c == '(') ++par;
                    else if (c == ')') { if (par == 0) break; --par; }
                    else if ((c == ';' || c == '\'' || c == '"') && par == 0) break;
                    ++j;
                }
                std::string cmd = in.substr(start, j - start);

                while (!cmd.empty() && (cmd.front() == ' ')) cmd.erase(cmd.begin());
                while (!cmd.empty() && (cmd.back() == ' '))  cmd.pop_back();
                out += runCommand(cmd);
                i = j;
                continue;
            }

            size_t j = i + 1;
            std::string name;
            while (j < in.size() && (std::isalnum((unsigned char)in[j]) || in[j] == '_'))
                name += in[j++];
            if (name.empty()) { out += in[i++]; continue; }

            auto it = vars.find(name);
            out += (it != vars.end() ? it->second : std::string());
            i = j;
        }
        return out;
    }

    std::string evalConcat(const std::string& expr) {

        std::vector<std::string> parts;
        std::string cur;
        bool inStr = false;
        for (size_t i = 0; i < expr.size(); ++i) {
            char c = expr[i];
            if (c == '"') inStr = !inStr;
            if (!inStr && c == '+') { parts.push_back(cur); cur.clear(); continue; }
            cur += c;
        }
        parts.push_back(cur);
        std::string out;
        for (auto& p : parts) {
            std::string t = p;
            while (!t.empty() && t.front() == ' ') t.erase(t.begin());
            while (!t.empty() && t.back()  == ' ') t.pop_back();
            if (t.size() >= 2 && t.front() == '"' && t.back() == '"')
                t = t.substr(1, t.size() - 2);
            out += expand(t);
        }
        return out;
    }

    bool evalCond(const std::string& cond) {
        std::string c = cond;

        size_t pos = c.find(" is ");
        if (pos == std::string::npos) {
            return !expand(c).empty();
        }
        std::string lhsRaw = c.substr(0, pos);
        std::string rhsRaw = c.substr(pos + 4);

        auto trim = [](std::string& s) {
            while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
            while (!s.empty() && (s.back()  == ' ' || s.back()  == '\t')) s.pop_back();
        };
        trim(lhsRaw); trim(rhsRaw);
        std::string lhs = expand(lhsRaw);

        bool  wildcard = false;
        std::string wcTail;
        size_t star = rhsRaw.rfind('*');
        if (star != std::string::npos && star + 1 < rhsRaw.size()) {

            std::string tail = rhsRaw.substr(star + 1);
            bool simple = true;
            for (char ch : tail)
                if (!std::isalnum((unsigned char)ch) && ch != '_' && ch != '.') { simple = false; break; }
            if (simple && !tail.empty()) { wildcard = true; wcTail = tail; }
        }

        std::string rhs = rhsRaw;
        if (wildcard) rhs = rhsRaw.substr(0, star);
        trim(rhs);
        if (rhs.size() >= 2 && rhs.front() == '"' && rhs.back() == '"')
            rhs = rhs.substr(1, rhs.size() - 2);
        rhs = expand(rhs);

        if (wildcard) {

            if (lhs.size() >= wcTail.size())
                return lhs.compare(lhs.size() - wcTail.size(), wcTail.size(), wcTail) == 0;
            return false;
        }
        return lhs == rhs;
    }

    void execCode(const std::string& code, bool echoPrompt = false) {
        if (code.empty()) return;
        (void)echoPrompt;
        Lexer  lx(code);
        Parser ps(lx.run());
        execStmts(ps.parseProgram());
    }

    std::vector<std::string> importPaths;
    std::map<std::string, bool> imported;

    static bool fileExists(const std::string& p) {
        FILE* f = std::fopen(p.c_str(), "rb");
        if (!f) return false;
        std::fclose(f);
        return true;
    }

    void doImport(const std::string& name) {
        if (name.empty()) return;
        if (imported.count(name)) return;
        imported[name] = true;

        std::string found;
        for (const auto& dir : importPaths) {
            std::string cand = dir + "/" + name;
            if (fileExists(cand)) { found = cand; break; }
        }
        if (found.empty() && fileExists(name)) found = name;

        if (found.empty()) {
            std::printf("[导入] %s  (未找到，跳过)\n", name.c_str());
            return;
        }

        std::string src = readWholeFile(found.c_str());
        std::printf("[导入] %s\n", found.c_str());
        Lexer  lx(src);
        Parser ps(lx.run());
        execStmts(ps.parseProgram());
    }

    void execStmts(const std::vector<StmtPtr>& stmts);

    void execOne(const StmtPtr& s);

    void callFunc(const std::string& name);
};

void Runtime::execOne(const StmtPtr& s) {
    if (!s || shouldExit) return;

    switch (s->kind) {

    case St::Func: {
        FuncDef d;
        d.name    = s->fname;
        d.type    = s->ftype;
        d.body    = s->body;
        d.doBlock = s->doBlock;
        funcs[d.name] = d;
        if (true) {
            std::printf("[定义函数] %s", d.name.c_str());
            if (!d.type.empty()) std::printf(" (type=%s)", d.type.c_str());
            std::printf(" 共 %zu 条语句\n", d.body.size());
        }
        break;
    }

    case St::Join: {
        mainFunc = s->a;
        hasMain  = !s->a.empty();
        std::printf("[装配] 主函数 = %s\n", s->a.c_str());
        break;
    }

    case St::AddOther: {
        std::string list = s->a;
        std::string cur;
        for (char c : list) {
            if (c == ',') { if (!cur.empty()) otherFuncs.push_back(cur); cur.clear(); }
            else cur += c;
        }
        if (!cur.empty()) otherFuncs.push_back(cur);
        std::printf("[装配] 附加函数 %zu 个:", otherFuncs.size());
        for (auto& f : otherFuncs) std::printf(" %s", f.c_str());
        std::printf("\n");
        break;
    }

    case St::StartFunc: {
        std::printf("[调用函数] %s\n", s->a.c_str());
        callFunc(s->a);
        break;
    }

    case St::LoopCreate:
        loops[s->a] = s->doBlock;
        std::printf("[定义循环] %s\n", s->a.c_str());
        break;

    case St::LoopStart:
        std::printf("[启动循环] %s\n", s->a.c_str());
        break;

    case St::Exec: {
        std::string path = evalConcat("\"" + s->a + "\"");
        std::string cmd  = path;
        for (auto& a : s->args) { cmd += ' '; cmd += a; }
        std::printf("  -> 执行 %s\n", cmd.c_str());
        int rc = std::system(cmd.c_str());
        lastExit = (rc == -1) ? 127 : (WEXITSTATUS(rc));
        vars["up_cmd_exitcode"] = std::to_string(lastExit);
        break;
    }

    case St::Wait: {
        int n = s->number;
        std::string unit = s->unit;
        int ms = 1000;
        if (unit == "ms") ms = 1;
        std::printf("[等待] %d %s\n", n, unit.empty() ? "second" : unit.c_str());
        if (n > 0) ::usleep((useconds_t)n * ms * 1000);
        break;
    }

    case St::WaitInput:
        std::printf("[等待输入]\n");
        break;

    case St::Kill:
        std::printf("[kill] %s\n", s->a.c_str());
        break;

    case St::Print: {
        std::string raw = s->a;
        if (raw.size() >= 2 && ((raw.front() == '"' && raw.back() == '"') ||
                                (raw.front() == '\'' && raw.back() == '\'')))
            raw = raw.substr(1, raw.size() - 2);
        std::string txt = expand(raw);
        std::printf("%s", txt.c_str());
        break;
    }

    case St::Set: {

        if (s->a == "get_keyboard") {
            std::printf("[键盘字符集] %s\n", s->b.c_str());
        } else if (s->a == "form_function") {
            std::printf("[绑定函数] %s\n", s->b.c_str());
        } else {
            vars[s->a] = evalConcat(s->b);
            std::printf("[设置] %s = %s\n", s->a.c_str(), vars[s->a].c_str());
        }
        break;
    }

    case St::Let: {
        if (!s->b.empty()) vars[s->a] = s->b;
        else               vars[s->a] = std::to_string(s->number);
        break;
    }

    case St::Exit:
        shouldExit = true;
        std::printf("[退出]\n");
        break;

    case St::If: {
        bool ok = evalCond(s->a);
        std::printf("[判断] %s  -> %s\n", s->a.c_str(), ok ? "真" : "假");
        const std::string& blk = ok ? s->thenBlock : s->elseBlock;
        if (!blk.empty()) execCode(blk);
        break;
    }

    case St::Expr:
        std::printf("[表达式] %s\n", expand(s->a).c_str());
        break;

    case St::Import:
        doImport(s->a);
        break;

    case St::Tips:
        break;
    }
}

void Runtime::execStmts(const std::vector<StmtPtr>& stmts) {
    for (auto& s : stmts) {
        if (shouldExit) break;
        execOne(s);
    }
}

void Runtime::callFunc(const std::string& name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
        std::printf("  (函数 %s 不存在)\n", name.c_str());
        return;
    }
    if (depth > 64) { std::printf("  (递归太深，停下)\n"); return; }
    ++depth;
    const FuncDef& f = it->second;
    if (!f.body.empty()) execStmts(f.body);
    else if (!f.doBlock.empty()) execCode(f.doBlock);
    --depth;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "用法: %s <file.re>\n", argv[0]);
        return 2;
    }
    std::string src = readWholeFile(argv[1]);
    std::printf("=== RealEngine 解释器 ===\n");
    std::printf("文件: %s (%zu 字节)\n", argv[1], src.size());

    auto toks = lexAll(src);
    std::printf("Token 数: %zu\n", toks.size() - 1);

    Parser ps(std::move(toks));
    auto prog = ps.parseProgram();
    std::printf("解析完成：%zu 条顶层语句\n\n", prog.size());

    Runtime rt;
    rt.importPaths.push_back("libs");
    rt.importPaths.push_back("include");
    rt.importPaths.push_back("examples/libs");
    {
        std::string p = argv[1];
        size_t slash = p.find_last_of('/');
        if (slash != std::string::npos) {
            std::string dir = p.substr(0, slash);
            rt.importPaths.push_back(dir);
            rt.importPaths.push_back(dir + "/libs");
        }
    }
    rt.execStmts(prog);

    std::printf("\n=== 解释结束 ===\n");
    return 0;
}
