# 编译错误快速修复指南

## 问题描述

编译时出现错误：
```
multiple definition of `main'
```

**原因**：`aes_sm3_integrity.c` 和 `test_aes_sm3_integrity.c` 都包含 `main` 函数，导致链接冲突。

---

## ✅ 解决方案

### 方案 1：使用修复后的自动化脚本（推荐）

脚本已修复，现在可以直接使用：

#### Ubuntu/Linux 平台

```bash
cd test1.1

# 赋予执行权限
chmod +x run_tests.sh

# 运行完整测试套件
./run_tests.sh

# 或快速测试模式
./run_tests.sh quick
```

#### Windows 平台（需要 MinGW/MSYS2）

```batch
cd test1.1
run_tests.bat
```

---

### 方案 2：手动编译（Ubuntu/Linux）

#### 编译测试套件

```bash
cd test1.1

# ARMv8.2 平台（完整优化）
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread \
    -o test_aes_sm3 aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c -lm

# ARMv8 平台（备选）
gcc -march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c -lm

# x86/通用平台（仅功能测试）
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c -lm

# 运行测试
./test_aes_sm3
```

#### 编译演示程序（不运行测试套件）

```bash
cd test1.1

# 只编译演示程序（有 main 函数）
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -pthread \
    -o aes_sm3_demo aes_sm3_integrity.c -lm

# 运行演示
./aes_sm3_demo
```

---

### 方案 3：使用 Makefile（如果项目有提供）

```bash
cd test1.1

# 查看可用的编译目标
make help

# 编译并运行测试
make test
```

---

## 📋 文件说明

| 文件名 | 说明 | 包含 main 函数 |
|--------|------|---------------|
| `aes_sm3_integrity.c` | 完整实现 + 演示程序 | ✅ 是 |
| `aes_sm3_integrity_v2.3_opt.c` | 纯库文件（优化版本） | ❌ 否 |
| `test_aes_sm3_integrity.c` | 测试套件 | ✅ 是 |
| `test_correctness.c` | 正确性测试 | ✅ 是 |

**编译规则**：
- ✅ **测试套件**：使用 `aes_sm3_integrity_v2.3_opt.c` + `test_aes_sm3_integrity.c`
- ✅ **演示程序**：单独使用 `aes_sm3_integrity.c`
- ❌ **错误示例**：同时使用 `aes_sm3_integrity.c` + `test_aes_sm3_integrity.c`（会导致 main 冲突）

---

## 🔍 验证编译

### 检查编译器版本

```bash
gcc --version
# 推荐 GCC 9.0+ 用于 ARMv8 优化
```

### 检查 CPU 特性（Linux）

```bash
cat /proc/cpuinfo | grep Features
# 应包含：asimd (NEON), aes, sha2
```

### 检查架构

```bash
uname -m
# ARMv8: aarch64 或 arm64
# x86: x86_64
```

---

## ⚠️ 常见问题

### Q1: 编译时找不到 `aes_sm3_integrity_v2.3_opt.c`？

**A**: 确保文件存在于 `test1.1` 目录中：
```bash
ls -lh test1.1/aes_sm3_integrity_v2.3_opt.c
```

### Q2: 仍然出现 "multiple definition" 错误？

**A**: 检查编译命令，确保使用的是 `aes_sm3_integrity_v2.3_opt.c` 而不是 `aes_sm3_integrity.c`：
```bash
# 正确 ✅
gcc ... aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c ...

# 错误 ❌
gcc ... aes_sm3_integrity.c test_aes_sm3_integrity.c ...
```

### Q3: 编译选项太多，有更简单的吗？

**A**: 最简化编译（所有平台）：
```bash
gcc -O3 -pthread -o test_aes_sm3 aes_sm3_integrity_v2.3_opt.c test_aes_sm3_integrity.c -lm
./test_aes_sm3
```

---

## 📊 测试预期结果

成功编译运行后，应该看到类似输出：

```
═══════════════════════════════════════════════════════════
        AES-SM3完整性校验算法 - 测试套件
═══════════════════════════════════════════════════════════

第一部分：功能正确性测试
─────────────────────────────────────────────
▶ 测试: 基本功能测试 - 256位输出
✓ 通过

▶ 测试: 雪崩效应测试
  平均差异率: 49.87%
✓ 通过 (接近理想值50%)

...

═══════════════════════════════════════════════════════════
  测试完成: 所有测试通过 ✓
═══════════════════════════════════════════════════════════
```

---

## 📞 需要帮助？

如果问题仍未解决：

1. 检查 GCC 版本：`gcc --version`
2. 检查文件完整性：`ls -lh test1.1/*.c`
3. 查看详细错误信息：保留 `compile_error.log`
4. 参考项目文档：`TEST_README.md`、`QUICKSTART.md`

---

**修复完成时间**: 2025-10-22  
**影响文件**: `run_tests.sh`, `run_tests.bat`  
**状态**: ✅ 已修复

