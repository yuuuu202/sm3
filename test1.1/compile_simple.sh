#!/bin/bash

# 简化编译脚本 - 使用通用编译选项
echo "编译AES-SM3完整性验证算法 (通用版本)..."
echo ""

# 使用更通用的编译选项
gcc -O3 -march=native -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm

if [ $? -eq 0 ]; then
    echo "✅ 编译成功!"
    echo "可执行文件: aes_sm3_integrity_opt"
    echo ""
    echo "运行程序:"
    echo "./aes_sm3_integrity_opt"
else
    echo "❌ 编译失败!"
    echo ""
    echo "请检查以下内容:"
    echo "1. 是否安装了GCC编译器"
    echo "2. 是否安装了OpenSSL开发库 (libssl-dev)"
    echo "3. 系统是否支持ARM NEON指令集"
    echo ""
    echo "安装命令 (Ubuntu/Debian):"
    echo "sudo apt-get update"
    echo "sudo apt-get install gcc libssl-dev"
fi