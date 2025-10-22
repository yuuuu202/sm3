# 编译方法总览

## 🚀 快速开始（推荐）

### Linux/Unix
```bash
./run_tests.sh
```

### Windows
```batch
run_tests.bat
```

---

## 📁 可用的编译脚本和文档

| 文件 | 平台 | 用途 | 推荐度 |
|------|------|------|--------|
| `run_tests.sh` | Linux/Unix | 自动编译+运行测试 | ⭐⭐⭐⭐⭐ |
| `run_tests.bat` | Windows | 自动编译+运行测试 | ⭐⭐⭐⭐⭐ |
| `compile.sh` | Linux/Unix | 仅编译，不运行 | ⭐⭐⭐⭐ |
| `compile.bat` | Windows | 仅编译，不运行 | ⭐⭐⭐⭐ |
| `QUICK_FIX.md` | 所有 | 错误快速修复指南 | ⭐⭐⭐⭐⭐ |
| `COMPILE_GUIDE.md` | 所有 | 详细编译文档 | ⭐⭐⭐⭐ |

---

## 🔥 遇到编译错误？

**如果看到 "multiple definition of main" 错误：**

👉 **立即查看：[QUICK_FIX.md](QUICK_FIX.md)** 👈

这个文档包含：
- ✅ 3种快速解决方案
- ✅ 一键命令
- ✅ 详细的分步说明

---

## 📋 编译方法对比

### 方法 1：自动化脚本（最简单）

**优点：**
- ✅ 一键搞定
- ✅ 自动检测平台
- ✅ 自动选择最优编译选项
- ✅ 自动运行测试
- ✅ 彩色输出，易于查看

**使用：**
```bash
# Linux/Unix
chmod +x run_tests.sh
./run_tests.sh

# Windows
run_tests.bat
```

---

### 方法 2：快速编译脚本（仅编译）

**优点：**
- ✅ 只编译，不运行测试
- ✅ 适合开发调试
- ✅ 可以单独运行可执行文件

**使用：**
```bash
# Linux/Unix
chmod +x compile.sh
./compile.sh
./test_aes_sm3  # 单独运行

# Windows
compile.bat
test_aes_sm3.exe
```

---

### 方法 3：手动编译（完全控制）

**优点：**
- ✅ 完全控制编译过程
- ✅ 自定义编译选项
- ✅ 适合集成到其他构建系统

**基础命令：**
```bash
# 步骤1: 编译算法文件为目标文件
gcc -O3 -pthread -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# 步骤2: 编译测试并链接
gcc -O3 -pthread -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 步骤3: 运行
./test_aes_sm3
```

**ARMv8 优化：**
```bash
# 使用硬件加速指令
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm
```

---

## ⚠️ 重要提示

### ❌ 不要这样做
```bash
# 这会导致 "multiple definition of main" 错误！
gcc -o test aes_sm3_integrity.c test_aes_sm3_integrity.c
```

### ✅ 正确的做法
```bash
# 必须分两步：先编译为 .o，再链接
gcc -c aes_sm3_integrity.c -o aes_sm3_integrity.o
gcc -o test aes_sm3_integrity.o test_aes_sm3_integrity.c
```

**原因：** 两个 .c 文件都有 `main()` 函数，必须用 `-c` 选项单独编译第一个文件。

---

## 🎯 推荐工作流程

### 第一次使用
```bash
# 1. 赋予脚本执行权限（仅Linux/Unix需要，仅一次）
chmod +x run_tests.sh compile.sh

# 2. 运行完整测试
./run_tests.sh

# 3. 查看结果
# 测试会自动运行并显示所有结果
```

### 日常开发
```bash
# 快速编译（不运行测试）
./compile.sh

# 手动运行测试
./test_aes_sm3

# 或只运行快速测试
./run_tests.sh quick
```

---

## 🔧 故障排除

### 问题 1: 找不到 gcc
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install gcc

# CentOS/RHEL
sudo yum install gcc

# Windows
# 下载安装 MinGW-w64 或 MSYS2
```

### 问题 2: 权限被拒绝
```bash
# 赋予执行权限
chmod +x run_tests.sh compile.sh
```

### 问题 3: multiple definition of main
```bash
# 查看快速修复指南
cat QUICK_FIX.md

# 或直接使用修复后的脚本
./run_tests.sh
```

### 问题 4: 编译选项不支持
```bash
# 使用简化的编译选项
gcc -O3 -c aes_sm3_integrity.c -o aes_sm3_integrity.o
gcc -O3 -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm
```

---

## 📊 性能优化建议

### 测试性能时的推荐设置

#### Linux
```bash
# 1. 设置CPU性能模式
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# 2. 使用完整优化编译
./run_tests.sh

# 3. 绑定到特定CPU核心（可选）
taskset -c 0 ./test_aes_sm3
```

#### Windows
```batch
REM 1. 关闭其他程序
REM 2. 运行测试
run_tests.bat
```

---

## 📚 详细文档

需要更多信息？查看：

- **编译错误？** → [QUICK_FIX.md](QUICK_FIX.md) - 快速解决方案
- **详细编译指南？** → [COMPILE_GUIDE.md](COMPILE_GUIDE.md) - 完整编译文档
- **测试说明？** → [TEST_README.md](TEST_README.md) - 测试套件文档
- **快速开始？** → [QUICKSTART.md](QUICKSTART.md) - 5分钟上手指南
- **算法原理？** → [ALGORITHM_DESIGN.md](ALGORITHM_DESIGN.md) - 算法设计文档

---

## 💡 一键命令速查

### Linux/Unix 用户
```bash
# 完整测试（推荐）
./run_tests.sh

# 快速测试
./run_tests.sh quick

# 仅编译
./compile.sh && ./test_aes_sm3

# 一行命令编译+测试
rm -f *.o test_aes_sm3 && gcc -O3 -pthread -c aes_sm3_integrity.c -o aes_sm3_integrity.o && gcc -O3 -pthread -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm && ./test_aes_sm3
```

### Windows 用户
```batch
REM 完整测试
run_tests.bat

REM 仅编译
compile.bat
```

---

## ✅ 验证编译成功

编译成功后，你应该看到：

```bash
$ ls -lh
-rwxr-xr-x 1 user user  512K test_aes_sm3           # 测试程序
-rw-r--r-- 1 user user  480K aes_sm3_integrity.o    # 目标文件

$ ./test_aes_sm3
╔══════════════════════════════════════════════════════════╗
║       AES-SM3完整性校验算法 - 综合测试套件               ║
╚══════════════════════════════════════════════════════════╝

测试平台: ARMv8.2-A
...
✓ 所有测试通过！
```

---

## 🎉 开始测试

选择你喜欢的方式，立即开始：

```bash
# 最简单的方式
./run_tests.sh
```

**祝你测试顺利！** 🚀

