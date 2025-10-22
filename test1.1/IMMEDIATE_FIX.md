# 🚨 立即修复 LTO 插件错误

## 错误信息
```
/usr/bin/ld: /tmp/ccHO62k4.o (symbol from plugin): in function `print_hash':
(.text+0x0): multiple definition of `main'; aes_sm3_integrity.o (symbol from plugin):(.text+0x0): first defined here
```

## ⚡ 立即执行（复制粘贴这些命令）

### 在 Linux 服务器上运行：

```bash
# 进入项目目录
cd test1.1

# 彻底清理所有编译产物（这是关键！）
rm -f *.o test_aes_sm3 a.out compile_error.log

# 重新编译（不使用 LTO）
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行测试
./test_aes_sm3
```

---

## 📋 如果是 ARM 平台，使用这个优化版本：

```bash
# 清理
rm -f *.o test_aes_sm3 a.out compile_error.log

# ARM 优化编译
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -fomit-frame-pointer -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o

gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -fomit-frame-pointer -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行
./test_aes_sm3
```

---

## 🔧 或者使用一键脚本：

```bash
# 赋予执行权限
chmod +x clean_and_compile.sh run_tests.sh

# 运行清理编译脚本
./clean_and_compile.sh

# 或运行完整测试
./run_tests.sh
```

---

## 🔍 问题原因

1. **旧的 `.o` 文件包含 LTO 信息**
   - 之前用 `-flto` 编译的 `.o` 文件会保留 LTO 元数据
   - 即使新的编译命令不使用 `-flto`，旧的 `.o` 文件仍会导致问题

2. **解决方案：清理后重新编译**
   - 必须删除所有 `.o` 文件
   - 用不带 `-flto` 的命令重新编译

---

## ✅ 验证步骤

### 1. 确认清理
```bash
ls *.o 2>/dev/null
# 应该显示：ls: cannot access '*.o': No such file or directory
# 或者什么都不显示
```

### 2. 确认不使用 -flto
```bash
# 编译时不应该看到 -flto
gcc -O3 -c aes_sm3_integrity.c -o aes_sm3_integrity.o
# 注意：没有 -flto
```

### 3. 确认编译成功
```bash
ls -lh test_aes_sm3
# 应该显示一个可执行文件

./test_aes_sm3
# 应该开始运行测试
```

---

## 🆘 如果还是不行

### 检查 GCC 配置
```bash
# 检查 GCC 版本
gcc --version

# 检查是否有默认的 LTO 配置
gcc -v 2>&1 | grep -i lto

# 检查环境变量
echo $CFLAGS
echo $LDFLAGS
# 如果这些包含 -flto，需要清除
```

### 临时禁用环境变量中的 LTO
```bash
# 如果环境变量中有 -flto
unset CFLAGS
unset LDFLAGS
unset CXXFLAGS

# 然后重新编译
rm -f *.o test_aes_sm3
gcc -O3 -c aes_sm3_integrity.c -o aes_sm3_integrity.o
gcc -O3 -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm
```

---

## 📞 最终方案：使用最简单的编译选项

如果上面都不行，使用最基础的编译选项：

```bash
# 清理
rm -f *.o test_aes_sm3 a.out

# 最简单的编译（不使用任何高级优化）
gcc -O2 -c aes_sm3_integrity.c -o aes_sm3_integrity.o
gcc -O2 -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行
./test_aes_sm3
```

---

## 💡 关键要点

1. ✅ **必须清理旧的 .o 文件**
2. ✅ **不要使用 -flto 选项**
3. ✅ **使用分步编译（-c 然后链接）**
4. ✅ **检查环境变量中没有 -flto**

---

## 🎯 推荐的完整流程

```bash
#!/bin/bash
# 复制这整段脚本运行

cd test1.1

# 1. 彻底清理
echo "清理旧文件..."
rm -f *.o test_aes_sm3 a.out compile_error.log

# 2. 编译算法文件
echo "编译算法文件..."
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o || exit 1

# 3. 编译测试并链接
echo "编译测试文件..."
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm || exit 1

# 4. 运行
echo "运行测试..."
./test_aes_sm3
```

---

**立即执行上面的命令，问题应该就能解决！** 🚀

