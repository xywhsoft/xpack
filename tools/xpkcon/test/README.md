# xpkcon 测试套件

xpkcon 测试套件用于验证命令行工具的各项功能是否正常工作。

## 测试框架

测试框架提供了以下功能：

- 测试宏定义（`TEST`, `RUN_TEST`, `ASSERT` 等）
- 文件操作工具（创建、读取、比较文件）
- 目录操作工具（创建、删除目录）
- 命令执行工具
- 跨平台支持（Windows/Linux）

## 测试用例

### 基本操作测试
| 测试 | 描述 |
|------|------|
| `basic_add_single_file` | 添加单个文件到压缩包 |
| `basic_add_multiple_files` | 添加多个文件到压缩包 |

### 提取测试
| 测试 | 描述 |
|------|------|
| `extract_full_path` | 完整路径解压（x 命令） |
| `extract_no_path` | 无路径解压（e 命令） |

### 列表和信息测试
| 测试 | 描述 |
|------|------|
| `list_files` | 列出压缩包内容（l 命令） |
| `info_command` | 显示压缩包详细信息（i 命令） |

### 测试完整性
| 测试 | 描述 |
|------|------|
| `test_integrity` | 测试压缩包完整性（t 命令） |

### 删除和更新测试
| 测试 | 描述 |
|------|------|
| `delete_file` | 从压缩包删除文件（d 命令） |
| `update_file` | 更新压缩包中的文件（u 命令） |

### 压缩测试
| 测试 | 描述 |
|------|------|
| `compression_levels` | 测试所有 16 级压缩级别 |

### 固实模式测试
| 测试 | 描述 |
|------|------|
| `solid_mode` | 测试固实压缩模式 |

### 包类型测试
| 测试 | 描述 |
|------|------|
| `package_types` | 测试四种包类型（core/index/linux/win32） |

### 边界情况测试
| 测试 | 描述 |
|------|------|
| `empty_files` | 处理空文件 |
| `binary_files` | 处理二进制文件 |
| `special_characters_in_filenames` | 处理特殊字符文件名 |
| `overwrite_existing_files` | 覆盖已存在文件 |
| `yes_flag` | 测试 -y 标志自动确认 |

### 高级测试
| 测试 | 描述 |
|------|------|
| `recursive_directory` | 递归处理目录（-r 标志） |
| `large_files` | 处理大文件（1MB） |

## 编译和运行

### Windows

#### 编译测试程序 (x64)
```batch
cd tools\xpkcon\test
build_test.bat
```

输出到：`release\x64\test_xpkcon.exe`, `release\x64\test_all.exe`

#### 编译测试程序 (x86)
```batch
cd tools\xpkcon\test
build_test_x86.bat
```

输出到：`release\x86\test_xpkcon.exe`, `release\x86\test_all.exe`

#### 运行测试
```batch
cd tools\xpkcon\test
run_test.bat
```

#### 运行完整测试套件（带报告）
```batch
cd tools\xpkcon\test
run_all.bat
```

生成 HTML 报告：
```batch
run_all.bat --html
```

生成 JSON 报告：
```batch
run_all.bat --json
```

同时生成两种报告：
```batch
run_all.bat --html --json
```

### Linux

#### 编译测试程序
```bash
cd tools/xpkcon/test
chmod +x build_test.sh
./build_test.sh
```

输出到：`release/linux/test_xpkcon`, `release/linux/test_all`

#### 运行测试
```bash
cd tools/xpkcon/test
chmod +x run_test.sh
./run_test.sh
```

#### 运行完整测试套件（带报告）
```bash
cd tools/xpkcon/test
chmod +x run_all.sh
./run_all.sh            # 完整测试套件
./run_all.sh --html     # 生成 HTML 报告
./run_all.sh --json     # 生成 JSON 报告
```

## 前置条件

运行测试前需要先编译 xpkcon：

### Windows
```batch
cd tools\xpkcon
build_tcc_static_x64.bat
```

### Linux
```bash
cd tools/xpkcon
./build_linux_static.sh
```

测试程序会自动在 `../../release/x64/xpkcon.exe`（Windows）或 `../../release/linux/xpkcon`（Linux）查找 xpkcon 可执行文件。

## 输出目录

所有编译输出统一放在 `release` 目录下：

```
release/
├── x64/           # Windows 64-bit
│   ├── xpkcon.exe
│   ├── test_xpkcon.exe
│   └── test_all.exe
├── x86/           # Windows 32-bit
│   ├── xpkcon.exe
│   ├── test_xpkcon.exe
│   └── test_all.exe
└── linux/         # Linux
    ├── xpkcon
    ├── test_xpkcon
    └── test_all
```

