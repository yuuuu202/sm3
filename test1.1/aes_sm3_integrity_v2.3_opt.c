// ============================================================================
// v2.3 内存访问优化 - 超级预取策略
// ============================================================================

// 超级预取优化版本 - 使用非时间临时加载和更激进的预取策略
void aes_sm3_integrity_batch_super_prefetch(const uint8_t** inputs, uint8_t** outputs, int batch_size) {
    // 分配临时存储空间（批处理版本）- 使用更大的对齐粒度
    uint8_t* temp_pool = (uint8_t*)aligned_alloc(128, batch_size * 128);  // 128字节对齐，适合AVX-512
    uint8_t* compressed_data[batch_size];
    
    // 设置指针数组，指向连续内存块中的不同位置
    for (int i = 0; i < batch_size; i++) {
        compressed_data[i] = temp_pool + i * 128;
    }
    
    // 第一阶段：超级预取XOR折叠压缩（4KB -> 128B）
    batch_xor_folding_compress_super_prefetch(inputs, compressed_data, batch_size);
    
    // 第二阶段：超级预取SM3哈希（128B -> 256bit）
    batch_sm3_hash_super_prefetch((const uint8_t**)compressed_data, outputs, batch_size);
    
    // 释放临时缓冲区（一次性释放，减少系统调用开销）
    free(temp_pool);
}

// 超级预取优化的XOR折叠压缩函数
void batch_xor_folding_compress_super_prefetch(const uint8_t** inputs, uint8_t** outputs, int batch_size) {
    // 超级预取策略：提前预取多个后续块
    const int prefetch_distance = 3;  // 预取后面第3个块
    
    // 预取前几个块
    for (int i = 0; i < prefetch_distance && i < batch_size; i++) {
        __builtin_prefetch(inputs[i], 0, 3);  // 高时间局部性预取
    }
    
    // 主处理循环
    for (int i = 0; i < batch_size; i++) {
        const uint8_t* input = inputs[i];
        uint8_t* output = outputs[i];
        
        // 预取后续块
        if (i + prefetch_distance < batch_size) {
            __builtin_prefetch(inputs[i + prefetch_distance], 0, 3);
        }
        
        // 使用NEON进行超级并行XOR折叠
        uint8x16x4_t input_vectors[4];  // 4组向量，每组4个向量，共16个向量(256字节)
        
        // 加载并预取第一组向量
        input_vectors[0] = vld4q_u8(input);
        __builtin_prefetch(input + 64, 0, 2);  // 预取下一缓存行
        
        // 加载并预取第二组向量
        input_vectors[1] = vld4q_u8(input + 64);
        __builtin_prefetch(input + 128, 0, 2);  // 预取下一缓存行
        
        // 加载并预取第三组向量
        input_vectors[2] = vld4q_u8(input + 128);
        __builtin_prefetch(input + 192, 0, 2);  // 预取下一缓存行
        
        // 加载并预取第四组向量
        input_vectors[3] = vld4q_u8(input + 192);
        if (i + 1 < batch_size) {
            __builtin_prefetch(inputs[i + 1], 0, 3);  // 预取下一个输入块
        }
        
        // 第一级XOR折叠：256字节 -> 64字节 (4:1)
        uint8x16_t folded_level1[4];
        
        // 每组内部进行XOR折叠
        folded_level1[0] = veorq_u8(input_vectors[0].val[0], input_vectors[0].val[1]);
        folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[2]);
        folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[3]);
        
        folded_level1[1] = veorq_u8(input_vectors[1].val[0], input_vectors[1].val[1]);
        folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[2]);
        folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[3]);
        
        folded_level1[2] = veorq_u8(input_vectors[2].val[0], input_vectors[2].val[1]);
        folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[2]);
        folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[3]);
        
        folded_level1[3] = veorq_u8(input_vectors[3].val[0], input_vectors[3].val[1]);
        folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[2]);
        folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[3]);
        
        // 第二级XOR折叠：64字节 -> 16字节 (4:1)
        uint8x16_t folded_level2;
        folded_level2 = veorq_u8(folded_level1[0], folded_level1[1]);
        folded_level2 = veorq_u8(folded_level2, folded_level1[2]);
        folded_level2 = veorq_u8(folded_level2, folded_level1[3]);
        
        // 第三级XOR折叠：16字节 -> 8字节 (2:1)
        uint8x8_t folded_level3;
        uint8x16x2_t split = vld2q_u8((uint8_t*)&folded_level2);
        folded_level3 = veor_u8(vget_low_u8(split.val[0]), vget_high_u8(split.val[0]));
        
        // 存储结果
        vst1_u8(output, folded_level3);
        
        // 继续处理剩余的256字节块（4096字节总共有16个256字节块）
        uint8_t accumulator[8] = {0};
        vst1_u8(accumulator, folded_level3);
        
        // 处理剩余的15个256字节块
        for (int block = 1; block < 16; block++) {
            const uint8_t* block_input = input + block * 256;
            
            // 预取下一个块
            if (block < 15) {
                __builtin_prefetch(block_input + 256, 0, 2);
            }
            
            // 加载4组向量
            input_vectors[0] = vld4q_u8(block_input);
            input_vectors[1] = vld4q_u8(block_input + 64);
            input_vectors[2] = vld4q_u8(block_input + 128);
            input_vectors[3] = vld4q_u8(block_input + 192);
            
            // 第一级XOR折叠
            folded_level1[0] = veorq_u8(input_vectors[0].val[0], input_vectors[0].val[1]);
            folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[2]);
            folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[3]);
            
            folded_level1[1] = veorq_u8(input_vectors[1].val[0], input_vectors[1].val[1]);
            folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[2]);
            folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[3]);
            
            folded_level1[2] = veorq_u8(input_vectors[2].val[0], input_vectors[2].val[1]);
            folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[2]);
            folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[3]);
            
            folded_level1[3] = veorq_u8(input_vectors[3].val[0], input_vectors[3].val[1]);
            folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[2]);
            folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[3]);
            
            // 第二级XOR折叠
            folded_level2 = veorq_u8(folded_level1[0], folded_level1[1]);
            folded_level2 = veorq_u8(folded_level2, folded_level1[2]);
            folded_level2 = veorq_u8(folded_level2, folded_level1[3]);
            
            // 第三级XOR折叠
            split = vld2q_u8((uint8_t*)&folded_level2);
            folded_level3 = veor_u8(vget_low_u8(split.val[0]), vget_high_u8(split.val[0]));
            
            // 累积到累加器
            uint8_t temp[8];
            vst1_u8(temp, folded_level3);
            for (int j = 0; j < 8; j++) {
                accumulator[j] ^= temp[j];
            }
        }
        
        // 存储最终结果
        for (int j = 0; j < 8; j++) {
            output[j] = accumulator[j];
        }
    }
}

