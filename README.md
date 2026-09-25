# RealEngine

**一门小巧的元编程 DSL —— 用 C++17 写的树遍历解释器**

> 代码由 **DeepSeek** 编写，语言设计来自一份手写的语法草稿。

---

## ⚠️ 先说清楚：这只是个玩具

**RealEngine 不是、也不打算成为一门能用的编程语言。**

它是个玩具语言，性质上跟 **Brainfuck** 差不多 —— 存在的意义是**好玩**、
是**折腾**、是拿来理解「一门语言是怎么从零跑起来的」。

请不要用它写正经程序：

- 它**不能**代替任何真正的编程语言（C / C++ / Python / Rust ...）
- 它**没有**生态、没有标准库规范、没有包管理、没有工具链
- 它**没有**实际工程价值，作者也不为生产环境使用导致的任何后果负责
- 它的性能、健壮性、错误提示都**只是能跑**的水平

如果你需要一个能干活的工具，请左转去用成熟语言。
但如果你也好奇「解释器到底长什么样」—— 那欢迎往下读。

---

## 这是什么

RealEngine 是一门**任务编排 DSL**，核心思想一句话：

> **程序 = 一堆可以被字符串描述的动作**

最能说明问题的是 `do="..."` 这个语法：

```re
create_loop "main"; in loop "main" do="
  execve('./libs/Main')
  wait 3 second
"
start_loop "main"
```

循环体**不是**一段语法块，而是**一个大字符串** —— 它在运行时才被送给词法器解析。
这在正经语言里叫 **homoiconicity（同像性）**，Lisp 家族才有这种味道。

换句话说：**代码即数据，数据即代码。**

---

## Hello World

```re
function_add "main"(type=main){printf("Hello world!")}
start_function "main"
```

```console
$ ./out/interp_linux_arm64 examples/helloworld.re
=== RealEngine 解释器 ===
文件: examples/helloworld.re (77 字节)
Token 数: 15
解析完成：2 条顶层语句
[定义函数] main (type=main) 共 1 条语句
[调用函数] main
Hello world!
=== 解释结束 ===
```

---

## 构建

需要 **C++17** 编译器（g++ / clang++ 均可）。

```sh
make            # 编译，产物名自动带平台：out/interp_<系统>_<架构>
make test       # 跑全部示例
make clean
```

产物名由 `uname` 自动决定：

| 平台 | 产物名 |
|---|---|
| Linux / aarch64 | `out/interp_linux_aarch64` |
| Linux / x86_64 | `out/interp_linux_x86_64` |

或者手动：

```sh
mkdir -p out
g++ -std=c++17 -O2 -Wall -Isrc -o out/interp_linux_aarch64 src/interp.cpp
```

### 发布用二进制

Release 附件是 **strip 过**的版本，构建方式：

```sh
make release    # -O2 编译并 strip，产物可直接上传 Release
```

### 预编译二进制

懒得编译的话，去 [Releases](../../releases) 下载附件：

| 附件 | 平台 | 说明 |
|---|---|---|
| `interp_linux_arm64` | Linux aarch64 | glibc，跑在普通 Linux / proot 里 |
| `interp_android_arm64` | Android arm64 | 链接 `/system/bin/linker64`，跑在 Termux 等 Android 原生环境 |

```sh
chmod +x interp_linux_arm64
./interp_linux_arm64 examples/helloworld.re
```

**注意两个二进制不能混用** —— Linux 版依赖 glibc，Android 版依赖 bionic。在 Android 上跑 Linux 版会报 `required file not found`（找不到 `ld-linux-aarch64.so.1`），反过来同理。

Android 上的用法：

```sh
chmod +x interp_android_arm64
./interp_android_arm64 examples/helloworld.re
```

---

## 用法

```sh
./out/interp examples/helloworld.re
./out/interp examples/ashell.re
```

直接把 `.re` 文件喂给它，边遍历 AST 边执行，无中间产物。

---

## 目录结构

```
realengine/
├── README.md          ← 本文件
├── Makefile           ← 构建脚本
├── src/
│   ├── token.hpp      ← 词法单元：Tk / Token
│   ├── keyword.hpp    ← 关键字表
│   ├── frontend.hpp   ← 词法器 + 语法器 + AST
│   └── interp.cpp     ← 解释器：遍历 AST 直接执行
├── libs/              ← .reh 库（被 #import 加载）
│   ├── linux.reh      ← 外部程序调用
│   ├── pwd.reh        ← 工作目录
│   ├── base.reh       ← 基础工具
│   └── gnu.reh        ← GNU 工具集
└── examples/
    ├── helloworld.re  ← 最小示例
    ├── hello.re       ← 字面量与退出
    ├── features.re    ← 全语法特性演示
    ├── main.re        ← 循环 + 分支综合示例
    ├── import_test.re ← 演示 #import
    └── ashell.re      ← 手写稿：一个 Shell 的骨架
```

