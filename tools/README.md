# tools/ — 开发辅助

## win_stub.h / win_stub/

**用途**：在 **Linux/macOS 上做 Windows 分支的语法检查**。

RealEngine 的跨平台代码用 `#if defined(_WIN32)` 隔离，正常在 Linux 上编译时
Windows 分支**完全不会被解析**，等于没有验证。用这组桩文件强制展开它：

```bash
# 在 Linux 上检查 Windows 分支是否存在语法错误
g++ -std=c++17 -fsyntax-only -D_WIN32 -Itools/win_stub -Isrc src/interp.cpp
```

- `win_stub.h` —— 提供 `DWORD` / `_popen` / `_pclose` / `Sleep` 的 Linux 桩实现
- `win_stub/windows.h`、`win_stub/io.h` —— 吸收掉 Windows 专有头文件的 `#include`

**注意**：这**只是语法检查**，不能替代在真机上的编译。
真正的 Windows 验证由 `.github/workflows/build.yml` 里的 `build-windows` job 完成
（在 GitHub 的 windows runner 上真编译 + 跑测试）。
