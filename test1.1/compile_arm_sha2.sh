#!/bin/bash

# 编译脚本 - 针对ARMv8架构的SHA2硬件加速优化
# 适用于支持SHA2指令的ARM服务器

echo "编译AES-SM3完整性验证算法 (ARMv8 SHA2硬件加速版)..."
echo "目标架构: ARMv8.2-a with SHA2指令集"
echo "编译器: GCC with ARM NEON支持"
echo ""

# 设置编译选项
COMPILE_FLAGS="-O3 -march=armv8.2-a+crypto+sha2 -funroll-loops -ftree-vectorize"
COMPILE_FLAGS="$COMPILE_FLAGS -finline-functions -ffast-math -flto -fomit-frame-pointer"
COMPILE_FLAGS="$COMPILE_FLAGS -pthread -D_GNU_SOURCE"

# 编译命令
gcc $COMPILE_FLAGS -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm

if [ $? -eq 0 ]; then
    echo "✅ 编译成功!"
    echo "可执行文件: aes_sm3_integrity_opt"
    echo ""
    echo "运行程序:"
    echo "./aes_sm3_integrity_opt"
else
    echo "❌ 编译失败!"
    echo ""
    echo "如果编译失败，请尝试以下替代方案:"
    echo "1. 使用更通用的编译选项:"
    echo "   gcc -O3 -march=native -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm"
    echo ""
    echo "2. 检查GCC版本是否支持ARMv8.2-a架构:"
    echo "   gcc --version"
    echo "   aarch64-linux-gnu-gcc --version"
    echo ""
    echo "3. 检查系统是否支持SHA2指令:"
    echo "   lscpu | grep sha2"
fi