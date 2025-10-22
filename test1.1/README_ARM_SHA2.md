# AES-SM3完整性验证算法 - ARMv8 SHA2硬件加速版

## 概述

本项目实现了面向4KB消息长度的高性能完整性校验算法，采用XOR+SM3混合方案，并针对ARMv8架构进行了深度优化。

## 主要特性

- **v2.3版本**: 实现了超级预取策略和流水线预取策略
- **SHA2硬件加速**: 使用ARMv8 SHA2指令集进行性能对比
- **批处理优化**: 一次处理多个4KB块，提高吞吐量
- **SIMD向量化**: 使用NEON指令集进行并行处理

## 系统要求

- ARMv8架构处理器 (aarch64)
- 支持SHA2指令集 (如华为鲲鹏处理器)
- GCC编译器
- OpenSSL开发库 (libssl-dev)

## 编译方法

### 方法1: 使用SHA2硬件加速编译 (推荐)

```bash
chmod +x compile_arm_sha2.sh
./compile_arm_sha2.sh
```

### 方法2: 使用通用编译选项

```bash
chmod +x compile_simple.sh
./compile_simple.sh
```

### 方法3: 手动编译

```bash
# SHA2硬件加速版本
gcc -O3 -march=armv8.2-a+crypto+sha2 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -fomit-frame-pointer \
    -pthread -D_GNU_SOURCE -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm

# 通用版本
gcc -O3 -march=native -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm
```

## 运行程序

```bash
./aes_sm3_integrity_opt
```

## 性能预期

- **vs 软件SHA256**: 15-20x 加速
- **vs 硬件SHA256**: 8-12x 加速
- **绝对吞吐率**: 20,000-35,000 MB/s

## 故障排除

### 编译错误: "SHA2硬件加速不可用"

如果遇到此错误，请尝试以下解决方案:

1. **使用通用编译选项**:
   ```bash
   gcc -O3 -march=native -o aes_sm3_integrity_opt aes_sm3_integrity.c -lm
   ```

2. **检查CPU支持**:
   ```bash
   lscpu | grep sha2
   ```

3. **检查编译器版本**:
   ```bash
   gcc --version
   ```

4. **安装必要的库** (Ubuntu/Debian):
   ```bash
   sudo apt-get update
   sudo apt-get install gcc libssl-dev
   ```

## 技术细节

### SHA2硬件加速检测

代码使用以下条件检测SHA2硬件支持:
```c
#if (defined(__ARM_FEATURE_SHA2) || defined(__aarch64__)) && defined(__aarch64__)
```

如果您的编译器没有定义`__ARM_FEATURE_SHA2`宏，代码会回退到使用`__aarch64__`作为检测条件。

### 优化技术

1. **超级预取策略**: 提前预取多个后续数据块
2. **流水线预取策略**: 使用双缓冲技术实现流水线处理
3. **SoA布局**: 提高缓存局部性
4. **NEON指令集**: 充分利用SIMD并行处理能力

## 许可证

请参阅LICENSE文件了解许可证信息。