---

## `#import` 与 `.reh` 库

`.reh` 就是 RealEngine 的头文件，写法与 `.re` 完全一样，只是**约定用于放可复用的函数**。

```re
#tips MY_PROGRAM
#import <pwd.reh>
#import <gnu.reh>

printf("pwd=")
start_function "pwd_cat"
exit
```

**搜索顺序**（找到第一个就停）：

1. 脚本所在目录
2. 脚本目录下的 `libs/`
3. 当前工作目录的 `libs/`
4. 当前工作目录的 `include/`

同一个 `.reh` 只会被加载一次（按名字去重），找不到时打印 `(未找到，跳过)` 而**不中断**。

`libs/` 里预置的四个库都遵循一个约定：函数名以 `rel_` 开头，参数从 `$INPUT_OPT` 取。

| 库 | 提供的函数 |
|---|---|
| `linux.reh` | `rel_echo` `rel_cat` `rel_sh` `rel_exit` |
| `pwd.reh` | `pwd_cat` `rel_cd` |
| `base.reh` | `rel_true` `rel_false` `rel_sleep` `rel_which` |
| `gnu.reh` | `rel_ls` `rel_whoami` `rel_uname` `rel_id` |

---

## 语法速览

### 定义与装配

| 语句 | 含义 |
|---|---|
| `function_add "名字"(type=main){ ... }` | 定义函数，花括号块 |
| `function_add "名字"(type=main) do="..."` | 定义函数，字符串块 |
| `join_function "名字"` | 指定主函数 |
| `add_other_function "a,b"` | 附加一组函数 |
| `start_function "名字"` | 调用函数 |
| `form_function="名字"` | 设置当前执行的函数 |

### 循环

| 语句 | 含义 |
|---|---|
| `create_loop "名字"; in loop "名字" do="..."` | 创建循环（创建与启动分离） |
| `start_loop "名字"` | 启动循环 |
| `kill "名字"` | 结束 |

### 动作与流程

| 语句 | 含义 |
|---|---|
| `execve("程序", "参数1", "参数2")` | 执行外部程序，退出码记入 `$up_cmd_exitcode` |
| `wait 3 second` | 等待 |
| `wait_input` | 等待输入 |
| `if $x is 0; then do="..."; else do="..."; end_if` | 分支 |
| `let $var = 0` | 赋值 |
| `set PS1="..."` | 设置提示符 |
| `printf("...")` | 输出 |
| `exit` | 程序结束 |

### 指令与注释

| 写法 | 含义 |
|---|---|
| `#tips 名字` | 顶部标记（原稿中声明这是 RealEngine 程序） |
| `#import <xxx.reh>` | 导入 |
| `// 说明` | 单行注释 |

### 字面量与取值

| 写法 | 含义 |
|---|---|
| `"..."` | 普通字符串 |
| `'...'` | 单引号字符串 |
| `$"..."$` | 美元字面量 |
| `$"""..."""$` | 美元字面量（产出单个引号） |
| `$NAME` | 变量 / 环境值引用 |
| `$execute_command 命令` | 命令替换，取命令输出 |
| `"a" + "b"` | 字符串拼接 |
| `get_keyboard ABC,...` | 声明键盘输入字符集 |
| `$INPUT is "*suffix"` | `*` 后缀通配匹配 |

---

## `do="..."` 的收尾规则

这是实现里最容易出 bug 的地方。

| 形态 | 收尾判定 | 例子 |
|---|---|---|
| 多行块 | **行首引号**收尾 | `do="` ↵ `...` ↵ `"` |
| 单行块 | **括号计数归零**后遇同款引号收尾 | `do="printf("x")"` |

**为什么单行块要数括号**：`do="printf("A shell")"` 里内层就有双引号，
若遇到第一个引号就收尾，块会在 `printf(` 处被截断。
所以解释器跟踪括号深度 —— 只有深度回到 0 时遇到的引号才算块尾。

**约定**：内层字符串**优先用单引号**，最省事。

---

## 实现要点

- **单文件前端**：词法 + 语法 + AST 全在 `src/frontend.hpp`
- **运行时二次解析**：`do="..."` 的内容在 `execCode()` 里重新走一遍词法 → 语法 → 执行，
  这就是同像性的落地点
- **变量展开**：`expand()` 处理 `$execute_command cmd`（调用 `popen`）与 `$NAME`
- **条件求值**：`evalCond()` 按 `" is "` 切分，支持 `*suffix` 后缀匹配
- **递归保护**：函数调用深度上限 64，防无限递归

---

## 许可

MIT
