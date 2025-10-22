@echo off
REM AES-SM3完整性校验算法测试运行脚本 (Windows版本)
REM 
REM 用法：
REM   run_tests.bat          # 运行完整测试套件
REM   run_tests.bat quick    # 快速测试（跳过长时间测试）

setlocal enabledelayedexpansion

echo ============================================================
echo   AES-SM3完整性校验算法 - 测试编译和运行脚本 (Windows)
echo ============================================================
echo.

REM 检查编译器
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到GCC编译器
    echo 请安装MinGW-w64或MSYS2
    exit /b 1
)

echo [+] 检测到GCC编译器
gcc --version | findstr "gcc"
echo.

echo ============================================================
echo   步骤1: 编译测试程序
echo ============================================================
echo.

REM 编译选项（简化版，不使用ARM特定选项）
set COMPILE_FLAGS=-O3 -funroll-loops -ftree-vectorize -finline-functions -pthread

echo 编译测试程序...
echo 编译选项: %COMPILE_FLAGS%
echo.

echo 步骤1: 编译主算法文件为目标文件...
gcc %COMPILE_FLAGS% -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm 2>compile_error.log

if %errorlevel% equ 0 (
    echo [+] 主算法文件编译成功！
    del compile_error.log 2>nul
) else (
    echo [-] 编译失败！
    echo.
    echo 错误信息:
    type compile_error.log
    del compile_error.log 2>nul
    exit /b 1
)

echo.
echo 步骤2: 编译测试文件并链接...
gcc %COMPILE_FLAGS% -o test_aes_sm3.exe aes_sm3_integrity.o test_aes_sm3_integrity.c -lm 2>compile_error.log

if %errorlevel% equ 0 (
    echo [+] 测试程序链接成功！
    del compile_error.log 2>nul
) else (
    echo [-] 链接失败！
    echo.
    echo 错误信息:
    type compile_error.log
    del compile_error.log 2>nul
    exit /b 1
)

echo.
echo ============================================================
echo   步骤2: 运行测试套件
echo ============================================================
echo.

REM 检查是否为快速测试模式
if "%1"=="quick" (
    echo [快速测试模式]
    echo.
)

REM 运行测试
test_aes_sm3.exe

set TEST_EXIT_CODE=%errorlevel%

echo.
echo ============================================================
echo   测试完成
echo ============================================================
echo.

if %TEST_EXIT_CODE% equ 0 (
    echo [+] 所有测试通过！
    echo.
    echo 测试可执行文件: test_aes_sm3.exe
    echo 可直接运行: test_aes_sm3.exe
) else (
    echo [-] 部分测试失败，退出码: %TEST_EXIT_CODE%
    exit /b %TEST_EXIT_CODE%
)

echo.
echo 注意: Windows版本不支持ARM硬件加速特性
echo 建议在ARMv8.2平台（如华为云KC2）上进行完整测试
echo.

endlocal

