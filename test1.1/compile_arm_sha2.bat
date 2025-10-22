@echo off
REM 编译脚本 - 针对ARMv8架构的SHA2硬件加速优化 (Windows版本)
REM 适用于WSL环境或交叉编译

echo 编译AES-SM3完整性验证算法 (ARMv8 SHA2硬件加速版)...
echo 目标架构: ARMv8.2-a with SHA2指令集
echo.

REM 检查是否在WSL环境中
wsl --list >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo 在WSL环境中编译...
    wsl bash -c "cd /mnt/d/Dev/SYF/cn_test1.1/test1.1 && chmod +x compile_arm_sha2.sh && ./compile_arm_sha2.sh"
    if %ERRORLEVEL% EQU 0 (
        echo ✅ 编译成功!
        echo 可执行文件: aes_sm3_integrity_opt
    ) else (
        echo ❌ 编译失败!
    )
) else (
    echo 未检测到WSL环境。
    echo.
    echo 请在ARM服务器上直接运行以下命令:
    echo gcc -O3 -march=armv8.2-a+crypto+sha2 -funroll-loops -ftree-vectorize -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread -D_GNU_SOURCE -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm
    echo.
    echo 或者使用通用编译选项:
    echo gcc -O3 -march=native -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm
)

pause