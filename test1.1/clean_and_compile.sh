#!/bin/bash
# 紧急清理和编译脚本 - 解决 LTO 残留问题
# 用于彻底清理所有编译产物并重新编译

set -e

echo "════════════════════════════════════════════════════════"
echo "  紧急清理和重新编译 - 解决 LTO 插件冲突"
echo "════════════════════════════════════════════════════════"
echo ""

# 步骤1: 彻底清理所有编译产物
echo "步骤 1/3: 彻底清理所有编译产物..."
rm -f *.o
rm -f test_aes_sm3 test_aes_sm3.exe
rm -f a.out
rm -f compile_error.log
echo "✓ 清理完成"
echo ""

# 步骤2: 编译主算法文件（不使用 LTO）
echo "步骤 2/3: 编译主算法文件..."

# 检测平台
ARCH=$(uname -m)
if [[ "$ARCH" =~ "aarch64" || "$ARCH" =~ "arm" ]]; then
    echo "检测到 ARM 平台"
    # 尝试 ARMv8.2-A
    if gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
           -finline-functions -ffast-math -fomit-frame-pointer -pthread \
           -c aes_sm3_integrity.c -o aes_sm3_integrity.o 2>/dev/null; then
        echo "✓ 使用 ARMv8.2-A 优化编译成功"
        COMPILE_FLAGS="-march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize -finline-functions -ffast-math -fomit-frame-pointer -pthread"
    elif gcc -march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize \
             -finline-functions -pthread \
             -c aes_sm3_integrity.c -o aes_sm3_integrity.o 2>/dev/null; then
        echo "✓ 使用 ARMv8-A 优化编译成功"
        COMPILE_FLAGS="-march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread"
    else
        echo "⚠ 使用通用优化"
        gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
            -c aes_sm3_integrity.c -o aes_sm3_integrity.o
        COMPILE_FLAGS="-O3 -funroll-loops -ftree-vectorize -finline-functions -pthread"
    fi
else
    echo "检测到 x86 平台，使用通用优化"
    gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
        -c aes_sm3_integrity.c -o aes_sm3_integrity.o
    COMPILE_FLAGS="-O3 -funroll-loops -ftree-vectorize -finline-functions -pthread"
fi
echo ""

# 步骤3: 编译测试文件并链接
echo "步骤 3/3: 编译测试文件并链接..."
gcc $COMPILE_FLAGS -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

if [ $? -eq 0 ]; then
    echo "✓ 编译完全成功！"
else
    echo "✗ 编译失败"
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
echo "如果还有问题，请确保："
echo "  1. 没有使用 -flto 选项"
echo "  2. 已清理所有 .o 文件"
echo "  3. GCC 版本正常（gcc --version）"
echo ""

