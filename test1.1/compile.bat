@echo off
REM AES-SM3 完整性校验算法 - 快速编译脚本 (Windows)
REM 解决 multiple definition of main 问题

setlocal enabledelayedexpansion

echo ============================================================
echo   AES-SM3 测试套件 - 快速编译脚本 (Windows)
echo ============================================================
echo.

REM 检查编译器
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [X] 错误: 未找到 GCC 编译器
    echo 请安装 MinGW-w64 或 MSYS2
    echo.
    echo 下载地址:
    echo   MinGW-w64: https://www.mingw-w64.org/
    echo   MSYS2:     https://www.msys2.org/
    exit /b 1
)

echo [+] GCC 版本:
gcc --version | findstr "gcc"
echo.

REM 清理旧文件
echo 清理旧的编译文件...
if exist aes_sm3_integrity.o del aes_sm3_integrity.o
if exist test_aes_sm3.exe del test_aes_sm3.exe
echo [+] 清理完成
echo.

REM 编译选项
set COMPILE_FLAGS=-O3 -funroll-loops -ftree-vectorize -finline-functions -pthread

echo 编译选项: %COMPILE_FLAGS%
echo.

REM 步骤1: 编译主算法文件
echo 步骤 1/2: 编译主算法文件...
gcc %COMPILE_FLAGS% -c aes_sm3_integrity.c -o aes_sm3_integrity.o 2>compile_error.log

if %errorlevel% equ 0 (
    echo [+] 主算法文件编译成功
    del compile_error.log 2>nul
) else (
    echo [X] 主算法文件编译失败
    echo.
    echo 错误信息:
    type compile_error.log
    del compile_error.log 2>nul
    exit /b 1
)
echo.

REM 步骤2: 编译测试文件并链接
echo 步骤 2/2: 编译测试文件并链接...
gcc %COMPILE_FLAGS% -o test_aes_sm3.exe aes_sm3_integrity.o test_aes_sm3_integrity.c -lm 2>compile_error.log

if %errorlevel% equ 0 (
    echo [+] 测试程序链接成功
    del compile_error.log 2>nul
) else (
    echo [X] 测试程序链接失败
    echo.
    echo 错误信息:
    type compile_error.log
    del compile_error.log 2>nul
    exit /b 1
)
echo.

echo ============================================================
echo   [+] 编译完成！
echo ============================================================
echo.
echo 运行测试:
echo   test_aes_sm3.exe
echo.
echo 清理编译产物:
echo   del *.o test_aes_sm3.exe
echo.
echo 注意: Windows 版本不支持 ARM 硬件加速特性
echo 建议在 ARMv8 平台上进行完整性能测试
echo.

endlocal

