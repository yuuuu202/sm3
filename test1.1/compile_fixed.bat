@echo off
REM AES-SM3完整性验证算法编译脚本 - 修复版本 (Windows)
REM 适用于ARMv8架构服务器

echo 编译AES-SM3完整性验证算法 (修复版本)...
echo 修复内容：
echo 1. 添加函数前向声明
echo 2. 添加NEON函数兼容性定义
echo 3. 优化编译选项
echo.

REM 检查是否在WSL环境中
where wsl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo 检测到WSL环境，尝试在WSL中编译...
    wsl -d Ubuntu bash -c "cd /mnt/d/Dev/SYF/cn_test1.1/test1.1 && chmod +x compile_fixed.sh && ./compile_fixed.sh"
    if %ERRORLEVEL% EQU 0 (
        echo ✅ WSL编译成功！
        goto :end
    ) else (
        echo ⚠️ WSL编译失败，尝试直接编译...
    )
)

REM 检查GCC是否可用
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ 未找到GCC编译器，请安装MinGW-w64或使用WSL
    echo 推荐安装方案：
    echo 1. 安装WSL (Windows Subsystem for Linux)
    echo 2. 在WSL中安装: sudo apt install gcc
    echo 3. 运行: wsl bash -c "cd /mnt/d/Dev/SYF/cn_test1.1/test1.1 && ./compile_fixed.sh"
    goto :end
)

REM 直接在Windows上编译
echo 在Windows上直接编译...
echo 注意：Windows上可能缺少ARM NEON支持，编译可能会有警告

REM 尝试基本编译
gcc -O3 -pthread -o aes_sm3_integrity_fixed.exe aes_sm3_integrity.c -lm
if %ERRORLEVEL% EQU 0 (
    echo ✅ Windows编译成功！
    echo 可执行文件: aes_sm3_integrity_fixed.exe
    echo.
    echo 运行程序:
    echo aes_sm3_integrity_fixed.exe
) else (
    echo ❌ Windows编译失败
    echo 建议使用WSL环境进行编译
)

:end
pause