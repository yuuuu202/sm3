# AES-SM3完整性校验算法 - 测试文档

## 概述

本测试套件基于整合文档要求，全面测试AES-SM3完整性校验算法的功能正确性、安全性和性能。

## 测试覆盖范围

### 1. 功能正确性测试 (6项)
- ✓ 基本功能测试 - 256位输出
- ✓ 基本功能测试 - 128位输出
- ✓ 确定性测试 - 相同输入产生相同输出
- ✓ 不同版本算法输出一致性（v2.2, v3.0, v3.1, v4.0, v5.0, v6.0）
- ✓ 边界条件 - 全0输入
- ✓ 边界条件 - 全1输入

### 2. 安全性测试 (3项)
- ✓ 雪崩效应测试 - 单比特变化影响
- ✓ 多点雪崩效应测试
- ✓ 输出分布均匀性测试

### 3. 性能基准测试 (4项)
- ✓ 单块处理性能基准（目标：35,000-55,000 MB/s）
- ✓ 不同版本性能对比
- ✓ vs SHA256/SM3基准性能对比
  - 目标：相对SHA256硬件加速 15-20倍
  - 目标：相对纯SM3 50-60倍
- ✓ 批处理性能测试

### 4. 内存访问优化测试
- ✓ 预取优化效果（目标：10-20%提升）
- ✓ 内存对齐优化效果（目标：5-10%提升）
- ✓ 总体优化效果（目标：15-30%提升）

### 5. 压力和稳定性测试 (2项)
- ✓ 长时间稳定性测试（30秒连续运行）
- ✓ 随机输入压力测试（10000组随机输入）

## 编译和运行

### Linux/Unix平台（推荐）

#### 方法1：使用自动化脚本（推荐）

```bash
# 赋予执行权限
chmod +x run_tests.sh

# 运行完整测试
./run_tests.sh

# 快速测试（跳过部分耗时测试）
./run_tests.sh quick
```

#### 方法2：手动编译

```bash
# 完整优化编译（ARMv8.2平台）- 分步编译避免main函数冲突
# 步骤1: 编译主算法文件为目标文件
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm

# 步骤2: 编译测试文件并链接
gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# 运行测试
./test_aes_sm3
```

#### 备选编译选项（不支持某些特性时）

```bash
# 基础ARMv8平台
gcc -march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm
gcc -march=armv8-a+crypto -O3 -funroll-loops -ftree-vectorize \
    -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm

# x86平台（仅功能测试，性能不佳）
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -c aes_sm3_integrity.c -o aes_sm3_integrity.o -lm
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread \
    -o test_aes_sm3 aes_sm3_integrity.o test_aes_sm3_integrity.c -lm
```

### Windows平台

#### 使用批处理脚本

```batch
REM 运行完整测试
run_tests.bat

REM 快速测试
run_tests.bat quick
```

#### 手动编译（MinGW/MSYS2）

```batch
gcc -O3 -funroll-loops -ftree-vectorize -finline-functions -pthread ^
    -o test_aes_sm3.exe aes_sm3_integrity.c test_aes_sm3_integrity.c -lm

test_aes_sm3.exe
```

## 测试结果解读

### 性能测试目标值（基于整合文档）

| 测试项目 | 目标值 | 备注 |
|---------|--------|------|
| 单块处理吞吐量 | 超过SHA256硬件10倍 | v5.0/v6.0版本 |
| vs SHA256硬件加速 | ≥10倍 | 核心性能指标 |
| vs 纯SM3 | 50-60倍 | 算法优化效果 |
| 预取优化提升 | 10-20% | 内存访问优化 |
| 内存对齐优化提升 | 5-10% | 内存访问优化 |

### 安全性测试目标值

| 测试项目 | 目标值 | 备注 |
|---------|--------|------|
| 雪崩效应 | 45-55% | 单比特变化导致输出比特翻转比例（接近理想50%） |
| 输出分布均匀性 | >75% | 至少75%的比特位分布均衡（35-65%范围）|

### 测试输出示例

```
═══════════════════════════════════════════════════════════
第三部分：性能基准测试
═══════════════════════════════════════════════════════════

▶ 测试: 单块处理性能基准测试（目标：35,000-55,000 MB/s）
  迭代次数: 100000
  总耗时: 8.890000秒
  吞吐量: 45000.50 MB/s
  单块延迟: 88.90微秒
  ✓ 达到性能目标（>= 35,000 MB/s）
✓ 通过 (耗时: 8.890000秒)

▶ 测试: vs SHA256/SM3基准性能对比
  ▶ SHA256硬件加速性能:
    吞吐量: 2500.30 MB/s
  
  ▶ 纯SM3算法性能:
    吞吐量: 800.15 MB/s
  
  ▶ XOR-SM3混合算法（v5.0 Super）:
    吞吐量: 45000.50 MB/s
  
  性能加速比汇总:
  ─────────────────────────────────────────────
  vs SHA256硬件加速: 18.00x ✓ 达标（目标15-20x）
  vs 纯SM3算法:     56.25x ✓ 达标（目标50-60x）
✓ 通过 (耗时: 5.630000秒)
```