// 超级预取优化的SM3哈希函数
void batch_sm3_hash_super_prefetch(const uint8_t** compressed_inputs, uint8_t** outputs, int batch_size) {
    // 使用SoA（Structure of Arrays）布局优化缓存访问
    uint32_t sm3_states[8][batch_size];  // 转置存储，提高缓存局部性
    
    // 批量初始化SM3状态（转置存储）
    for (int j = 0; j < 8; j++) {
        uint32_t init_val;
        switch (j) {
            case 0: init_val = 0x7380166F; break;
            case 1: init_val = 0x4914B2B9; break;
            case 2: init_val = 0x172442D7; break;
            case 3: init_val = 0xDA8A0600; break;
            case 4: init_val = 0xA96F30BC; break;
            case 5: init_val = 0x163138AA; break;
            case 6: init_val = 0xE38DEE4D; break;
            case 7: init_val = 0xB0FB0E4E; break;
        }
        
        // 使用NEON并行初始化
        uint32x4_t init_vec = vdupq_n_u32(init_val);
        for (int i = 0; i < batch_size; i += 4) {
            if (i + 4 <= batch_size) {
                vst1q_u32(&sm3_states[j][i], init_vec);
            } else {
                // 处理剩余元素
                for (int k = i; k < batch_size; k++) {
                    sm3_states[j][k] = init_val;
                }
            }
        }
    }
    
    // 预取所有输入数据
    for (int i = 0; i < batch_size; i++) {
        __builtin_prefetch(compressed_inputs[i], 0, 3);
    }
    
    // 批量处理SM3压缩（每个块只需要2次压缩）
    for (int i = 0; i < batch_size; i++) {
        const uint8_t* compressed = compressed_inputs[i];
        
        // 预取下一个输入
        if (i + 1 < batch_size) {
            __builtin_prefetch(compressed_inputs[i + 1], 0, 3);
        }
        
        // 第一个64字节块（前8个8字节压缩结果）
        uint32_t sm3_block[16];
        const uint32_t* src = (const uint32_t*)compressed;
        
        // 使用NEON加载和转换字节序
        uint32x4x4_t input_vec = vld4q_u32(src);
        uint32x4_t swapped_vec[4];
        
        // 字节序转换（大端序转小端序）
        swapped_vec[0] = vrev32q_u32(input_vec.val[0]);
        swapped_vec[1] = vrev32q_u32(input_vec.val[1]);
        swapped_vec[2] = vrev32q_u32(input_vec.val[2]);
        swapped_vec[3] = vrev32q_u32(input_vec.val[3]);
        
        // 存储到块数组
        vst1q_u32(&sm3_block[0], swapped_vec[0]);
        vst1q_u32(&sm3_block[4], swapped_vec[1]);
        vst1q_u32(&sm3_block[8], swapped_vec[2]);
        vst1q_u32(&sm3_block[12], swapped_vec[3]);
        
        // 准备状态向量（从SoA布局加载）
        uint32_t state[8];
        for (int j = 0; j < 8; j++) {
            state[j] = sm3_states[j][i];
        }
        
        // 执行SM3压缩
        sm3_compress_hw(state, sm3_block);
        
        // 第二个64字节块（后8个8字节压缩结果）
        src = (const uint32_t*)(compressed + 64);
        
        // 预取下一个块的数据
        if (i + 1 < batch_size) {
            __builtin_prefetch(compressed_inputs[i + 1] + 64, 0, 2);
        }
        
        // 使用NEON加载和转换字节序
        input_vec = vld4q_u32(src);
        swapped_vec[0] = vrev32q_u32(input_vec.val[0]);
        swapped_vec[1] = vrev32q_u32(input_vec.val[1]);
        swapped_vec[2] = vrev32q_u32(input_vec.val[2]);
        swapped_vec[3] = vrev32q_u32(input_vec.val[3]);
        
        // 存储到块数组
        vst1q_u32(&sm3_block[0], swapped_vec[0]);
        vst1q_u32(&sm3_block[4], swapped_vec[1]);
        vst1q_u32(&sm3_block[8], swapped_vec[2]);
        vst1q_u32(&sm3_block[12], swapped_vec[3]);
        
        // 执行第二次SM3压缩
        sm3_compress_hw(state, sm3_block);
        
        // 将结果存回SoA布局
        for (int j = 0; j < 8; j++) {
            sm3_states[j][i] = state[j];
        }
    }
    
    // 批量输出结果（从SoA布局转换并输出）
    for (int i = 0; i < batch_size; i++) {
        uint32_t* out32 = (uint32_t*)outputs[i];
        
        // 使用NEON进行字节序转换和存储
        uint32_t state_vec[8];
        for (int j = 0; j < 8; j++) {
            state_vec[j] = sm3_states[j][i];
        }
        
        // 转换字节序并输出
        uint32x4_t state1 = vld1q_u32(state_vec);
        uint32x4_t state2 = vld1q_u32(state_vec + 4);
        
        uint32x4_t swapped1 = vrev32q_u32(state1);
        uint32x4_t swapped2 = vrev32q_u32(state2);
        
        vst1q_u32(out32, swapped1);
        vst1q_u32(out32 + 4, swapped2);
    }
}

