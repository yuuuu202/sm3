# 🔧 编译错误快速修复指南

## ❌ 错误：Multiple definition of `main`

**完整错误信息（两种形式）：**

**形式1：常规错误**
```
/usr/bin/ld: /tmp/ccXwBiBA.o: in function `main':
test_aes_sm3_integrity.c:(.text.startup+0x0): multiple definition of `main'; 
/tmp/ccD3A2Ts.o:aes_sm3_integrity.c:(.text.startup+0x0): first defined here
collect2: error: ld returned 1 exit status
```

**形式2：LTO（链接时优化）错误**
```
/usr/bin/ld: /tmp/ccjVMrOW.o (symbol from plugin): in function `print_hash':
(.text+0x0): multiple definition of `main'; aes_sm3_integrity.o (symbol from plugin):(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```
> 如果看到 `(symbol from plugin)` 字样，说明是 `-flto` 选项导致的。

---

## ✅ 解决方案（3种方法，任选其一）

### 方法 1：使用修复后的自动化脚本（最简单，推荐）

```bash
# Linux/Unix
chmod +x run_tests.sh
./run_tests.sh
```

```batch
REM Windows
run_tests.bat
```

---

### 方法 2：使用快速编译脚本

```bash
# Linux/Unix
chmod +x compile.sh
./compile.sh
./test_aes_sm3
```

```batch
REM Windows
compile.bat
test_aes_sm3.exe
```

---

### 方法 3：手动分步编译（完全控制）

#### Linux/Unix

```bash
# 步骤1: 编译主算法文件为 .o 目标文件（不链接）
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# 步骤2: 编译测试文件并链接目标文件
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 步骤3: 运行测试
./test_aes_sm3
```

#### Windows

```batch
REM 步骤1: 编译主算法文件
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

REM 步骤2: 编译测试文件并链接
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -o test_aes_sm3.exe aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

REM 步骤3: 运行测试
test_aes_sm3.exe
```

#### ARMv8 平台（完整优化）

```bash
# 步骤1: 编译主算法文件（不使用 -flto，避免 LTO 导致符号冲突）
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -fomit-frame-pointer -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# 步骤2: 编译测试文件并链接
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -fomit-frame-pointer -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 步骤3: 运行测试
./test_aes_sm3
```

> **注意：** 如果你在错误信息中看到 `(symbol from plugin)` 字样，说明是 `-flto`（链接时优化）导致的。移除 `-flto` 选项即可解决。

---

## 📝 关键要点

### ❌ 错误的做法
```bash
# 这会失败！两个文件都有 main 函数
gcc -o test aes_sm3_integrity.c test_aes_sm3_integrity.c
```

### ✅ 正确的做法
```bash
# 第一个文件编译为 .o（使用 -c 选项，不链接）
gcc -c aes_sm3_integrity.c -o aes_sm3_integrity.o

# 第二个文件编译并链接第一个文件的 .o
gcc -o test aes_sm3_integrity.o test_aes_sm3_integrity.c
```

---

## 🎯 为什么会出现这个错误？

### 根本原因
这个项目包含两个独立的程序：

1. **`aes_sm3_integrity.c`** 
   - 主算法实现 + 性能测试的 `main` 函数
   - 可以独立运行进行基准测试

2. **`test_aes_sm3_integrity.c`**
   - 综合测试套件 + 自己的 `main` 函数
   - 包含15个完整的测试用例

当你同时编译这两个文件时，链接器发现了两个 `main` 函数，不知道应该使用哪个，因此报错。

### LTO（链接时优化）问题
如果你看到错误信息中有 `(symbol from plugin)` 字样，这是因为 `-flto` 选项在处理多个 `main` 函数时会产生额外的符号冲突。

**解决方案：**
1. 使用分步编译（`-c` 选项）
2. 移除 `-flto` 选项（或在链接时不使用）

---

## 🔍 验证编译成功

编译成功后，你应该看到：

```bash
# Linux/Unix
$ ls -lh
-rwxr-xr-x 1 user user  512K test_aes_sm3      # 可执行文件
-rw-r--r-- 1 user user  480K aes_sm3_integrity.o  # 目标文件

# 运行测试
$ ./test_aes_sm3

╔══════════════════════════════════════════════════════════╗
║       AES-SM3完整性校验算法 - 综合测试套件               ║
╚══════════════════════════════════════════════════════════╝

测试平台: ARMv8.2-A
...
```

---

## 🆘 还是不行？

### 检查清单

1. **确认使用了 `-c` 选项编译第一个文件**
   ```bash
   gcc -c aes_sm3_integrity.c -o aes_sm3_integrity.o
   # 注意这里的 -c 选项！
   ```

2. **确认 .o 文件已生成**
   ```bash
   ls -lh aes_sm3_integrity.o
   # 应该显示文件存在且大小合理（几百KB）
   ```

3. **确认第二步链接时使用的是 .o 文件**
   ```bash
   gcc -o test aes_sm3_integrity.o test_aes_sm3_integrity.c
   # 第一个参数是 .o 文件，不是 .c 文件！
   ```

4. **清理旧文件重新开始**
   ```bash
   rm -f *.o test_aes_sm3 test_aes_sm3.exe
   # 然后重新编译
   ```

---

## 📚 相关文档

- 详细编译指南：[COMPILE_GUIDE.md](COMPILE_GUIDE.md)
- 测试文档：[TEST_README.md](TEST_README.md)
- 快速开始：[QUICKSTART.md](QUICKSTART.md)

---

## 💡 一键解决命令

如果你只想快速运行测试，复制粘贴下面的命令：

### Linux/Unix 一键命令

```bash
rm -f *.o test_aes_sm3 && \
gcc -O3 -pthread -c aes_sm3_integrity.c -o aes_sm3_integrity.o && \
gcc -O3 -pthread -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm && \
./test_aes_sm3
```

### Windows 一键命令

```batch
del /Q *.o test_aes_sm3.exe 2>nul & gcc -O3 -pthread -c aes_sm3_integrity.c -o aes_sm3_integrity.o & gcc -O3 -pthread -o test_aes_sm3.exe aes_sm3_integrity.o test_aes_sm3_integrity.c -lm & test_aes_sm3.exe
```

---

**✅ 问题解决了吗？如果还有问题，请检查 GCC 版本或查看详细的编译错误日志。**