## 测试环境要求

### 推荐配置
- **处理器**: ARMv8.2-A或更高（鲲鹏920、Cortex-A76/A78等）
- **指令集**: NEON SIMD、Crypto扩展（AES/SHA2/SM3）
- **操作系统**: Linux内核4.15+（Ubuntu 20.04 LTS或CentOS 8）
- **编译器**: GCC 9.3.0+或Clang 10.0+
- **内存**: 至少2GB可用内存
- **CPU核心**: 建议4核以上（多线程测试）

### 最低配置
- **处理器**: ARMv8-A
- **指令集**: NEON SIMD
- **操作系统**: Linux/Unix
- **编译器**: GCC 7.0+
- **内存**: 至少512MB

### 验证CPU特性

```bash
# Linux平台检查CPU特性
cat /proc/cpuinfo | grep Features

# 应包含：
# - asimd (NEON支持)
# - aes (AES硬件加速)
# - sha2 (SHA2硬件加速)
```

## 故障排除

### 编译错误

**错误1**: `unknown option '-march=armv8.2-a+crypto'`
- **原因**: GCC版本太旧或不支持该架构
- **解决**: 使用备选编译选项 `-march=armv8-a+crypto`

**错误2**: `undefined reference to 'vld1q_u8'`
- **原因**: 未启用NEON支持
- **解决**: 确保编译选项包含 `-march=armv8-a` 或更高

**错误3**: `implicit declaration of function '__builtin_prefetch'`
- **原因**: 编译器不支持该内建函数
- **解决**: GCC通常支持，检查GCC版本 >= 4.0

### 运行时错误

**错误1**: 段错误（Segmentation fault）
- **原因**: 可能是内存对齐问题
- **解决**: 检查平台是否支持非对齐访问

**错误2**: 性能远低于预期
- **原因**: 未启用优化或不在ARM平台
- **解决**: 
  1. 确保使用 `-O3` 优化
  2. 检查CPU是否为ARMv8架构
  3. 检查CPU是否支持NEON/Crypto扩展

**错误3**: 测试超时
- **原因**: 性能过低或系统负载高
- **解决**: 
  1. 关闭其他程序
  2. 使用快速测试模式 `./run_tests.sh quick`

## 性能优化建议

### 1. CPU频率设置
```bash
# 设置为性能模式（Linux）
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

### 2. 关闭CPU节能功能
```bash
# 禁用CPU C-states（可选）
sudo cpupower idle-set -D 0
```

### 3. 绑定CPU核心
```bash
# 使用taskset绑定到特定核心
taskset -c 0 ./test_aes_sm3
```

### 4. 提高进程优先级
```bash
# 使用nice降低优先级值（提高优先级）
sudo nice -n -20 ./test_aes_sm3
```

## 测试报告

测试完成后，建议记录以下信息：

1. **测试环境**
   - CPU型号和频率
   - 内存大小
   - 操作系统版本
   - GCC版本

2. **性能数据**
   - 单块处理吞吐量
   - vs SHA256加速比
   - vs SM3加速比
   - 各版本性能对比

3. **功能测试结果**
   - 通过测试数量
   - 失败测试数量（如有）
   - 雪崩效应比例
   - 输出分布均匀性

4. **优化效果**
   - 预取优化提升百分比
   - 内存对齐优化提升百分比

## 常见问题FAQ

**Q: 为什么在x86平台性能很低？**
A: 本算法专门针对ARMv8平台和NEON指令集优化，x86平台无法使用这些优化。

**Q: v5.0和v6.0哪个版本更好？**
A: v5.0 Super是生产推荐版本，性能和代码大小平衡；v6.0 Hyper是极限性能版本，适合追求绝对性能的场景。

**Q: 测试需要多长时间？**
A: 完整测试约2-3分钟，快速测试约1分钟。

**Q: 如何只运行性能测试？**
A: 可以直接运行主程序的 `performance_benchmark()` 函数，或修改测试文件注释掉其他测试。

**Q: 测试结果可以用于对比吗？**
A: 可以，但需要在相同硬件、相同编译选项下对比才有意义。

## 联系和反馈

如有问题或建议，请参考项目文档或提交Issue。

## 许可证

本测试套件与主项目使用相同许可证。