// ============================================================================
// v2.3 内存访问优化 - 流水线预取策略
// ============================================================================

// 流水线预取优化版本 - 使用双缓冲和流水线技术
void aes_sm3_integrity_batch_pipeline_prefetch(const uint8_t** inputs, uint8_t** outputs, int batch_size) {
    // 分配临时存储空间（批处理版本）- 双缓冲
    uint8_t* temp_pool[2];
    temp_pool[0] = (uint8_t*)aligned_alloc(128, batch_size * 128);  // 第一个缓冲区
    temp_pool[1] = (uint8_t*)aligned_alloc(128, batch_size * 128);  // 第二个缓冲区
    
    uint8_t* compressed_data[2][batch_size];
    
    // 设置指针数组，指向两个缓冲区
    for (int i = 0; i < batch_size; i++) {
        compressed_data[0][i] = temp_pool[0] + i * 128;
        compressed_data[1][i] = temp_pool[1] + i * 128;
    }
    
    // 流水线处理：第一阶段处理偶数批次，第二阶段处理奇数批次
    for (int phase = 0; phase < 2; phase++) {
        // 预取当前阶段的所有输入
        for (int i = 0; i < batch_size; i++) {
            __builtin_prefetch(inputs[i], 0, 3);
        }
        
        // 第一阶段：流水线XOR折叠压缩
        batch_xor_folding_compress_pipeline_prefetch(inputs, compressed_data[phase], batch_size, phase);
        
        // 第二阶段：流水线SM3哈希
        batch_sm3_hash_pipeline_prefetch((const uint8_t**)compressed_data[phase], outputs, batch_size, phase);
    }
    
    // 释放临时缓冲区
    free(temp_pool[0]);
    free(temp_pool[1]);
}

