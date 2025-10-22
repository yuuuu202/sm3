# AES-SM3完整性校验算法 - 编译指南

## 🔴 常见编译错误及解决方案

### 错误 1: Multiple definition of `main`

**错误信息（两种形式）：**

**形式 A：常规错误**
```
/usr/bin/ld: /tmp/ccXwBiBA.o: in function `main':
test_aes_sm3_integrity.c:(.text.startup+0x0): multiple definition of `main'; 
/tmp/ccD3A2Ts.o:aes_sm3_integrity.c:(.text.startup+0x0): first defined here
collect2: error: ld returned 1 exit status
```

**形式 B：LTO（链接时优化）错误**
```
/usr/bin/ld: /tmp/ccjVMrOW.o (symbol from plugin): in function `print_hash':
(.text+0x0): multiple definition of `main'; aes_sm3_integrity.o (symbol from plugin):(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```
> 如果看到 `(symbol from plugin)` 字样，说明是 `-flto` 选项导致的。

**原因：**
- `aes_sm3_integrity.c` 和 `test_aes_sm3_integrity.c` 两个文件都有 `main` 函数
- 直接同时编译这两个文件会导致链接器冲突
- 使用 `-flto`（链接时优化）会产生额外的符号冲突

**✅ 解决方案：分步编译 + 移除 -flto**

#### Linux/Unix 平台

```bash
# 步骤1: 将主算法文件编译为目标文件（.o文件，不链接）
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# 步骤2: 编译测试文件并链接目标文件
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 步骤3: 运行测试
./test_aes_sm3
```

#### Windows 平台

```batch
REM 步骤1: 编译主算法文件为目标文件
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

REM 步骤2: 编译测试文件并链接
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -o test_aes_sm3.exe aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

REM 步骤3: 运行测试
test_aes_sm3.exe
```

---

## 📋 完整编译方案

### 方案 1: 使用自动化脚本（强烈推荐）

这些脚本已经修复了 `main` 函数冲突问题，可以直接使用：

#### Linux/Unix
```bash
chmod +x run_tests.sh
./run_tests.sh
```

#### Windows
```batch
run_tests.bat
```

---

### 方案 2: 手动编译（各平台详细说明）

#### A. ARMv8.2-A 平台（最佳性能）

```bash
# 步骤1: 编译主算法文件（注意：不使用 -flto，避免 LTO 导致的符号冲突）
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -fomit-frame-pointer -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# 步骤2: 编译测试文件并链接
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -fomit-frame-pointer -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行测试
./test_aes_sm3
```

#### B. ARMv8-A 平台（基础版本）

```bash
# 步骤1: 编译主算法文件
gcc -march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm

# 步骤2: 编译测试并链接
gcc -march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行测试
./test_aes_sm3
```

#### C. x86/x64 平台（仅功能测试）

```bash
# 步骤1: 编译主算法文件
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm

# 步骤2: 编译测试并链接
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行测试
./test_aes_sm3
```

#### D. Windows MinGW/MSYS2

```batch
REM 步骤1: 编译主算法文件
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

REM 步骤2: 编译测试并链接
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -o test_aes_sm3.exe aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

REM 运行测试
test_aes_sm3.exe
```

---

---

### 错误 1b: LTO 导致的符号冲突

**错误信息特征：**
```
(symbol from plugin): multiple definition of `main'
```

**原因：**
`-flto`（Link Time Optimization，链接时优化）在处理包含多个 `main` 函数的项目时会产生符号冲突。

**✅ 解决方案：移除 -flto 选项**

```bash
# ❌ 错误：使用 -flto
gcc -O3 -flto -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# ✅ 正确：移除 -flto
gcc -O3 -c aes_sm3_integrity.c -o aes_sm3_integrity.o
```

**性能影响：**
- LTO 通常可以带来 5-10% 的性能提升
- 但在本项目中，由于符号冲突，必须禁用
- 其他优化选项（`-O3`, `-funroll-loops` 等）已经提供了足够的性能

---

## 🛠️ 其他常见编译错误

### 错误 2: 找不到 gcc 编译器

**错误信息：**
```
bash: gcc: command not found
```

**解决方案：**

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential gcc
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install gcc
```

**Windows:**
- 安装 MinGW-w64: https://www.mingw-w64.org/
- 或安装 MSYS2: https://www.msys2.org/

---

### 错误 3: 不支持的编译选项

**错误信息：**
```
gcc: error: unrecognized command line option '-march=armv8.2-a+crypto'
```

**原因：** GCC 版本过旧或不是 ARM 平台

**解决方案：**

1. **检查 GCC 版本：**
```bash
gcc --version
# 需要 GCC 8.0+ 才能完整支持 ARMv8.2
```

