# xPack Ver7 测试完善规范 (Test Roadmap Spec)

> 创建日期: 2026-02-01  
> 版本: 1.0

---

## 📋 目录

1. [任务概览](#任务概览)
2. [任务1: 为所有33个模块添加高质量测试](#任务1-为所有33个模块添加高质量测试)
3. [任务2: 添加性能基准测试](#任务2-添加性能基准测试)
4. [任务3: 添加覆盖率统计](#任务3-添加覆盖率统计)
5. [实施计划](#实施计划)
6. [验收标准](#验收标准)

---

## 任务概览

### 🎯 总体目标

构建一个完整的、可维护的、高覆盖率的测试体系，确保 xPack Ver7 的质量和稳定性。

### 📊 当前状态

| 模块范围 | 现有测试文件数 | 已实现测试 | 测试覆盖率 |
|---------|---------------|-----------|-----------|
| 33个模块 | 33个 | 111个 | ~20% |

### 🔍 需要改进的问题

1. **测试文件不完整**: 部分模块的测试文件是空的或只有基本框架
2. **测试质量不高**: 许多测试只是调用API，没有验证实际行为
3. **缺少性能测试**: 没有系统的性能基准测试
4. **缺少覆盖率统计**: 无法知道哪些代码路径没有被测试

---

## 任务1: 为所有33个模块添加高质量测试

### 📝 测试质量标准

每个模块需要包含以下类型的测试：

#### 1.1 基础功能测试 (Basic Function Tests)
- 测试每个API的基本功能
- 验证正常输入下的预期行为
- 覆盖所有公共API

#### 1.2 边界条件测试 (Edge Case Tests)
- 空输入/零长度
- 最大/最小值
- 边界值（如UINT32_MAX）
- 单个元素

#### 1.3 错误处理测试 (Error Handling Tests)
- 无效参数（NULL指针）
- 无效文件路径
- 无效索引
- 只读模式下的写操作

#### 1.4 数据完整性测试 (Data Integrity Tests)
- 压缩-解压缩循环验证
- Hash一致性检查
- 多次操作后数据一致性

#### 1.5 性能相关测试 (Performance-related Tests)
- 大文件处理
- 大量文件处理
- 内存使用情况

### 📦 模块测试清单

#### 核心模块 (01-03)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 01_core_basic | 基础功能, 边界条件 | P0 |
| 02_core_operations | 所有CRUD操作 | P0 |
| 03_core_edge_cases | 边界条件, 错误处理 | P0 |

#### 索引与路径 (04-06)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 04_index_operations | 索引访问, 更新, 删除 | P0 |
| 05_path_operations | 路径添加, 提取 | P0 |
| 06_path_case_sensitivity | 大小写敏感性 | P1 |

#### 压缩功能 (07-10)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 07_compression_data_patterns | 各种数据模式 | P0 |
| 08_compression_accuracy | 数据准确性 | P0 |
| 09_compression_large_files | 大文件压缩 | P0 |
| 10_solid_compression | 固实压缩 | P1 |

#### 高级功能 (11-17)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 11_error_handling | 错误处理机制 | P0 |
| 12_batch_operations | 批量操作 | P0 |
| 13_traverse_operations | 遍历功能 | P1 |
| 14_statistics | 统计功能 | P1 |
| 15_verify_operations | 验证功能 | P0 |
| 16_rebuild_operations | 重建功能 | P1 |
| 17_package_properties | 包属性 | P1 |

#### 类型与状态 (18-20)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 18_file_type | 文件类型 | P1 |
| 19_save_load_cycles | 保存加载循环 | P0 |
| 20_multiple_packages | 多包操作 | P1 |

#### 边界测试 (21-24)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 21_edge_large_files | 超大文件 | P1 |
| 22_edge_many_files | 超多文件 | P1 |
| 23_concurrent_access | 并发访问 | P2 |
| 24_corruption_recovery | 损坏恢复 | P1 |

#### 跨平台与内存 (25-26)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 25_cross_platform | 跨平台兼容性 | P2 |
| 26_memory_management | 内存管理 | P0 |

#### 集成与性能 (27-29)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 27_integration_real_world | 真实场景集成 | P1 |
| 28_performance_benchmark | 性能基准 | P2 |
| 29_compression_ratio | 压缩率 | P1 |

#### 回归与分卷 (30-33)
| 模块 | 测试类型 | 优先级 |
|-----|---------|--------|
| 30_regression_tests | 回归测试 | P1 |
| 31_volume_basic | 基础分卷 | P0 |
| 32_volume_single_file | 单文件分卷 | P0 |
| 33_volume_cross | 分卷交叉测试 | P1 |

### ✅ 每个模块的测试数量目标

| 优先级 | 目标测试数/模块 | 说明 |
|-------|---------------|------|
| P0 | 8-12个 | 核心功能必须完整覆盖 |
| P1 | 5-8个 | 重要功能需要充分测试 |
| P2 | 3-5个 | 辅助功能基本覆盖 |

**总目标**: 200-250个高质量测试

---

## 任务2: 添加性能基准测试

### 🎯 测试目标

1. 建立性能基准线
2. 监控性能退化
3. 识别性能瓶颈
4. 优化建议

### 📊 基准测试场景

#### 2.1 压缩性能
- 不同压缩级别的速度对比
- 不同压缩算法的速度对比
- 不同数据类型的压缩速度

#### 2.2 解压性能
- 单文件解压速度
- 多文件批量解压速度
- 随机访问解压速度

#### 2.3 操作性能
- 文件添加速度
- 文件删除速度
- 文件更新速度
- 包重建速度

#### 2.4 I/O 性能
- 大文件读写速度
- 小文件批量读写速度
- 分卷文件读写速度

#### 2.5 内存性能
- 峰值内存使用
- 平均内存使用
- 内存泄漏检测

### 📈 基准数据格式

```c
typedef struct {
    const char* test_name;
    uint64_t data_size;
    uint64_t time_ms;
    uint64_t speed_mb_per_sec;
    uint64_t peak_memory_kb;
    const char* notes;
} BenchmarkResult;
```

### 📁 输出格式

支持多种输出格式：
- 控制台表格输出
- JSON 格式（便于CI集成）
- CSV 格式（便于分析）
- Markdown 报告

### 🎯 性能基准线

需要为以下场景建立基准线：

| 场景 | 数据大小 | 目标速度 |
|-----|---------|---------|
| LZ4压缩 | 100MB | > 500 MB/s |
| LZMA2压缩 | 100MB | > 20 MB/s |
| ZSTD压缩 | 100MB | > 100 MB/s |
| 解压(任意) | 100MB | > 500 MB/s |
| 文件添加 | 1000个文件 | < 5秒 |
| 包重建 | 1000个文件 | < 10秒 |

---

## 任务3: 添加覆盖率统计

### 🎯 测试目标

1. 代码行覆盖率 > 80%
2. 分支覆盖率 > 70%
3. 函数覆盖率 > 90%

### 🔧 实现方案

#### 选项A: 使用 gcov/lcov (GCC/Clang)

```bash
# 编译时添加覆盖率标志
gcc --coverage -O0 -g ...

# 运行测试
./xpack_full_test.exe

# 生成覆盖率报告
gcov *.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

#### 选项B: 使用 OpenCppCoverage (Windows/MSVC)

```bash
OpenCppCoverage.exe --sources src --export_type cobertura:coverage.xml -- xpack_full_test.exe
```

#### 选项C: 自定义覆盖率统计

在测试框架中添加覆盖率统计：
- 记录每个API的调用次数
- 记录每个分支的执行情况
- 生成覆盖率报告

### 📊 覆盖率报告格式

```
Coverage Report
===============

File                            Lines    Functions    Branches
-----------------------------------------------------------
src/xpack.c                     85%      90%          75%
src/xpack_core.c                 82%      88%          70%
src/xpack_compress.c            78%      85%          68%
src/xpack_index.c               80%      92%          72%
src/xpack_volume.c              75%      80%          65%
src/xpack_ldb.c                 70%      78%          60%
src/xpack_util.c                90%      95%          85%
src/xpack_path.c                85%      90%          75%

Overall Coverage                81%      87%          72%

Legend: 
  [OK]  Coverage >= 80%
  [WARN] Coverage 60-79%
  [FAIL] Coverage < 60%
```

### 🎯 覆盖率目标分解

| 文件 | 行覆盖率目标 | 分支覆盖率目标 | 当前状态 |
|-----|------------|--------------|---------|
| src/xpack.c | 85% | 75% | 待测试 |
| src/xpack_core.c | 85% | 75% | 待测试 |
| src/xpack_compress.c | 80% | 70% | 待测试 |
| src/xpack_index.c | 85% | 75% | 待测试 |
| src/xpack_volume.c | 80% | 70% | 待测试 |
| src/xpack_ldb.c | 75% | 65% | 待测试 |
| src/xpack_util.c | 90% | 85% | 待测试 |
| src/xpack_path.c | 85% | 75% | 待测试 |

---

## 实施计划

### 📅 第一阶段: 基础测试完善 (P0模块)

**预计时间**: 2-3天  
**模块**: 01-03, 04, 07-09, 11, 12, 15, 19, 31, 32

**步骤**:
1. 审查现有测试，识别缺失的测试场景
2. 为每个P0模块补充完整的测试用例
3. 确保每个测试都能独立运行
4. 验证测试通过

### 📅 第二阶段: 扩展测试覆盖 (P1模块)

**预计时间**: 2-3天  
**模块**: 05-06, 10, 13-14, 16-18, 20-22, 24, 27, 29, 30, 33

**步骤**:
1. 为每个P1模块添加标准测试套件
2. 添加跨模块集成测试
3. 添加边界条件测试
4. 验证测试通过

### 📅 第三阶段: 性能基准测试

**预计时间**: 1-2天

**步骤**:
1. 设计基准测试框架
2. 实现各个基准测试场景
3. 建立初始基准线
4. 添加性能报告生成

### 📅 第四阶段: 覆盖率统计

**预计时间**: 1-2天

**步骤**:
1. 选择覆盖率工具（gcov或自定义）
2. 配置编译选项
3. 集成覆盖率统计到测试流程
4. 生成覆盖率报告
5. 分析未覆盖代码，补充测试

### 📅 第五阶段: 优化与完善

**预计时间**: 1天

**步骤**:
1. 分析覆盖率报告
2. 为未覆盖代码路径添加测试
3. 优化测试运行速度
4. 完善文档

---

## 验收标准

### ✅ 任务1验收标准

- [ ] 所有33个模块都有测试文件
- [ ] 至少200个测试用例
- [ ] P0模块每个至少8个测试
- [ ] P1模块每个至少5个测试
- [ ] P2模块每个至少3个测试
- [ ] 所有测试可以独立运行
- [ ] 测试通过率 > 90%

### ✅ 任务2验收标准

- [ ] 基准测试框架可用
- [ ] 至少10个基准测试场景
- [ ] 生成可读的性能报告
- [ ] 建立性能基准线
- [ ] 支持性能退化检测

### ✅ 任务3验收标准

- [ ] 覆盖率统计可运行
- [ ] 生成HTML格式的覆盖率报告
- [ ] 总体代码覆盖率 > 80%
- [ ] 分支覆盖率 > 70%
- [ ] 函数覆盖率 > 90%

### ✅ 总体验收标准

- [ ] 所有测试可以通过
- [ ] 测试套件可以在5分钟内完成
- [ ] 测试报告清晰易读
- [ ] 文档完整（测试编写指南、覆盖率报告说明）

---

## 附录

### A. 测试编写指南

#### A.1 测试命名规范

```
[module]_[scenario]_[expected_result]
```

示例:
- `core_append_single_file_success`
- `compress_large_file_no_error`
- `edge_case_empty_data_handled`

#### A.2 测试结构模板

```c
TEST(test_name) {
    // 1. 准备 (Arrange)
    xpkObject xpk = xpkOpen("test_file.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    // 2. 执行 (Act)
    uint32_t pos = xpkAppendData(xpk, data, size, level);
    ASSERT_NE(pos, UINT32_MAX);
    
    // 3. 断言 (Assert)
    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 1);
    
    // 4. 清理 (Cleanup)
    xpkClose(xpk);
}
```

#### A.3 测试最佳实践

1. **独立性**: 每个测试应该独立运行，不依赖其他测试
2. **可重复性**: 测试结果应该可重复
3. **快速性**: 测试应该快速执行
4. **清晰性**: 测试名称和断言应该清晰表达意图
5. **全面性**: 覆盖正常、边界、错误情况

### B. 性能测试模板

```c
TEST_PERF(benchmark_compression_lz4) {
    Benchmark bench;
    benchmark_init(&bench, "LZ4 Compression");
    
    for (int i = 0; i < 5; i++) {
        benchmark_start(&bench);
        
        xpkObject xpk = xpkOpen("bench_lz4.xpk", 0, 0);
        xpkAppendData(xpk, large_data, large_size, 0);
        xpkSave(xpk);
        xpkClose(xpk);
        
        benchmark_end(&bench);
    }
    
    benchmark_report(&bench);
}
```

### C. CI/CD集成

```yaml
# GitHub Actions 示例
name: xPack Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: [windows-latest, ubuntu-latest]
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: gcc -o xpack_test.exe test/*.c src/*.c lib/**/*.c
      - name: Run Tests
        run: ./xpack_test.exe
      - name: Coverage
        run: |
          gcc --coverage -o xpack_test.exe ...
          ./xpack_test.exe
          gcov *.c
          lcov --capture --output-file coverage.info
      - name: Upload Coverage
        uses: codecov/codecov-action@v2
```

---

## 更新历史

| 日期 | 版本 | 更新内容 |
|-----|------|---------|
| 2026-02-01 | 1.0 | 初始版本 |

---

## 参考资料

- xPack Ver7 API 文档
- 测试框架: test/test_framework.h
- 现有测试: test/*.h