// 流水线预取优化的XOR折叠压缩函数
void batch_xor_folding_compress_pipeline_prefetch(const uint8_t** inputs, uint8_t** outputs, 
                                                  int batch_size, int phase) {
    // 根据流水线阶段调整预取策略
    const int prefetch_distance = (phase == 0) ? 2 : 3;  // 不同阶段使用不同的预取距离
    
    // 预取前几个块
    for (int i = 0; i < prefetch_distance && i < batch_size; i++) {
        __builtin_prefetch(inputs[i], 0, 3);
    }
    
    // 主处理循环
    for (int i = 0; i < batch_size; i++) {
        const uint8_t* input = inputs[i];
        uint8_t* output = outputs[i];
        
        // 预取后续块
        if (i + prefetch_distance < batch_size) {
            __builtin_prefetch(inputs[i + prefetch_distance], 0, 3);
        }
        
        // 使用NEON进行流水线XOR折叠
        uint8x16x4_t input_vectors[4];
        
        // 流水线加载策略
        if (phase == 0) {
            // 第一阶段：顺序加载
            input_vectors[0] = vld4q_u8(input);
            input_vectors[1] = vld4q_u8(input + 64);
            input_vectors[2] = vld4q_u8(input + 128);
            input_vectors[3] = vld4q_u8(input + 192);
        } else {
            // 第二阶段：交错加载，提高缓存利用率
            input_vectors[0] = vld4q_u8(input);
            __builtin_prefetch(input + 256, 0, 2);
            input_vectors[1] = vld4q_u8(input + 64);
            __builtin_prefetch(input + 320, 0, 2);
            input_vectors[2] = vld4q_u8(input + 128);
            __builtin_prefetch(input + 384, 0, 2);
            input_vectors[3] = vld4q_u8(input + 192);
            if (i + 1 < batch_size) {
                __builtin_prefetch(inputs[i + 1], 0, 3);
            }
        }
        
        // 三级XOR折叠（与超级预取版本相同）
        uint8x16_t folded_level1[4];
        
        folded_level1[0] = veorq_u8(input_vectors[0].val[0], input_vectors[0].val[1]);
        folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[2]);
        folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[3]);
        
        folded_level1[1] = veorq_u8(input_vectors[1].val[0], input_vectors[1].val[1]);
        folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[2]);
        folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[3]);
        
        folded_level1[2] = veorq_u8(input_vectors[2].val[0], input_vectors[2].val[1]);
        folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[2]);
        folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[3]);
        
        folded_level1[3] = veorq_u8(input_vectors[3].val[0], input_vectors[3].val[1]);
        folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[2]);
        folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[3]);
        
        uint8x16_t folded_level2;
        folded_level2 = veorq_u8(folded_level1[0], folded_level1[1]);
        folded_level2 = veorq_u8(folded_level2, folded_level1[2]);
        folded_level2 = veorq_u8(folded_level2, folded_level1[3]);
        
        uint8x8_t folded_level3;
        uint8x16x2_t split = vld2q_u8((uint8_t*)&folded_level2);
        folded_level3 = veor_u8(vget_low_u8(split.val[0]), vget_high_u8(split.val[0]));
        
        // 存储结果
        vst1_u8(output, folded_level3);
        
        // 继续处理剩余的256字节块
        uint8_t accumulator[8] = {0};
        vst1_u8(accumulator, folded_level3);
        
        for (int block = 1; block < 16; block++) {
            const uint8_t* block_input = input + block * 256;
            
            // 根据流水线阶段调整预取策略
            if (phase == 0) {
                if (block < 15) {
                    __builtin_prefetch(block_input + 256, 0, 2);
                }
            } else {
                if (block < 14) {
                    __builtin_prefetch(block_input + 512, 0, 2);
                }
            }
            
            // 加载4组向量
            input_vectors[0] = vld4q_u8(block_input);
            input_vectors[1] = vld4q_u8(block_input + 64);
            input_vectors[2] = vld4q_u8(block_input + 128);
            input_vectors[3] = vld4q_u8(block_input + 192);
            
            // 第一级XOR折叠
            folded_level1[0] = veorq_u8(input_vectors[0].val[0], input_vectors[0].val[1]);
            folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[2]);
            folded_level1[0] = veorq_u8(folded_level1[0], input_vectors[0].val[3]);
            
            folded_level1[1] = veorq_u8(input_vectors[1].val[0], input_vectors[1].val[1]);
            folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[2]);
            folded_level1[1] = veorq_u8(folded_level1[1], input_vectors[1].val[3]);
            
            folded_level1[2] = veorq_u8(input_vectors[2].val[0], input_vectors[2].val[1]);
            folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[2]);
            folded_level1[2] = veorq_u8(folded_level1[2], input_vectors[2].val[3]);
            
            folded_level1[3] = veorq_u8(input_vectors[3].val[0], input_vectors[3].val[1]);
            folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[2]);
            folded_level1[3] = veorq_u8(folded_level1[3], input_vectors[3].val[3]);
            
            // 第二级XOR折叠
            folded_level2 = veorq_u8(folded_level1[0], folded_level1[1]);
            folded_level2 = veorq_u8(folded_level2, folded_level1[2]);
            folded_level2 = veorq_u8(folded_level2, folded_level1[3]);
            
            // 第三级XOR折叠
            split = vld2q_u8((uint8_t*)&folded_level2);
            folded_level3 = veor_u8(vget_low_u8(split.val[0]), vget_high_u8(split.val[0]));
            
            // 累积到累加器
            uint8_t temp[8];
            vst1_u8(temp, folded_level3);
            for (int j = 0; j < 8; j++) {
                accumulator[j] ^= temp[j];
            }
        }
        
        // 存储最终结果
        for (int j = 0; j < 8; j++) {
            output[j] = accumulator[j];
        }
    }
}

