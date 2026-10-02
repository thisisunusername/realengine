// 桩头文件：让 Linux 上的 g++ 能继续解析 _WIN32 分支
// 仅用于语法检查，不参与真实编译
#pragma once

typedef unsigned long DWORD;

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

static inline FILE* _popen(const char* c, const char* m) { return ::popen(c, m); }
static inline int   _pclose(FILE* f) { return ::pclose(f); }
static inline void  Sleep(DWORD ms) { ::usleep((unsigned int)ms * 1000); }
