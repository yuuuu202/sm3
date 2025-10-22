#!/bin/bash
# AES-SM3完整性校验算法测试运行脚本
# 
# 用法：
#   ./run_tests.sh          # 运行完整测试套件
#   ./run_tests.sh quick    # 快速测试（跳过长时间测试）

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  AES-SM3完整性校验算法 - 测试编译和运行脚本${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo ""

# 检查是否在ARMv8平台上
ARCH=$(uname -m)
if [[ ! "$ARCH" =~ "aarch64" && ! "$ARCH" =~ "arm" ]]; then
    echo -e "${YELLOW}警告: 当前架构为 $ARCH，不是ARMv8/ARM平台${NC}"
    echo -e "${YELLOW}测试可能无法正常运行或性能不佳${NC}"
    echo ""
fi

# 检查编译器
if ! command -v gcc &> /dev/null; then
    echo -e "${RED}错误: 未找到GCC编译器${NC}"
    exit 1
fi

GCC_VERSION=$(gcc --version | head -n1)
echo -e "${GREEN}✓${NC} 编译器: $GCC_VERSION"

# 检查CPU特性
if [ -f /proc/cpuinfo ]; then
    echo -e "${GREEN}✓${NC} CPU信息:"
    
    # 检查NEON支持
    if grep -q "asimd" /proc/cpuinfo; then
        echo -e "  ${GREEN}✓${NC} NEON SIMD支持"
    else
        echo -e "  ${RED}✗${NC} 不支持NEON SIMD"
    fi
    
    # 检查Crypto扩展
    if grep -q "aes" /proc/cpuinfo && grep -q "sha2" /proc/cpuinfo; then
        echo -e "  ${GREEN}✓${NC} Crypto扩展支持 (AES/SHA2)"
    else
        echo -e "  ${YELLOW}⚠${NC} 部分或不支持Crypto扩展"
    fi
    
    # 显示CPU型号
    CPU_MODEL=$(grep "model name" /proc/cpuinfo | head -n1 | cut -d: -f2 | xargs)
    if [ -z "$CPU_MODEL" ]; then
        CPU_MODEL=$(grep "CPU implementer" /proc/cpuinfo | head -n1 | cut -d: -f2 | xargs)
    fi
    echo -e "  CPU型号: $CPU_MODEL"
    
    # 显示CPU核心数
    CPU_CORES=$(grep -c "processor" /proc/cpuinfo)
    echo -e "  CPU核心数: $CPU_CORES"
fi

echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  步骤1: 编译测试程序${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo ""

# 编译选项
COMPILE_FLAGS="-march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread"

# 备选编译选项（如果不支持某些特性）
FALLBACK_FLAGS="-march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread"

echo "编译测试程序..."
echo "编译选项: $COMPILE_FLAGS"

# 尝试编译
if gcc $COMPILE_FLAGS -o test_aes_sm3 aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c -lm 2>compile_error.log; then
    echo -e "${GREEN}✓${NC} 编译成功！"
    rm -f compile_error.log
else
    echo -e "${YELLOW}⚠${NC} 使用默认编译选项失败，尝试备选方案..."
    
    if gcc $FALLBACK_FLAGS -o test_aes_sm3 aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c -lm 2>compile_error.log; then
        echo -e "${GREEN}✓${NC} 使用备选编译选项成功！"
        rm -f compile_error.log
    else
        echo -e "${RED}✗${NC} 编译失败！"
        echo ""
        echo "错误信息:"
        cat compile_error.log
        rm -f compile_error.log
        exit 1
    fi
fi

echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  步骤2: 运行测试套件${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo ""

# 检查是否为快速测试模式
if [ "$1" == "quick" ]; then
    echo -e "${YELLOW}快速测试模式（跳过部分耗时测试）${NC}"
    echo ""
fi

# 运行测试
if [ "$1" == "quick" ]; then
    # 快速测试模式：使用timeout限制时间
    timeout 120 ./test_aes_sm3 || {
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 124 ]; then
            echo -e "${YELLOW}测试超时（2分钟）${NC}"
            exit 1
        else
            exit $EXIT_CODE
        fi
    }
else
    # 完整测试模式
    ./test_aes_sm3
fi

EXIT_CODE=$?

echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  测试完成${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════${NC}"
echo ""

if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ 所有测试通过！${NC}"
    echo ""
    echo "测试可执行文件: ./test_aes_sm3"
    echo "可直接运行: ./test_aes_sm3"
else
    echo -e "${RED}✗ 部分测试失败，退出码: $EXIT_CODE${NC}"
    exit $EXIT_CODE
fi

echo ""
echo "清理编译产物？"
echo "  保留测试程序: test_aes_sm3"
echo "  运行 'make clean' 或 'rm test_aes_sm3' 清理"
echo ""