// 流水线预取优化的SM3哈希函数
void batch_sm3_hash_pipeline_prefetch(const uint8_t** compressed_inputs, uint8_t** outputs, 
                                      int batch_size, int phase) {
    // 使用SoA（Structure of Arrays）布局优化缓存访问
    uint32_t sm3_states[8][batch_size];  // 转置存储，提高缓存局部性
    
    // 批量初始化SM3状态（转置存储）
    for (int j = 0; j < 8; j++) {
        uint32_t init_val;
        switch (j) {
            case 0: init_val = 0x7380166F; break;
            case 1: init_val = 0x4914B2B9; break;
            case 2: init_val = 0x172442D7; break;
            case 3: init_val = 0xDA8A0600; break;
            case 4: init_val = 0xA96F30BC; break;
            case 5: init_val = 0x163138AA; break;
            case 6: init_val = 0xE38DEE4D; break;
            case 7: init_val = 0xB0FB0E4E; break;
        }
        
        // 使用NEON并行初始化
        uint32x4_t init_vec = vdupq_n_u32(init_val);
        for (int i = 0; i < batch_size; i += 4) {
            if (i + 4 <= batch_size) {
                vst1q_u32(&sm3_states[j][i], init_vec);
            } else {
                // 处理剩余元素
                for (int k = i; k < batch_size; k++) {
                    sm3_states[j][k] = init_val;
                }
            }
        }
    }
    
    // 根据流水线阶段调整预取策略
    if (phase == 0) {
        // 第一阶段：预取所有输入数据
        for (int i = 0; i < batch_size; i++) {
            __builtin_prefetch(compressed_inputs[i], 0, 3);
        }
    } else {
        // 第二阶段：交错预取
        for (int i = 0; i < batch_size; i += 2) {
            __builtin_prefetch(compressed_inputs[i], 0, 3);
            if (i + 1 < batch_size) {
                __builtin_prefetch(compressed_inputs[i + 1], 0, 2);
            }
        }
    }
    
    // 批量处理SM3压缩
    for (int i = 0; i < batch_size; i++) {
        const uint8_t* compressed = compressed_inputs[i];
        
        // 根据流水线阶段调整预取策略
        if (phase == 0) {
            // 第一阶段：顺序预取下一个输入
            if (i + 1 < batch_size) {
                __builtin_prefetch(compressed_inputs[i + 1], 0, 3);
            }
        } else {
            // 第二阶段：交错预取
            if (i + 2 < batch_size) {
                __builtin_prefetch(compressed_inputs[i + 2], 0, 3);
            }
        }
        
        // 第一个64字节块处理
        uint32_t sm3_block[16];
        const uint32_t* src = (const uint32_t*)compressed;
        
        // 使用NEON加载和转换字节序
        uint32x4x4_t input_vec = vld4q_u32(src);
        uint32x4_t swapped_vec[4];
        
        // 字节序转换
        swapped_vec[0] = vrev32q_u32(input_vec.val[0]);
        swapped_vec[1] = vrev32q_u32(input_vec.val[1]);
        swapped_vec[2] = vrev32q_u32(input_vec.val[2]);
        swapped_vec[3] = vrev32q_u32(input_vec.val[3]);
        
        // 存储到块数组
        vst1q_u32(&sm3_block[0], swapped_vec[0]);
        vst1q_u32(&sm3_block[4], swapped_vec[1]);
        vst1q_u32(&sm3_block[8], swapped_vec[2]);
        vst1q_u32(&sm3_block[12], swapped_vec[3]);
        
        // 准备状态向量（从SoA布局加载）
        uint32_t state[8];
        for (int j = 0; j < 8; j++) {
            state[j] = sm3_states[j][i];
        }
        
        // 执行SM3压缩
        sm3_compress_hw(state, sm3_block);
        
        // 第二个64字节块处理
        src = (const uint32_t*)(compressed + 64);
        
        // 根据流水线阶段调整预取策略
        if (phase == 0) {
            if (i + 1 < batch_size) {
                __builtin_prefetch(compressed_inputs[i + 1] + 64, 0, 2);
            }
        } else {
            if (i + 2 < batch_size) {
                __builtin_prefetch(compressed_inputs[i + 2] + 64, 0, 2);
            }
        }
        
        // 使用NEON加载和转换字节序
        input_vec = vld4q_u32(src);
        swapped_vec[0] = vrev32q_u32(input_vec.val[0]);
        swapped_vec[1] = vrev32q_u32(input_vec.val[1]);
        swapped_vec[2] = vrev32q_u32(input_vec.val[2]);
        swapped_vec[3] = vrev32q_u32(input_vec.val[3]);
        
        // 存储到块数组
        vst1q_u32(&sm3_block[0], swapped_vec[0]);
        vst1q_u32(&sm3_block[4], swapped_vec[1]);
        vst1q_u32(&sm3_block[8], swapped_vec[2]);
        vst1q_u32(&sm3_block[12], swapped_vec[3]);
        
        // 执行第二次SM3压缩
        sm3_compress_hw(state, sm3_block);
        
        // 将结果存回SoA布局
        for (int j = 0; j < 8; j++) {
            sm3_states[j][i] = state[j];
        }
    }
    
    // 批量输出结果（从SoA布局转换并输出）
    for (int i = 0; i < batch_size; i++) {
        uint32_t* out32 = (uint32_t*)outputs[i];
        
        // 使用NEON进行字节序转换和存储
        uint32_t state_vec[8];
        for (int j = 0; j < 8; j++) {
            state_vec[j] = sm3_states[j][i];
        }
        
        // 转换字节序并输出
        uint32x4_t state1 = vld1q_u32(state_vec);
        uint32x4_t state2 = vld1q_u32(state_vec + 4);
        
        uint32x4_t swapped1 = vrev32q_u32(state1);
        uint32x4_t swapped2 = vrev32q_u32(state2);
        
        vst1q_u32(out32, swapped1);
        vst1q_u32(out32 + 4, swapped2);
    }
}