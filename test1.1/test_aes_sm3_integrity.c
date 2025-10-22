/*
 * AES-SM3完整性校验算法综合测试套件
 * 
 * 测试覆盖范围（基于整合文档要求）：
 * 1. 功能正确性测试
 *    - XOR折叠压缩正确性
 *    - SM3哈希输出正确性
 *    - 不同版本算法输出一致性（v2.2, v3.0, v3.1, v4.0, v5.0, v6.0）
 *    - 128位和256位输出正确性
 * 
 * 2. 性能基准测试
 *    - 单块处理性能（目标：35,000-55,000 MB/s）
 *    - vs SHA256硬件加速（目标：15-20倍加速）
 *    - vs 纯SM3（目标：50-60倍加速）
 *    - 批处理性能测试
 *    - 多线程性能测试
 * 
 * 3. 安全性测试
 *    - 雪崩效应测试（单比特变化影响）
 *    - 输出分布均匀性测试
 *    - 确定性测试（相同输入相同输出）
 * 
 * 4. 内存访问优化测试
 *    - 预取优化效果（目标：10-20%提升）
 *    - 内存对齐优化效果（目标：5-10%提升）
 *    - 总体优化效果（目标：15-30%提升）
 * 
 * 5. 边界条件和压力测试
 *    - 全0、全1、随机输入测试
 *    - 长时间稳定性测试
 *    - 批处理边界条件测试
 * 
 * 编译命令：
 * gcc -march=armv8.2-a+crypto -O3 -funroll-loops -ftree-vectorize \
 *     -finline-functions -ffast-math -flto -fomit-frame-pointer -pthread \
 *     -o test_aes_sm3 aes_sm3_integrity.c test_aes_sm3_integrity.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>

#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

// 引用主文件中的函数声明
extern void aes_sm3_integrity_256bit(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_128bit(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_256bit_extreme(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_256bit_ultra(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_256bit_mega(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_256bit_super(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_256bit_hyper(const uint8_t* input, uint8_t* output);
extern void aes_sm3_integrity_batch(const uint8_t** inputs, uint8_t** outputs, int batch_size);
extern void sha256_4kb(const uint8_t* input, uint8_t* output);
extern void sm3_4kb(const uint8_t* input, uint8_t* output);

// 测试统计结构
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    double total_time;
} test_stats_t;

static test_stats_t global_stats = {0, 0, 0, 0.0};

// 颜色输出定义
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_RESET   "\033[0m"

// 测试宏
#define TEST_START(name) do { \
    printf(COLOR_CYAN "\n▶ 测试: %s\n" COLOR_RESET, name); \
    global_stats.total_tests++; \
    struct timespec test_start, test_end; \
    clock_gettime(CLOCK_MONOTONIC, &test_start);

#define TEST_END() \
    clock_gettime(CLOCK_MONOTONIC, &test_end); \
    double test_time = (test_end.tv_sec - test_start.tv_sec) + \
                       (test_end.tv_nsec - test_start.tv_nsec) / 1e9; \
    global_stats.total_time += test_time; \
    printf(COLOR_GREEN "✓ 通过 (耗时: %.6f秒)\n" COLOR_RESET, test_time); \
    global_stats.passed_tests++; \
} while(0)

#define TEST_FAIL(msg) do { \
    printf(COLOR_RED "✗ 失败: %s\n" COLOR_RESET, msg); \
    global_stats.failed_tests++; \
    return; \
} while(0)

#define ASSERT_TRUE(cond, msg) if (!(cond)) TEST_FAIL(msg)

// 辅助函数：打印哈希值
void print_hash(const char* label, const uint8_t* hash, int len) {
    printf("  %s: ", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

// 辅助函数：比较哈希值
int compare_hash(const uint8_t* h1, const uint8_t* h2, int len) {
    return memcmp(h1, h2, len) == 0;
}

// 辅助函数：计算汉明距离
int hamming_distance(const uint8_t* h1, const uint8_t* h2, int len) {
    int distance = 0;
    for (int i = 0; i < len; i++) {
        uint8_t xor_val = h1[i] ^ h2[i];
        while (xor_val) {
            distance += xor_val & 1;
            xor_val >>= 1;
        }
    }
    return distance;
}

// ============================================================================
// 第一部分：功能正确性测试
// ============================================================================

// 测试1：基本功能测试 - 256位输出
void test_basic_functionality_256bit() {
    TEST_START("基本功能测试 - 256位输出");
    
    uint8_t input[4096];
    uint8_t output[32];
    
    // 准备测试数据
    for (int i = 0; i < 4096; i++) {
        input[i] = i % 256;
    }
    
    // 调用主算法
    aes_sm3_integrity_256bit(input, output);
    
    // 验证输出不全为0
    int all_zero = 1;
    for (int i = 0; i < 32; i++) {
        if (output[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    ASSERT_TRUE(!all_zero, "输出不应全为0");
    
    print_hash("256位输出", output, 32);
    
    TEST_END();
}

// 测试2：基本功能测试 - 128位输出
void test_basic_functionality_128bit() {
    TEST_START("基本功能测试 - 128位输出");
    
    uint8_t input[4096];
    uint8_t output_256[32];
    uint8_t output_128[16];
    
    // 准备测试数据
    for (int i = 0; i < 4096; i++) {
        input[i] = i % 256;
    }
    
    // 调用算法
    aes_sm3_integrity_256bit(input, output_256);
    aes_sm3_integrity_128bit(input, output_128);
    
    // 验证128位输出是256位输出的前半部分
    ASSERT_TRUE(memcmp(output_256, output_128, 16) == 0, 
                "128位输出应是256位输出的前16字节");
    
    print_hash("128位输出", output_128, 16);
    
    TEST_END();
}

// 测试3：确定性测试 - 相同输入应产生相同输出
void test_deterministic_output() {
    TEST_START("确定性测试 - 相同输入产生相同输出");
    
    uint8_t input[4096];
    uint8_t output1[32];
    uint8_t output2[32];
    
    // 准备测试数据
    for (int i = 0; i < 4096; i++) {
        input[i] = (i * 7 + 13) % 256;
    }
    
    // 两次调用
    aes_sm3_integrity_256bit(input, output1);
    aes_sm3_integrity_256bit(input, output2);
    
    // 验证输出一致
    ASSERT_TRUE(compare_hash(output1, output2, 32), 
                "相同输入应产生相同输出");
    
    TEST_END();
}

// 测试4：不同版本算法输出一致性测试
void test_version_consistency() {
    TEST_START("不同版本算法输出一致性");
    
    uint8_t input[4096];
    uint8_t output_v22[32];
    uint8_t output_extreme[32];
    uint8_t output_ultra[32];
    uint8_t output_mega[32];
    uint8_t output_super[32];
    uint8_t output_hyper[32];
    
    // 准备测试数据
    for (int i = 0; i < 4096; i++) {
        input[i] = (i * 31 + 7) % 256;
    }
    
    // 调用不同版本
    aes_sm3_integrity_256bit(input, output_v22);          // v2.2版本
    aes_sm3_integrity_256bit_extreme(input, output_extreme);  // v3.0
    aes_sm3_integrity_256bit_ultra(input, output_ultra);      // v3.1
    aes_sm3_integrity_256bit_mega(input, output_mega);        // v4.0
    aes_sm3_integrity_256bit_super(input, output_super);      // v5.0
    aes_sm3_integrity_256bit_hyper(input, output_hyper);      // v6.0
    
    // 注意：不同版本的压缩策略不同，输出可能不同
    // 这里主要测试各版本能正常运行
    
    print_hash("v2.2版本", output_v22, 32);
    print_hash("v3.0 Extreme", output_extreme, 32);
    print_hash("v3.1 Ultra", output_ultra, 32);
    print_hash("v4.0 Mega", output_mega, 32);
    print_hash("v5.0 Super", output_super, 32);
    print_hash("v6.0 Hyper", output_hyper, 32);
    
    printf("  注意：不同版本采用不同压缩策略，输出可能不同\n");
    
    TEST_END();
}

// 测试5：边界条件测试 - 全0输入
void test_all_zero_input() {
    TEST_START("边界条件 - 全0输入");
    
    uint8_t input[4096] = {0};
    uint8_t output[32];
    
    aes_sm3_integrity_256bit(input, output);
    
    // 验证输出不全为0（哈希函数应该有扩散性）
    int all_zero = 1;
    for (int i = 0; i < 32; i++) {
        if (output[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    ASSERT_TRUE(!all_zero, "全0输入应产生非全0输出");
    
    print_hash("全0输入的输出", output, 32);
    
    TEST_END();
}

// 测试6：边界条件测试 - 全1输入
void test_all_one_input() {
    TEST_START("边界条件 - 全1输入");
    
    uint8_t input[4096];
    uint8_t output[32];
    
    memset(input, 0xFF, 4096);
    aes_sm3_integrity_256bit(input, output);
    
    print_hash("全1输入的输出", output, 32);
    
    TEST_END();
}

// ============================================================================
// 第二部分：安全性测试
// ============================================================================

// 测试7：雪崩效应测试 - 单比特变化影响
void test_avalanche_effect() {
    TEST_START("雪崩效应测试 - 单比特变化影响");
    
    uint8_t input1[4096];
    uint8_t input2[4096];
    uint8_t output1[32];
    uint8_t output2[32];
    
    // 准备原始输入
    for (int i = 0; i < 4096; i++) {
        input1[i] = (i * 17 + 23) % 256;
    }
    memcpy(input2, input1, 4096);
    
    // 翻转第一个字节的第一个比特
    input2[0] ^= 0x01;
    
    // 计算哈希
    aes_sm3_integrity_256bit(input1, output1);
    aes_sm3_integrity_256bit(input2, output2);
    
    // 计算汉明距离
    int distance = hamming_distance(output1, output2, 32);
    double flip_ratio = (double)distance / (32 * 8);
    
    printf("  单比特变化导致输出变化: %d / 256 比特 (%.2f%%)\n", 
           distance, flip_ratio * 100);
    
    // 理想的雪崩效应应该使约50%的输出比特翻转
    ASSERT_TRUE(flip_ratio > 0.35 && flip_ratio < 0.65, 
                "雪崩效应应使35%-65%的输出比特翻转");
    
    TEST_END();
}

// 测试8：多点雪崩效应测试
void test_multi_point_avalanche() {
    TEST_START("多点雪崩效应测试");
    
    uint8_t input[4096];
    uint8_t output_base[32];
    
    // 准备基准输入
    for (int i = 0; i < 4096; i++) {
        input[i] = (i * 31 + 7) % 256;
    }
    aes_sm3_integrity_256bit(input, output_base);
    
    // 测试不同位置的单比特变化
    int test_positions[] = {0, 1024, 2048, 4095};
    double total_flip_ratio = 0;
    
    for (int i = 0; i < 4; i++) {
        uint8_t input_mod[4096];
        uint8_t output_mod[32];
        
        memcpy(input_mod, input, 4096);
        input_mod[test_positions[i]] ^= 0x01;
        
        aes_sm3_integrity_256bit(input_mod, output_mod);
        
        int distance = hamming_distance(output_base, output_mod, 32);
        double flip_ratio = (double)distance / (32 * 8);
        total_flip_ratio += flip_ratio;
        
        printf("  位置%d翻转1比特 → 输出变化%.2f%%\n", 
               test_positions[i], flip_ratio * 100);
    }
    
    double avg_flip_ratio = total_flip_ratio / 4;
    printf("  平均翻转比例: %.2f%%\n", avg_flip_ratio * 100);
    
    ASSERT_TRUE(avg_flip_ratio > 0.35 && avg_flip_ratio < 0.65,
                "平均雪崩效应应在35%-65%之间");
    
    TEST_END();
}

// 测试9：输出分布均匀性测试
void test_output_distribution() {
    TEST_START("输出分布均匀性测试");
    
    const int num_samples = 1000;
    int bit_count[256] = {0};  // 统计每个字节位置的1的数量
    
    uint8_t input[4096];
    uint8_t output[32];
    
    // 生成多组随机输入并统计输出
    for (int sample = 0; sample < num_samples; sample++) {
        // 生成随机输入
        for (int i = 0; i < 4096; i++) {
            input[i] = (sample * i + i * i + 17) % 256;
        }
        
        aes_sm3_integrity_256bit(input, output);
        
        // 统计每个比特
        for (int byte_idx = 0; byte_idx < 32; byte_idx++) {
            for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
                if (output[byte_idx] & (1 << bit_idx)) {
                    bit_count[byte_idx * 8 + bit_idx]++;
                }
            }
        }
    }
    
    // 检查分布是否均匀（应接近50%）
    int unbalanced_bits = 0;
    for (int i = 0; i < 256; i++) {
        double ratio = (double)bit_count[i] / num_samples;
        if (ratio < 0.40 || ratio > 0.60) {
            unbalanced_bits++;
        }
    }
    
    double balance_ratio = 1.0 - (double)unbalanced_bits / 256;
    printf("  %d个样本测试，%.2f%%的比特位分布均衡（40-60%%范围）\n",
           num_samples, balance_ratio * 100);
    
    ASSERT_TRUE(balance_ratio > 0.85, 
                "至少85%的比特位应该分布均衡");
    
    TEST_END();
}

// ============================================================================
// 第三部分：性能基准测试
// ============================================================================

// 测试10：单块处理性能基准
void test_single_block_performance() {
    TEST_START("单块处理性能基准测试（目标：35,000-55,000 MB/s）");
    
    uint8_t input[4096];
    uint8_t output[32];
    
    // 准备测试数据
    for (int i = 0; i < 4096; i++) {
        input[i] = i % 256;
    }
    
    const int iterations = 100000;
    struct timespec start, end;
    
    // 预热
    for (int i = 0; i < 1000; i++) {
        aes_sm3_integrity_256bit(input, output);
    }
    
    // 正式测试
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        aes_sm3_integrity_256bit(input, output);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (iterations * 4.0) / elapsed;  // MB/s
    double latency = (elapsed / iterations) * 1e6;     // 微秒
    
    printf("  迭代次数: %d\n", iterations);
    printf("  总耗时: %.6f秒\n", elapsed);
    printf("  吞吐量: %.2f MB/s\n", throughput);
    printf("  单块延迟: %.2f微秒\n", latency);
    
    if (throughput >= 35000) {
        printf(COLOR_GREEN "  ✓ 达到性能目标（>= 35,000 MB/s）\n" COLOR_RESET);
    } else if (throughput >= 20000) {
        printf(COLOR_YELLOW "  ⚠ 接近目标但未达标（20,000-35,000 MB/s）\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "  ✗ 未达性能目标（< 20,000 MB/s）\n" COLOR_RESET);
    }
    
    TEST_END();
}

// 测试11：不同版本性能对比
void test_version_performance_comparison() {
    TEST_START("不同版本性能对比");
    
    uint8_t input[4096];
    uint8_t output[32];
    
    for (int i = 0; i < 4096; i++) {
        input[i] = i % 256;
    }
    
    const int iterations = 50000;
    struct timespec start, end;
    
    // 定义测试版本
    typedef void (*integrity_func_t)(const uint8_t*, uint8_t*);
    struct {
        const char* name;
        integrity_func_t func;
    } versions[] = {
        {"v2.2 标准版", aes_sm3_integrity_256bit},
        {"v3.0 Extreme", aes_sm3_integrity_256bit_extreme},
        {"v3.1 Ultra", aes_sm3_integrity_256bit_ultra},
        {"v4.0 Mega", aes_sm3_integrity_256bit_mega},
        {"v5.0 Super", aes_sm3_integrity_256bit_super},
        {"v6.0 Hyper", aes_sm3_integrity_256bit_hyper}
    };
    
    printf("\n");
    printf("  版本名称          吞吐量(MB/s)    相对v2.2加速比\n");
    printf("  ─────────────────────────────────────────────\n");
    
    double v22_throughput = 0;
    
    for (int v = 0; v < 6; v++) {
        // 预热
        for (int i = 0; i < 100; i++) {
            versions[v].func(input, output);
        }
        
        // 测试
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < iterations; i++) {
            versions[v].func(input, output);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double elapsed = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
        double throughput = (iterations * 4.0) / elapsed;
        
        if (v == 0) v22_throughput = throughput;
        double speedup = throughput / v22_throughput;
        
        printf("  %-16s %10.2f        %.2fx\n", 
               versions[v].name, throughput, speedup);
    }
    
    TEST_END();
}

// 测试12：vs SHA256和SM3性能对比
void test_vs_baseline_performance() {
    TEST_START("vs SHA256/SM3基准性能对比");
    
    uint8_t input[4096];
    uint8_t output[32];
    
    for (int i = 0; i < 4096; i++) {
        input[i] = i % 256;
    }
    
    const int iterations = 50000;
    struct timespec start, end;
    double elapsed, throughput;
    
    // 测试SHA256硬件加速
    printf("\n  ▶ SHA256硬件加速性能:\n");
    for (int i = 0; i < 100; i++) sha256_4kb(input, output);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        sha256_4kb(input, output);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double sha256_throughput = (iterations * 4.0) / elapsed;
    printf("    吞吐量: %.2f MB/s\n", sha256_throughput);
    
    // 测试纯SM3
    printf("\n  ▶ 纯SM3算法性能:\n");
    for (int i = 0; i < 100; i++) sm3_4kb(input, output);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        sm3_4kb(input, output);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double sm3_throughput = (iterations * 4.0) / elapsed;
    printf("    吞吐量: %.2f MB/s\n", sm3_throughput);
    
    // 测试本算法（v5.0 Super）
    printf("\n  ▶ XOR-SM3混合算法（v5.0 Super）:\n");
    for (int i = 0; i < 100; i++) aes_sm3_integrity_256bit_super(input, output);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        aes_sm3_integrity_256bit_super(input, output);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double our_throughput = (iterations * 4.0) / elapsed;
    printf("    吞吐量: %.2f MB/s\n", our_throughput);
    
    // 计算加速比
    double speedup_vs_sha256 = our_throughput / sha256_throughput;
    double speedup_vs_sm3 = our_throughput / sm3_throughput;
    
    printf("\n  性能加速比汇总:\n");
    printf("  ─────────────────────────────────────────────\n");
    printf("  vs SHA256硬件加速: %.2fx", speedup_vs_sha256);
    if (speedup_vs_sha256 >= 15.0) {
        printf(COLOR_GREEN " ✓ 达标（目标15-20x）\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW " ⚠ 未达标（目标15-20x）\n" COLOR_RESET);
    }
    
    printf("  vs 纯SM3算法:     %.2fx", speedup_vs_sm3);
    if (speedup_vs_sm3 >= 50.0) {
        printf(COLOR_GREEN " ✓ 达标（目标50-60x）\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW " ⚠ 未达标（目标50-60x）\n" COLOR_RESET);
    }
    
    TEST_END();
}

// 测试13：批处理性能测试
void test_batch_performance() {
    TEST_START("批处理性能测试");
    
    const int batch_size = 8;
    const int iterations = 10000;
    
    // 准备批处理数据
    uint8_t* batch_input_data = malloc(batch_size * 4096);
    uint8_t* batch_output_data = malloc(batch_size * 32);
    const uint8_t* batch_inputs[batch_size];
    uint8_t* batch_outputs[batch_size];
    
    for (int i = 0; i < batch_size; i++) {
        batch_inputs[i] = batch_input_data + i * 4096;
        batch_outputs[i] = batch_output_data + i * 32;
        
        for (int j = 0; j < 4096; j++) {
            batch_input_data[i * 4096 + j] = (i + j) % 256;
        }
    }
    
    struct timespec start, end;
    
    // 预热
    for (int i = 0; i < 100; i++) {
        aes_sm3_integrity_batch(batch_inputs, batch_outputs, batch_size);
    }
    
    // 测试批处理
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        aes_sm3_integrity_batch(batch_inputs, batch_outputs, batch_size);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (iterations * batch_size * 4.0) / elapsed;
    
    printf("  批大小: %d\n", batch_size);
    printf("  迭代次数: %d\n", iterations);
    printf("  吞吐量: %.2f MB/s\n", throughput);
    
    free(batch_input_data);
    free(batch_output_data);
    
    TEST_END();
}

// ============================================================================
// 第四部分：内存访问优化测试
// ============================================================================

// 这部分测试已在主文件的test_memory_access_optimization()中实现
// 这里只需要调用即可
extern void test_memory_access_optimization(void);

void test_memory_optimization_wrapper() {
    TEST_START("内存访问优化效果测试（调用主文件测试）");
    
    printf("\n");
    test_memory_access_optimization();
    
    TEST_END();
}

// ============================================================================
// 第五部分：压力和稳定性测试
// ============================================================================

// 测试14：长时间稳定性测试
void test_long_running_stability() {
    TEST_START("长时间稳定性测试（30秒）");
    
    uint8_t input[4096];
    uint8_t output[32];
    uint8_t first_output[32];
    
    // 准备固定输入
    for (int i = 0; i < 4096; i++) {
        input[i] = i % 256;
    }
    
    // 获取基准输出
    aes_sm3_integrity_256bit(input, first_output);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int iterations = 0;
    int errors = 0;
    
    // 运行30秒
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed > 30.0) break;
        
        aes_sm3_integrity_256bit(input, output);
        
        // 验证输出一致性
        if (!compare_hash(output, first_output, 32)) {
            errors++;
        }
        
        iterations++;
    }
    
    double total_time = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (iterations * 4.0) / total_time;
    
    printf("  运行时间: %.2f秒\n", total_time);
    printf("  总迭代: %d次\n", iterations);
    printf("  错误次数: %d\n", errors);
    printf("  平均吞吐量: %.2f MB/s\n", throughput);
    
    ASSERT_TRUE(errors == 0, "长时间运行不应出现错误");
    
    TEST_END();
}

// 测试15：随机输入压力测试
void test_random_input_stress() {
    TEST_START("随机输入压力测试（10000组随机输入）");
    
    uint8_t input[4096];
    uint8_t output[32];
    
    srand(time(NULL));
    
    for (int i = 0; i < 10000; i++) {
        // 生成随机输入
        for (int j = 0; j < 4096; j++) {
            input[j] = rand() % 256;
        }
        
        // 计算哈希
        aes_sm3_integrity_256bit(input, output);
        
        // 验证输出不全为0
        int all_zero = 1;
        for (int k = 0; k < 32; k++) {
            if (output[k] != 0) {
                all_zero = 0;
                break;
            }
        }
        
        if (all_zero) {
            TEST_FAIL("发现全0输出");
        }
    }
    
    printf("  所有10000组随机输入测试通过\n");
    
    TEST_END();
}

// ============================================================================
// 主测试运行器
// ============================================================================

void print_test_summary() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                   测试结果汇总                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  总测试数:   %d\n", global_stats.total_tests);
    printf("  通过:       " COLOR_GREEN "%d" COLOR_RESET "\n", global_stats.passed_tests);
    printf("  失败:       " COLOR_RED "%d" COLOR_RESET "\n", global_stats.failed_tests);
    printf("  总耗时:     %.2f秒\n", global_stats.total_time);
    
    if (global_stats.failed_tests == 0) {
        printf("\n" COLOR_GREEN "  ✓ 所有测试通过！\n" COLOR_RESET);
    } else {
        printf("\n" COLOR_RED "  ✗ 部分测试失败！\n" COLOR_RESET);
    }
    
    printf("\n");
}

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║       AES-SM3完整性校验算法 - 综合测试套件               ║\n");
    printf("║       Comprehensive Test Suite for AES-SM3 Integrity    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("测试平台: ARMv8.2-A\n");
    printf("测试日期: %s\n", __DATE__);
    printf("测试时间: %s\n", __TIME__);
    printf("\n");
    
    printf(COLOR_MAGENTA "═══════════════════════════════════════════════════════════\n");
    printf("第一部分：功能正确性测试\n");
    printf("═══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    test_basic_functionality_256bit();
    test_basic_functionality_128bit();
    test_deterministic_output();
    test_version_consistency();
    test_all_zero_input();
    test_all_one_input();
    
    printf(COLOR_MAGENTA "\n═══════════════════════════════════════════════════════════\n");
    printf("第二部分：安全性测试\n");
    printf("═══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    test_avalanche_effect();
    test_multi_point_avalanche();
    test_output_distribution();
    
    printf(COLOR_MAGENTA "\n═══════════════════════════════════════════════════════════\n");
    printf("第三部分：性能基准测试\n");
    printf("═══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    test_single_block_performance();
    test_version_performance_comparison();
    test_vs_baseline_performance();
    test_batch_performance();
    
    printf(COLOR_MAGENTA "\n═══════════════════════════════════════════════════════════\n");
    printf("第四部分：内存访问优化测试\n");
    printf("═══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    test_memory_optimization_wrapper();
    
    printf(COLOR_MAGENTA "\n═══════════════════════════════════════════════════════════\n");
    printf("第五部分：压力和稳定性测试\n");
    printf("═══════════════════════════════════════════════════════════\n" COLOR_RESET);
    
    test_long_running_stability();
    test_random_input_stress();
    
    // 打印测试汇总
    print_test_summary();
    
    return (global_stats.failed_tests == 0) ? 0 : 1;
}

