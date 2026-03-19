# xpkcon 构建说明

## 前置条件

确保系统已安装 TCC (Tiny C Compiler) 编译器。TCC 可从以下位置获取：
- http://bellard.org/tcc/
- 或者在 xPack 项目中的 tcc 目录

## 构建步骤

### 方法一：使用现有 DLL（推荐）

如果 `release/x64/xpack.dll` 已存在：

```batch
cd tools\xpkcon
build_x64.bat
```

这将生成 `xpkcon.exe`，但需要 `xpack.dll` 在同一目录或 PATH 中。

### 方法二：静态链接（独立可执行文件）

如果没有 DLL 或希望生成独立的可执行文件：

```batch
cd tools\xpkcon
REM 确保 build_x64.bat 中注释掉了 DLL 部分
REM 脚本会自动尝试静态链接
build_x64.bat
```

## 手动编译

如果自动构建脚本遇到问题，可以手动编译：

### x64 版本（DLL 模式）
```batch
tcc -o xpkcon.exe ^
    -I../../lib -I../../src ^
    ../../lib/xrt/xrt.c ^
    xpkcon.c ^
    -m64 ^
    -DXPK_BUILD_DLL ^
    -L../../release/x64 ^
    -lxpack ^
    -lkernel32 -luser32 -ladvapi32
```

### x64 版本（静态链接）
```batch
tcc -o xpkcon.exe ^
    -I../../lib -I../../src ^
    ../../lib/xrt/xrt.c ^
    ../../lib/lz4/lz4.c ^
    ../../lib/lz4/lz4hc.c ^
    ../../lib/lzma/Alloc.c ^
    ../../lib/lzma/CpuArch.c ^
    ../../lib/lzma/LzFind.c ^
    ../../lib/lzma/LzmaDec.c ^
    ../../lib/lzma/LzmaEnc.c ^
    ../../lib/lzma/Lzma2Dec.c ^
    ../../lib/lzma/Lzma2Enc.c ^
    ../../lib/lzma/tcc_stub.c ^
    ../../lib/zstd/zstd.c ^
    ../../src/xpack.c ^
    ../../src/xpack_core.c ^
    ../../src/xpack_index.c ^
    ../../src/xpack_path.c ^
    ../../src/xpack_util.c ^
    ../../src/xpack_compress.c ^
    ../../src/xpack_ldb.c ^
    xpkcon.c ^
    -m64 ^
    -DZSTD_NO_INTRINSICS ^
    -DZ7_ST ^
    -DXPK_BUILD_LIB ^
    -lkernel32 -luser32 -ladvapi32
```

## 测试

编译成功后，可以测试基本功能：

```batch
xpkcon -h
xpkcon --version

# 创建测试文件
echo test content > test.txt

# 创建压缩包
xpkcon a test.xpk test.txt

# 列出内容
xpkcon l test.xpk

# 查看信息
xpkcon i test.xpk
```

## 故障排除

### 错误：找不到 xpack.dll
- 将 `release/x64/xpack.dll` 复制到 `tools/xpkcon/` 目录
- 或者将 `release/x64/` 添加到 PATH 环境变量

### 错误：找不到 tcc 命令
- 确保已安装 TCC 编译器
- 将 TCC 添加到 PATH 环境变量

### 编译错误
- 检查所有源文件路径是否正确
- 确保使用正确的架构参数（-m64 或 -m32）
