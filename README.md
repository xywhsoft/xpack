<div align="center">

# xPack

[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platforms](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue.svg)](#)
[![Focus](https://img.shields.io/badge/focus-runtime%20package%20library-orange.svg)](#)

**面向动态读写与运行时资源分发的跨平台 C 语言包格式与打包库**

[English](README.en.md) | 简体中文

</div>

**文档入口：** [设计规格](docs/design.md) | [实现规格](docs/spec.md) | [开发说明](docs/development.md) | [测试说明](docs/testing.md) | [覆盖矩阵](docs/test_matrix.md) | [Windows 状态](docs/windows_status.md) | [压测说明](docs/stress.md)


## 项目定位

**xPack 不是通用序列化库，也不是单纯追求极限压缩率的冷归档格式。**

xPack 的目标，是成为一套面向运行时素材包的 C 语言打包基础设施：

- 优先服务于游戏资源包、微端内容分发、运行时素材加载这类场景
- 支持包内文件的增删改查，而不是只做“一次打完就不再改”的离线归档
- 在运行时读取效率、动态写入便利性、空间回收能力之间做工程化平衡
- 统一提供位置访问、索引访问、路径访问三类对外模型
- 从设计上就考虑 Windows 与 Linux 的路径语义差异、只读布局、分卷布局与大文件边界

xPack 的核心立场很明确：

- **动态读写优先于极限压缩**
- **`save` 负责提交一致性，`build` 才负责全量重构与空间回收**
- **它首先是“运行时资源包主线”，其次才是“可归档”**


## xPack 具备哪些能力

xPack 当前不是零散 API 的集合，而是已经形成一条比较完整的包管理主线。  
如果从使用者视角去看，大致可以拆成下面这些层次：

### 1. 四种统一的包访问模型

xPack 支持四种包类型，但它们共享同一套底层格式，而不是四套彼此割裂的实现：

- `Core`
  - 按顺序位置访问条目
  - 适合最直接、最轻量的包读写
- `Index`
  - 按整数索引访问条目
  - 适合资源 ID、配置 ID、表项 ID 这类场景
- `Linux`
  - 按路径访问
  - 大小写敏感
  - 保持 Linux 风格路径语义
- `Win32`
  - 按路径访问
  - 大小写不敏感
  - 自动做 Win32 风格路径归一化

这意味着同一个库既可以服务：

- 按位置装载的简单资源包
- 按 ID 查找的索引包
- 按路径查找的素材包
- 需要区分 Linux / Windows 路径规则的跨平台分发


### 2. 面向动态修改的包写入模型

xPack 的主线不是“每改一个文件就重写整个包”，而是尽量让动态写入保持可控：

- 支持 `add / update / remove / rename / attr`
- 支持包级 `meta`
- 支持条目级 `infoExt`
- 支持 `buffered` 与 `immediate` 两种写入策略
- 支持只读打开、失败回滚、替换原包与重建临时路径

其中最重要的语义是：

- `xpkSave`
  - 提交当前修改
  - 让包重新回到一致状态
  - **不会主动回收历史空洞**
- `xpkBuild`
  - 重构整个包
  - 回收历史空洞
  - 重新整理布局
  - 也承担 `normal / solid / volume` 等布局切换时的正式重建职责

这让 xPack 更适合“反复更新素材包”的工程，而不是只适合一次性归档。


### 3. 压缩、Solid 与分卷能力

xPack 当前支持这些压缩算法：

- `STORE`
- `LZ4`
- `LZ4HC`
- `ZSTD`
- `LZMA2`

它不是简单把压缩当成“写包时顺手压一下”，而是把布局能力纳入正式模型：

- `normal mode`
  - 普通逐文件压缩
  - 适合动态读写主路径
- `solid mode`
  - 数据段整体压缩
  - 读取与分发友好
  - 但数据段进入 solid 后，不再允许继续追加或更新文件数据
- `volume mode`
  - 通过 `volumeSize` 把单个逻辑包映射为连续分卷
  - 适合受单文件大小限制的存储环境

其中 `solid` 不是另一种包类型，而是一种数据段布局模式；  
`volume` 也不是额外的索引系统，而是面向底层存储的连续视图。


### 4. 面向运行时读取与导出的读取主线

xPack 不只是“能写进去”，还提供了比较完整的读取、导出和校验能力：

- `xpkReadToMemory / xpkReadToFile`
- `xpkVerify / xpkVerifyAll`
- `xpkEach / xpkEachMatch`
- `xpkStatGet`
- `xpkLastError / xpkLastErrorMessage`

当前工程已经重点打通这些方向：

- 普通条目的直接读取与导出
- `solid` 条目的按目标切片读取与校验
- 非分卷包尾段的映射式打开与解码
- `4GB+` 文件定位与大逻辑偏移分卷原始 I/O 回归
- 大文件输入、导出和重建链路上的低峰值路径

换句话说，xPack 当前关注的不是“API 表面能跑”，而是让：

- 打开
- 读取
- 校验
- 导出
- 重建

这些主路径在大文件和复杂布局下仍然保持可控。


### 5. 面向分发与嵌入的工程形态

xPack 当前提供三种比较明确的工程使用方式：

- `xpack.c + xpack.h`
  - 项目正式实现入口与公共头
- `singlehead/xpack.h`
  - 适合单头分发或嵌入式集成
- `DLL / SO`
  - 仓库内已提供 Windows 与 Linux 的构建脚本

工程上当前采用：

- 单实现入口
- 生成式公共头文件
- 独立测试入口
- vendor 压缩库独立编译
- 依赖 `xrt` 作为基础运行时


## 为什么 xPack 不是普通归档库

很多打包格式强调的是：

- 压得更小
- 兼容更多历史工具
- 一次打包后长期不再修改

xPack 的优先级不同：

- 它更强调**动态更新素材包时不要频繁重写全包**
- 更强调**运行时读取和校验主路径要清晰**
- 更强调**`save` 与 `build` 的职责边界要稳定**
- 更强调**路径包、索引包、普通包要共享同一条底层主线**

所以如果你的目标是：

- 做一个 ZIP 替代品
- 面向长期冷归档
- 依赖外部通用解包工具生态

那么 xPack 不是优先为这个目标设计的。

但如果你的目标是：

- 游戏资源包
- 客户端运行时素材包
- 微端内容下载与落地
- 需要多轮增删改查和重构的包格式

那么 xPack 的设计会更贴近这个问题本身。


## 核心特征

### 1. 动态读写优先

xPack 把“改包”当成正式场景，而不是附带能力。

### 2. 运行时主路径明确

从打开、读取、导出、校验到重建，都是同一条工程主线，不是若干互不相干的辅助函数。

### 3. 统一而不是分裂

`Core / Index / Linux / Win32` 共享一套格式模型，而不是四套单独维护的底层实现。

### 4. 压缩与布局是正式能力

`STORE / LZ4 / LZ4HC / ZSTD / LZMA2`、`solid`、`volume` 都在公开能力模型里，不是后加补丁。

### 5. 面向大文件与低峰值路径优化

当前实现已经对大文件输入、分卷 I/O、压缩条目导出、`solid` 切片校验等主路径做了持续回归和低峰值处理。


## 当前成熟度信号

xPack 目前已经具备比较明确的工程成熟度信号：

- Windows 下，`open / close / save / build` 主链路已经稳定打通
- `Core / Index / Linux / Win32` 四种包类型都已有正式回归覆盖
- `read / add / update / remove / rename / attr / meta / verify` 已形成完整主线
- `solid / volume / buffered / immediate` 都已有跨模块集成回归
- `4GB+` 文件定位与大逻辑偏移分卷 I/O 已有专门验证
- Windows 已有独立压测脚本；Debian 13 主回归已通过，Linux 压测也已进入持续推进阶段

同时也要明确当前边界：

- 更长时间的发布级连续压测还在继续补
- 接近 `4GB` 分卷上限与更大逻辑文件还需要更多真实压力验证
- 单文件数据块与编解码本身仍然受当前 block 模型边界约束


## 典型适用场景

- 游戏客户端资源包
- 微端或启动器分阶段下载的内容包
- 运行时需要按路径或按 ID 查找的素材包
- 热更新、版本补丁、内容重建链路
- 需要在 Windows / Linux 间保持可控路径语义的资源分发


## 设计原则

xPack 当前设计主线可以概括成几条非常明确的原则：

- 固定 `64B` 文件头
- 统一 `32B` 基础条目 + `infoExt` 扩展模型
- `save` 负责一致性，`build` 负责重构与压紧
- `solid` 是数据段只读模式，不是另一套格式
- `volume` 是存储视图，不是额外段表系统
- 规格优先于实现，行为优先于历史兼容


## 当前状态说明

xPack 已经不是“概念验证”阶段的小实验。  
它已经具备完整主线，并且很多核心场景已经可以按正式工程思路使用。

但它当前更准确的状态仍然是：

- 主能力已经成型
- Windows 侧可用于受控生产场景
- Linux 主回归已通过并在持续补压测
- 项目仍在继续统一文档、补更大边界验证和更长时间压力验证

也就是说，xPack 当前追求的不是“能用就停”，而是把这条资源包基础设施主线继续做扎实。


## 快速开始

下面是一个最小的 `Win32 Path` 包示例。  
它会创建一个包、写入一个路径条目、保存、重新打开、校验并读回内容：

```c
#include <stdio.h>
#include <string.h>

#include "xpack.h"

int main(void)
{
	xpkObject pkg = NULL;
	xpkOpenOptions openOpt = {0};
	xpkWriteOptions writeOpt = {0};
	const char* text = "hello xpack";
	void* data = NULL;
	uint64_t dataSize = 0;

	openOpt.createIfMissing = 1;

	writeOpt.compLevel = 6;
	writeOpt.writePolicy = XPK_WRITE_BUFFERED;

	pkg = xpkOpen("assets.xpk", &openOpt);
	if (pkg == NULL) {
		return 1;
	}

	if (xpkSetPackType(pkg, XPK_PACK_WIN32) != XPK_OK) {
		goto fail;
	}
	if (xpkPathAddData(pkg, "assets/hello.txt", text, (uint64_t)strlen(text), &writeOpt) != XPK_OK) {
		goto fail;
	}
	if (xpkSave(pkg) != XPK_OK) {
		goto fail;
	}
	if (xpkClose(pkg) != XPK_OK) {
		return 2;
	}
	pkg = NULL;

	pkg = xpkOpen("assets.xpk", NULL);
	if (pkg == NULL) {
		return 3;
	}
	if (xpkVerifyAll(pkg) != XPK_OK) {
		goto fail;
	}

	data = xpkPathReadToMemory(pkg, "assets/hello.txt", &dataSize);
	if (data == NULL) {
		goto fail;
	}

	fwrite(data, 1, (size_t)dataSize, stdout);
	putchar('\n');

	xpkFree(data);
	data = NULL;

	return xpkClose(pkg) == XPK_OK ? 0 : 4;

fail:
	if (data != NULL) {
		xpkFree(data);
	}
	if (pkg != NULL) {
		fprintf(stderr, "xpack error: %s\n", xpkLastErrorMessage(pkg));
		xpkClose(pkg);
	}
	return 5;
}
```

如果你需要的是：

- 按位置访问：使用 `xpkAddData / xpkReadToMemory`
- 按整数索引访问：先设为 `XPK_PACK_INDEX`，再使用 `xpkIndex*`
- 按 Linux 路径访问：先设为 `XPK_PACK_LINUX`，再使用 `xpkPath*`


## 构建与验证

仓库里已经提供了直接可用的构建脚本。

Windows 下常用：

```bat
build_header.bat
build_GCC_DLL_x64.bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
```

Linux 下常用：

```sh
./build.sh
./build_test.sh
release/x64/xpack_test all
```

如果需要独立压测入口：

```bat
build_GCC_STRESS_x64.bat
```

```sh
./build_stress.sh
```


## 单头文件分发

如果你希望用单头文件方式分发或嵌入，可以先生成：

```bat
build_single_head.bat
```

或：

```sh
./build_single_head.sh
```

生成结果位于：

```text
singlehead/xpack.h
```

使用方式大致如下：

```c
#define XRT_IMPLEMENTATION
#include "xrt.h"

#define XPACK_IMPLEMENTATION
#include "xpack.h"
```

需要注意的是，单头版本仍然依赖外部的：

- `xrt.h`
- `lz4 / lz4hc`
- `zstd`
- `lzma`

也就是说，单头文件解决的是 **xPack 自身的分发形态**，不是把所有第三方实现静态内联成一个最终头文件。


## 文档

- [设计规格](docs/design.md)：格式设计、产品定位、模式边界与公开能力模型
- [实现规格](docs/spec.md)：工程结构、公开头生成、编译组织与错误模型
- [开发说明](docs/development.md)：目录职责、构建脚本、推荐开发流程与当前边界
- [测试说明](docs/testing.md)：测试架构、helper 约定、过滤值与执行方式
- [覆盖矩阵](docs/test_matrix.md)：功能到测试文件的映射索引
- [Windows 状态](docs/windows_status.md)：Windows 当前能力结论与剩余边界
- [压测说明](docs/stress.md)：Windows / Linux 压测入口、日志与历史记录


## 依赖

xPack 当前依赖并整合了这些基础组件：

- `xrt`
- `LZ4 / LZ4HC`
- `Zstandard`
- `LZMA SDK`

其中 `xrt` 负责基础运行时能力，压缩库负责 `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` 这条编解码主线。


## 最后

如果你正在寻找的不是一个“只负责离线打包”的归档工具，  
而是一套能够支撑：

- 动态改包
- 运行时读取
- 路径 / 索引 / 位置三类访问
- `solid / volume / recompress / rebuild`
- Windows / Linux 资源分发

的 **C 语言资源包基础设施**，

那么 xPack 正在朝这个方向持续推进。