2. **升级 GCC（Ubuntu）：**
```bash
sudo apt-get update
sudo apt-get install gcc-10 g++-10
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100
```

3. **使用降级的编译选项：**
```bash
# 使用 ARMv8-A 而不是 ARMv8.2-A
gcc -march=armv8-a+crypto -O3 -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm
gcc -march=armv8-a+crypto -O3 -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm
```

---

### 错误 4: 找不到 pthread 库

**错误信息：**
```
undefined reference to `pthread_create'
```

**解决方案：**
```bash
# 确保添加 -pthread 标志
gcc -pthread -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm
```

---

### 错误 5: 找不到数学库函数

**错误信息：**
```
undefined reference to `sqrt'
```

**解决方案：**
```bash
# 确保添加 -lm 标志链接数学库
gcc -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm -pthread
```

---

## 📊 编译选项说明

### 优化级别

| 选项 | 说明 | 性能影响 | 推荐场景 |
|------|------|---------|---------|
| `-O0` | 无优化 | 最慢 | 调试 |
| `-O1` | 基础优化 | 较慢 | 开发 |
| `-O2` | 标准优化 | 较快 | 一般生产 |
| `-O3` | 高级优化 | 快 | **推荐：性能测试** |
| `-Ofast` | 激进优化 | 最快 | 极限性能 |

### 常用优化选项

| 选项 | 说明 | 效果 |
|------|------|------|
| `-funroll-loops` | 循环展开 | 提升 10-20% |
| `-ftree-vectorize` | 自动向量化 | 提升 15-30% |
| `-finline-functions` | 函数内联 | 提升 5-15% |
| `-ffast-math` | 快速数学运算 | 提升 5-10% |
| `-flto` | 链接时优化 | 提升 5-15% |
| `-fomit-frame-pointer` | 省略帧指针 | 提升 2-5% |

### 架构特定选项

| 平台 | 选项 | 说明 |
|------|------|------|
| ARMv8.2-A | `-march=armv8.2-a+crypto` | 支持完整硬件加速 |
| ARMv8-A | `-march=armv8-a+crypto` | 基础硬件加速 |
| 原生优化 | `-march=native` | 自动检测当前CPU |
| x86-64 | `-march=x86-64` | x86平台通用 |

---

## ✅ 快速检查清单

编译前请确认：

- [ ] 已安装 GCC 编译器（`gcc --version`）
- [ ] GCC 版本 >= 7.0（推荐 >= 9.0）
- [ ] 在 ARM 平台上测试性能（x86 仅能测试功能）
- [ ] 使用**分步编译**避免 main 函数冲突
- [ ] 添加 `-pthread` 和 `-lm` 链接库

---

## 🎯 一键编译脚本

创建一个简单的编译脚本 `compile.sh`：

```bash
#!/bin/bash
# 一键编译脚本

echo "编译 AES-SM3 测试套件..."

# 清理旧文件
rm -f aes_sm3_integrity.o test_aes_sm3

# 编译主算法文件
echo "步骤 1/2: 编译算法文件..."
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm || exit 1

# 编译测试文件
echo "步骤 2/2: 编译测试文件..."
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm || exit 1

echo "✓ 编译成功！"
echo "运行测试: ./test_aes_sm3"
```

使用方法：
```bash
chmod +x compile.sh
./compile.sh
./test_aes_sm3
```

---

## 📞 获取帮助

如果遇到其他编译问题：

1. 检查 GCC 版本：`gcc --version`
2. 检查 CPU 架构：`uname -m`
3. 检查 CPU 特性（ARM）：`cat /proc/cpuinfo | grep Features`
4. 查看完整错误信息并搜索解决方案
5. 使用自动化脚本 `run_tests.sh` 或 `run_tests.bat`

---

## 🚀 推荐的完整工作流程

```bash
# 1. 进入项目目录
cd test1.1

# 2. 清理旧文件（可选）
rm -f *.o test_aes_sm3 test_aes_sm3.exe

# 3. 使用自动化脚本（推荐）
chmod +x run_tests.sh
./run_tests.sh

# 或者手动编译
gcc -O3 -pthread -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm
gcc -O3 -pthread -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 4. 运行测试
./test_aes_sm3

# 5. 查看结果
# 测试会自动运行并显示结果
```

---

**编译成功后，你将看到类似如下的输出：**

```
╔══════════════════════════════════════════════════════════╗
║       AES-SM3完整性校验算法 - 综合测试套件               ║
║       Comprehensive Test Suite for AES-SM3 Integrity    ║
╚══════════════════════════════════════════════════════════╝

测试平台: ARMv8.2-A
...
✓ 所有测试通过！
```

祝测试顺利！🎉

