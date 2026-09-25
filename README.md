# RealEngine

**一门小巧的元编程 DSL —— 用 C++17 写的树遍历解释器**

> 代码由 **DeepSeek** 编写，语言设计来自一份手写的语法草稿。

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
$ ./out/interp examples/helloworld.re
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
make            # 编译到 out/interp
make test       # 跑全部示例
make clean
```

或者手动：

```sh
mkdir -p out
g++ -std=c++17 -O0 -Wall -Isrc -o out/interp src/interp.cpp
```

> **关于 `-O0`**：单翻译单元（`interp.cpp` 包含全部头文件）在开了优化后
> 编译期内存占用会涨到 1–2 GB，在手机上（尤其是同时跑着别的应用时）
> 容易被系统杀掉。默认构建因此用 `-O0`。想要优化版：
>
> ```sh
> make CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -Isrc"
> ```
>
> 内存实在紧张时，再加 `-fno-var-tracking -fno-var-tracking-assignments
> -fno-inline --param ggc-min-expand=10 --param ggc-min-heapsize=8192`。

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
