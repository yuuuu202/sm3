#!/bin/bash

# AES-SM3完整性验证算法编译脚本 - 修复版本
# 适用于ARMv8架构服务器

echo "编译AES-SM3完整性验证算法 (修复版本)..."
echo "修复内容："
echo "1. 添加函数前向声明"
echo "2. 添加NEON函数兼容性定义"
echo "3. 优化编译选项"
echo ""

# 检查架构
ARCH=$(uname -m)
echo "检测到系统架构: $ARCH"

# 设置编译选项
if [ "$ARCH" = "aarch64" ]; then
    # ARMv8架构特定优化
    COMMON_FLAGS="-O3 -march=native -funroll-loops -ftree-vectorize -finline-functions -ffast-math"
    SPECIFIC_FLAGS="-march=armv8.2-a+crypto+sha2 -flto -fomit-frame-pointer"
    echo "使用ARMv8.2-a+crypto+sha2优化..."
    
    # 尝试使用完整优化选项编译
    echo "尝试完整优化编译..."
    gcc $COMMON_FLAGS $SPECIFIC_FLAGS -pthread -o aes_sm3_integrity_fixed aes_sm3_integrity.c -lm 2>&1
    
    if [ $? -eq 0 ]; then
        echo "✅ 完整优化编译成功！"
        echo "编译选项: $COMMON_FLAGS $SPECIFIC_FLAGS"
    else
        echo "⚠️ 完整优化编译失败，尝试基本优化..."
        
        # 回退到基本优化选项
        gcc $COMMON_FLAGS -pthread -o aes_sm3_integrity_fixed aes_sm3_integrity.c -lm 2>&1
        
        if [ $? -eq 0 ]; then
            echo "✅ 基本优化编译成功！"
            echo "编译选项: $COMMON_FLAGS"
        else
            echo "❌ 编译失败，尝试最基本选项..."
            
            # 最基本编译选项
            gcc -O3 -pthread -o aes_sm3_integrity_fixed aes_sm3_integrity.c -lm 2>&1
            
            if [ $? -eq 0 ]; then
                echo "✅ 基本编译成功！"
            else
                echo "❌ 编译完全失败，请检查错误信息"
                exit 1
            fi
        fi
    fi
else
    echo "非ARM架构，使用通用编译选项..."
    
    # 非ARM架构通用编译
    gcc -O3 -pthread -o aes_sm3_integrity_fixed aes_sm3_integrity.c -lm 2>&1
    
    if [ $? -eq 0 ]; then
        echo "✅ 通用编译成功！"
    else
        echo "❌ 编译失败，请检查错误信息"
        exit 1
    fi
fi

echo ""
echo "编译完成！可执行文件: aes_sm3_integrity_fixed"
echo ""
echo "运行程序:"
echo "./aes_sm3_integrity_fixed"