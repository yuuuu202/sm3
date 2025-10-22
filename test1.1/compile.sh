#!/bin/bash
# AES-SM3 完整性校验算法 - 快速编译脚本
# 解决 multiple definition of main 问题

set -e

echo "════════════════════════════════════════════════════════"
echo "  AES-SM3 测试套件 - 快速编译脚本"
echo "════════════════════════════════════════════════════════"
echo ""

# 检查编译器
if ! command -v gcc &> /dev/null; then
    echo "❌ 错误: 未找到 GCC 编译器"
    echo "请先安装 GCC: sudo apt-get install gcc"
    exit 1
fi

echo "✓ GCC 版本: $(gcc --version | head -n1)"
echo ""

# 清理旧文件（重要：清理所有 .o 文件避免 LTO 残留）
echo "清理旧的编译文件..."
rm -f aes_sm3_integrity.o test_aes_sm3 test_aes_sm3.exe *.o
echo "✓ 清理完成"
echo ""

# 检测平台并设置编译选项
ARCH=$(uname -m)
if [[ "$ARCH" =~ "aarch64" || "$ARCH" =~ "arm" ]]; then
    echo "检测到 ARM 平台，使用硬件加速优化"
    # 尝试 ARMv8.2-A 优化（不使用 -flto，避免 LTO 导致的符号冲突）
    COMPILE_FLAGS="-march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize -finline-functions -ffast-math -fomit-frame-pointer"
    
    # 测试是否支持
    if ! gcc $COMPILE_FLAGS -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm 2>/dev/null; then
        echo "⚠ ARMv8.2-A 不支持，降级到 ARMv8-A"
        COMPILE_FLAGS="-march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize -finline-functions"
    fi
else
    echo "检测到 x86 平台，使用通用优化（性能会较低）"
    COMPILE_FLAGS="-O3 -funroll-loops -ftree-vectorize -finline-functions"
fi

echo "编译选项: $COMPILE_FLAGS"
echo ""

# 步骤1: 编译主算法文件
echo "步骤 1/2: 编译主算法文件..."
gcc $COMPILE_FLAGS -pthread -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm

if [ $? -eq 0 ]; then
    echo "✓ 主算法文件编译成功"
else
    echo "❌ 主算法文件编译失败"
    exit 1
fi
echo ""

# 步骤2: 编译测试文件并链接
echo "步骤 2/2: 编译测试文件并链接..."
gcc $COMPILE_FLAGS -pthread -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

if [ $? -eq 0 ]; then
    echo "✓ 测试程序链接成功"
else
    echo "❌ 测试程序链接失败"
    exit 1
fi
echo ""

echo "════════════════════════════════════════════════════════"
echo "  ✓ 编译完成！"
echo "════════════════════════════════════════════════════════"
echo ""
echo "运行测试："
echo "  ./test_aes_sm3"
echo ""
echo "清理编译产物："
echo "  rm -f *.o test_aes_sm3"
echo ""