## 测试输出

成功运行的测试输出示例：

```
=================================================
  xpkcon Test Suite - Command Line Tool Tests
=================================================

[Basic Operations]

  Testing basic_add_single_file... PASSED
  Testing basic_add_multiple_files... PASSED

[Extract Tests]
  Testing extract_full_path... PASSED
  Testing extract_no_path... PASSED

[List and Info Tests]
  Testing list_files... PASSED
  Testing info_command... PASSED

[Test Integrity]
  Testing test_integrity... PASSED

[Delete and Update Tests]
  Testing delete_file... PASSED
  Testing update_file... PASSED

[Compression Tests]
  Testing compression_levels... PASSED

[Solid Mode Tests]
  Testing solid_mode... PASSED

[Package Type Tests]
  Testing package_types... PASSED

[Edge Cases]
  Testing empty_files... PASSED
  Testing binary_files... PASSED
  Testing special_characters_in_filenames... PASSED
  Testing overwrite_existing_files... PASSED
  Testing yes_flag... PASSED

[Advanced Tests]
  Testing recursive_directory... PASSED
  Testing large_files... PASSED

=================================================
  Summary
=================================================
  Total Tests:  20
  Passed:       20
  Failed:       0
  Success Rate: 100.0%
=================================================

  Detailed Test Report
=================================================
  basic_add_single_file           | Basic Operations   | PASSED
  basic_add_multiple_files        | Basic Operations   | PASSED
  extract_full_path              | Extract Tests     | PASSED
  extract_no_path               | Extract Tests     | PASSED
  list_files                   | List and Info     | PASSED
  info_command                 | List and Info     | PASSED
  test_integrity                | Basic Operations  | PASSED
  delete_file                  | Delete Tests     | PASSED
  update_file                  | Update Tests     | PASSED
  compression_levels            | Compression Tests | PASSED
  solid_mode                   | Solid Mode       | PASSED
  package_types                | Package Types    | PASSED
  empty_files                  | Edge Cases       | PASSED
  binary_files                 | Edge Cases       | PASSED
  special_characters_in_filenames | Edge Cases    | PASSED
  overwrite_existing_files      | Edge Cases       | PASSED
  yes_flag                    | Edge Cases       | PASSED
  recursive_directory          | Advanced Tests    | PASSED
  large_files                  | Advanced Tests    | PASSED

  HTML report saved to: test_report.html
  JSON report saved to: test_report.json
```

## 添加新测试

1. 在 `test_xpkcon.c` 中添加测试函数：
```c
TEST(my_new_test) {
    // 测试代码
    ASSERT(test_create_test_file("test.txt", "content") == 0);
    // 更多断言...
}
```

2. 在 `test_all.c` 的 `g_test_cases` 数组中注册测试：
```c
{"my_new_test", test_my_new_test, TEST_CATEGORY_BASIC},
```

3. 重新编译并运行测试。

## 测试文件

测试过程中会创建以下临时文件（在当前目录）：

- `test_*.xpk` - 测试压缩包
- `test_*.txt` - 测试文本文件
- `test_*.dat` - 测试二进制文件
- `test_output/`, `test_*_out/` - 测试输出目录

测试完成后会自动清理这些文件。

## 报告文件

运行 `test_all` 时会生成以下报告：

| 文件 | 描述 |
|------|------|
| `test_report.html` | HTML 格式的详细测试报告 |
| `test_report.json` | JSON 格式的测试结果 |

## 故障排除

### Q: 测试失败提示找不到 xpkcon

**A:** 确保已编译 xpkcon 并输出到正确的目录：
- Windows x64: `release/x64/xpkcon.exe`
- Windows x86: `release/x86/xpkcon.exe`
- Linux: `release/linux/xpkcon`

### Q: 编译测试程序失败

**A:** 确保已安装 GCC 编译器。

### Q: 某些测试失败

**A:** 检查：
1. xpkcon 是否正确编译
2. 是否有足够的磁盘空间
3. 是否有足够的权限创建和删除文件

### Q: x86 测试无法运行

**A:** 确保系统支持 32 位程序，或使用 `build_test_x86.bat` 编译 x86 版本。

## 贡献

添加新测试时请遵循以下准则：

1. 测试名称应该清晰描述测试内容
2. 每个测试应该独立，不依赖其他测试
3. 使用适当的断言（`ASSERT_EQ`, `ASSERT_STR_EQ` 等）
4. 测试完成后清理临时文件
5. 测试应该覆盖正常情况和边界情况
6. 在 `test_all.c` 中正确注册测试类别
