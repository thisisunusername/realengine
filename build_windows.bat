@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion
REM ============================================================
REM  RealEngine - Windows 构建脚本
REM
REM  支持的编译器（按优先级自动探测）:
REM    1. LLVM-MinGW  的 clang++      <-- 推荐
REM    2. MinGW-w64   的 g++
REM    3. MSVC        的 cl
REM
REM  用法:
REM    build_windows.bat            编译
REM    build_windows.bat test       编译 + 跑示例测试
REM    build_windows.bat clean      清理
REM    build_windows.bat CXX=g++    手动指定编译器
REM ============================================================

cd /d "%~dp0"

set "OUT=out"
set "SRC=src"
set "BIN=%OUT%\interp_windows_x86_64.exe"

REM ---------- 参数: clean ----------
if /i "%~1"=="clean" (
    if exist "%OUT%" rmdir /s /q "%OUT%"
    echo   [OK] 已清理
    exit /b 0
)

REM ---------- 参数: CXX=xxx 手动指定 ----------
set "CXX="
set "CXXKIND="
if not "%~1"=="" (
    echo %~1 | findstr /b /i "CXX=" >nul
    if !errorlevel!==0 (
        for /f "tokens=2 delims==" %%a in ("%~1") do set "CXX=%%a"
    )
)

REM ---------- 自动探测编译器 ----------
if not defined CXX (
    REM 优先 llvm-mingw，其次 MinGW，最后 MSVC
    where clang++.exe >nul 2>nul
    if !errorlevel!==0 (
        set "CXX=clang++"
        set "CXXKIND=clang"
    ) else (
        where g++.exe >nul 2>nul
        if !errorlevel!==0 (
            set "CXX=g++"
            set "CXXKIND=gcc"
        ) else (
            where cl.exe >nul 2>nul
            if !errorlevel!==0 (
                set "CXX=cl"
                set "CXXKIND=msvc"
            )
        )
    )
)

if not defined CXX (
    echo   [错误] 没找到 C++ 编译器
    echo.
    echo   请确认以下任一在 PATH 中:
    echo     clang++   LLVM-MinGW / MSYS2
    echo     g++       MinGW-w64
    echo     cl        Visual Studio
    echo.
    echo   或手动指定:  build_windows.bat CXX=clang++
    exit /b 1
)

if not defined CXXKIND set "CXXKIND=custom"
echo   [信息] 编译器: %CXX%

if not exist "%OUT%" mkdir "%OUT%"

REM ---------- 编译 ----------
echo   [阶段] 编译中...
if /i "%CXXKIND%"=="msvc" (
    %CXX% /nologo /std:c++17 /O2 /EHsc /D_CRT_SECURE_NO_WARNINGS /Isrc /Fo:%OUT%\ /Fe:%BIN% %SRC%\interp.cpp
) else (
    %CXX% -std=c++17 -O2 -Wall -Wextra -Isrc -o %BIN% %SRC%\interp.cpp
)

if not exist "%BIN%" (
    echo   [错误] 编译失败
    exit /b 1
)
echo   [OK] %BIN%

REM ---------- 测试 ----------
set "DOTEST=0"
if /i "%~1"=="test" set "DOTEST=1"
if /i "%~2"=="test" set "DOTEST=1"
if "!DOTEST!"=="0" goto :done

echo.
echo   === 交互测试 ===
set "TMPIN=%TEMP%\re_test_input.txt"
(
    echo whoami
    echo seq 2 4
    echo exit
) > "%TMPIN%"

echo   --- 运行 examples\ashell.re ---
"%BIN%" "examples\ashell.re" < "%TMPIN%"
set "RC=!errorlevel!"
del "%TMPIN%" >nul 2>nul

if !RC!==0 (
    echo   [OK] 测试通过
) else (
    echo   [警告] 退出码 !RC!
)

:done
endlocal