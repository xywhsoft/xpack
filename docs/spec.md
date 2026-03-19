# xpk Implementation Spec

版本: 1.0
日期: 2026-03-19
状态: Draft for Build

## 1. 目的

本文档不是再次定义 xpk 文件格式，而是把 [design.md](/D:/Git/xPack/docs/design.md) 落成可执行的工程规格。

本文档回答五个问题：

1. 新实现的工程结构应该长什么样。
2. 单入口 `.c` + 模块 `.h` 的开发模式如何组织。
3. `xpack.h` 作为生成头文件，应由什么来源生成。
4. vendor 依赖应如何参与编译。
5. 实际编码时应按什么顺序推进，才能最快得到可运行版本。

## 2. 基本立场

1. `design.md` 是格式与行为规范，本文件是工程与实现规范。
2. 新实现继续采用 clean-room 立场，不为旧代码保留兼容实现路径。
3. 公开错误模型固定为“错误码 + 对象内错误消息”。
4. 工程组织优先采用 `xrt.c` 与 `xserver/main.c` 的单入口聚合模式。
5. 项目代码层面只保留一个正式实现入口：`xpack.c`。

## 3. 工程约束

### 3.1 单入口约束

1. `xpack.c` 是项目唯一正式实现入口。
2. `src/` 下所有模块文件统一使用 `.h`，但这些 `.h` 是实现文件，不是公共头文件。
3. `src/` 下的实现文件只能由 `xpack.c` 直接包含。
4. 除测试、工具、vendor 依赖外，不再新增项目级 `.c` 实现文件。

### 3.2 公共头文件约束

1. `xpack.h` 是发布给外部使用的公共头文件。
2. `xpack.h` 不手工维护，统一由声明源生成。
3. 公开常量、类型、结构和 API 原型必须有唯一声明源，不能在 `xpack.c` 和 `xpack.h` 中重复手写两份。

### 3.3 依赖约束

1. `xrt` 以单头文件方式嵌入项目，通过 `lib/xrt.h` 使用。
2. `lz4`、`zstd`、`lzma` 保持 vendor 原始源码目录，不并入 `src/`。
3. 非 codec 模块不得直接调用 `lz4`、`zstd`、`lzma` 的原生 API。
4. 非 runtime 模块不得直接依赖 xrt 的文件、时间、错误接口。
5. 默认模式 / Index 模式 / Path 模式对 xrt 容器的使用，必须通过 mode/service 层统一封装。

## 4. 目标目录结构

```text
xPack/
├─ xpack.c
├─ xpack.h
├─ docs/
│  ├─ design.md
│  └─ spec.md
├─ lib/
│  ├─ xrt.h
│  ├─ lz4/
│  ├─ zstd/
│  └─ lzma/
├─ src/
│  ├─ api/
│  │  ├─ public_decl.h
│  │  ├─ package_api.h
│  │  ├─ default_api.h
│  │  ├─ index_api.h
│  │  ├─ path_api.h
│  │  └─ admin_api.h
│  ├─ base/
│  │  ├─ config.h
│  │  ├─ const.h
│  │  ├─ types.h
│  │  ├─ error.h
│  │  ├─ memory.h
│  │  ├─ hash.h
│  │  └─ time.h
│  ├─ model/
│  │  ├─ entry.h
│  │  ├─ object.h
│  │  └─ dirty.h
│  ├─ codec/
│  │  ├─ codec.h
│  │  ├─ router.h
│  │  ├─ lz4.h
│  │  ├─ zstd.h
│  │  └─ lzma2.h
│  ├─ storage/
│  │  ├─ storage.h
│  │  ├─ rawio.h
│  │  ├─ single.h
│  │  ├─ volume.h
│  │  └─ queue.h
│  ├─ format/
│  │  ├─ head.h
│  │  ├─ entry.h
│  │  ├─ meta.h
│  │  └─ layout.h
│  ├─ service/
│  │  ├─ open.h
│  │  ├─ save.h
│  │  ├─ build.h
│  │  ├─ write.h
│  │  ├─ meta.h
│  │  └─ verify.h
│  ├─ mode/
│  │  ├─ default.h
│  │  ├─ index.h
│  │  ├─ path.h
│  │  └─ normalize.h
│  └─ internal/
│     ├─ forward.h
│     └─ trace.h
├─ tests/
│  ├─ smoke/
│  ├─ unit/
│  ├─ spec/
│  └─ integration/
└─ tools/
   └─ make_header/
```

说明：

1. `xpack.c` 负责聚合所有实现文件，类似 `xrt.c` 的组织方式。
2. `src/` 下的 `.h` 文件按模块分层，类似 `xserver/main.c` 对 `src/` 的聚合方式。
3. `xpack.h` 是构建产物，不作为声明事实来源。

## 5. 核心文件职责

### 5.1 xpack.c

`xpack.c` 负责：

1. 引入系统头。
2. 定义 `XRT_IMPLEMENTATION` 并包含 `lib/xrt.h`。
3. 依赖顺序包含 `src/` 下各模块实现头。
4. 形成项目唯一实现翻译单元。

`xpack.c` 不负责：

1. 手写重复的公共 API 声明。
2. 直接实现所有逻辑细节。
3. 直接夹带 vendor 源码。

### 5.2 xpack.h

`xpack.h` 负责：

1. 对外暴露公开常量、公开结构、公开枚举、公开 API 原型。
2. 保持与实现一致的 ABI 视图。
3. 作为发布产物和外部工程引用入口。

`xpack.h` 约束：

1. 不暴露内部对象结构。
2. 不暴露 `src/` 下内部 helper。
3. 不包含 vendor 私有头。
4. 可以依赖 `xrt.h`，因为公开结构中使用了 `xtime`。

### 5.3 src/api/public_decl.h

这是公开声明的唯一事实来源。

它应包含：

1. `XPKAPI` 导出宏。
2. 公开常量。
3. 公开枚举。
4. 公开结构。
5. 公开 API 原型。

它不应包含：

1. 任何函数实现。
2. 任何内部对象定义。
3. 任何 vendor 依赖细节。

## 6. xpack.c 的包含顺序

`xpack.c` 的包含顺序固定如下：

```c
#include <...system headers...>

#define XRT_IMPLEMENTATION
#include "lib/xrt.h"

#include "src/api/public_decl.h"

#include "src/base/config.h"
#include "src/base/const.h"
#include "src/base/types.h"
#include "src/base/error.h"
#include "src/base/memory.h"
#include "src/base/hash.h"
#include "src/base/time.h"

#include "src/internal/forward.h"

#include "src/model/entry.h"
#include "src/model/object.h"
#include "src/model/dirty.h"

#include "src/codec/codec.h"
#include "src/codec/router.h"
#include "src/codec/lz4.h"
#include "src/codec/zstd.h"
#include "src/codec/lzma2.h"

#include "src/storage/storage.h"
#include "src/storage/rawio.h"
#include "src/storage/single.h"
#include "src/storage/volume.h"
#include "src/storage/queue.h"

#include "src/format/head.h"
#include "src/format/entry.h"
#include "src/format/meta.h"
#include "src/format/layout.h"

#include "src/service/open.h"
#include "src/service/write.h"
#include "src/service/meta.h"
#include "src/service/save.h"
#include "src/service/build.h"
#include "src/service/verify.h"

#include "src/mode/default.h"
#include "src/mode/index.h"
#include "src/mode/path.h"
#include "src/mode/normalize.h"

#include "src/api/package_api.h"
#include "src/api/default_api.h"
#include "src/api/index_api.h"
#include "src/api/path_api.h"
#include "src/api/admin_api.h"
```

规则：

1. 公共声明先于实现模块进入翻译单元。
2. 基础类型和错误系统先于任何业务模块定义。
3. service 层先于公开 API 实现。
4. API 层始终最后包含。

## 7. 公共头文件生成策略

### 7.1 生成原则

1. `xpack.h` 必须由工具生成。
2. 生成工具应类似 `xrt` 的 single-header builder，但目标更简单。
3. 生成后的 `xpack.h` 应可直接被外部项目包含。

### 7.2 建议来源

推荐由以下片段生成：

1. 固定前导模板。
2. `src/api/public_decl.h`。
3. 固定尾模板。

推荐的生成器位置：

```text
tools/make_header/make_header.c
```

### 7.3 生成后的内容约束

生成后的 `xpack.h` 必须包含：

1. include guard。
2. `extern "C"` 包裹。
3. `#include <stdint.h>`。
4. `#include <xrt.h>`。
5. 所有公开类型与 API。

生成后的 `xpack.h` 不得包含：

1. `src/` 路径。
2. 内部 helper 宏。
3. `static` 内部函数。
4. vendor 头文件路径。

## 8. xrt 依赖策略

实现层面固定如下：

1. `xpack.c` 内定义 `XRT_IMPLEMENTATION`，直接包含 `lib/xrt.h`。
2. 不再单独编译 `xrt.c`。
3. 文件与目录 I/O 走 xrt 的文件接口。
4. 时间戳与时间处理走 xrt 的时间接口。
5. 默认模式条目容器走 xrt 的 `xarray`。
6. Index 模式条目容器走 xrt 的 `xlist`。
7. Linux / Win32 模式条目容器走 xrt 的 `xdict`。

引用来源以独立工程目录为准：

1. `D:\Git\xrt\xrt.h`
2. `D:\Git\xrt\lib\file.h`
3. `D:\Git\xrt\lib\time.h`
4. `D:\Git\xrt\lib\array.h`
5. `D:\Git\xrt\lib\list.h`
6. `D:\Git\xrt\lib\dict.h`

## 9. vendor 编译策略

### 9.1 默认策略

项目代码维持单入口，但 vendor 依赖单独参与编译。

需要参与编译的 vendor 源文件如下：

```text
lib/lz4/lz4.c
lib/lz4/lz4hc.c
lib/zstd/zstd.c
lib/lzma/Alloc.c
lib/lzma/CpuArch.c
lib/lzma/LzFind.c
lib/lzma/LzmaDec.c
lib/lzma/LzmaEnc.c
lib/lzma/Lzma2Dec.c
lib/lzma/Lzma2Enc.c
```

### 9.2 必要编译宏

```text
-DZ7_ST
```

### 9.3 建议编译脚本形态

Windows 共享库示例：

```bat
gcc -m64 -shared ^
    xpack.c ^
    lib/lz4/lz4.c ^
    lib/lz4/lz4hc.c ^
    lib/zstd/zstd.c ^
    lib/lzma/Alloc.c ^
    lib/lzma/CpuArch.c ^
    lib/lzma/LzFind.c ^
    lib/lzma/LzmaDec.c ^
    lib/lzma/LzmaEnc.c ^
    lib/lzma/Lzma2Dec.c ^
    lib/lzma/Lzma2Enc.c ^
    -Ilib -Ilib/lz4 -Ilib/zstd -Ilib/lzma ^
    -DZ7_ST -O2 -s ^
    -ffunction-sections -fdata-sections -Wl,--gc-sections ^
    -lws2_32 -liphlpapi ^
    -o release/xpack.dll
```

Linux 测试构建示例：

```sh
gcc tests/smoke/open_close.c xpack.c \
    lib/lz4/lz4.c \
    lib/lz4/lz4hc.c \
    lib/zstd/zstd.c \
    lib/lzma/Alloc.c \
    lib/lzma/CpuArch.c \
    lib/lzma/LzFind.c \
    lib/lzma/LzmaDec.c \
    lib/lzma/LzmaEnc.c \
    lib/lzma/Lzma2Dec.c \
    lib/lzma/Lzma2Enc.c \
    -Ilib -Ilib/lz4 -Ilib/zstd -Ilib/lzma \
    -DZ7_ST -O2 -ldl -lpthread \
    -o release/xpk_smoke
```

## 10. 错误模型

### 10.1 公开规则

1. 公开函数返回 `0` 表示成功，负值表示失败。
2. 每个 `xpkObject` 内保存最近一次错误码和错误文本。
3. 错误文本长度固定，建议 `256` 字节。
4. `xpkLastError(xpk)` 返回对象内最近错误码。
5. `xpkLastErrorMessage(xpk)` 返回对象内最近错误文本。

### 10.2 对象内实现

建议对象中固定包含：

```c
typedef struct xpkErrorState {
    int  code;
    char text[256];
} xpkErrorState;
```

建议基础 helper：

```c
static void xpkClearError(xpkObject xpk);
static int  xpkSetError(xpkObject xpk, int code, const char* text);
static int  xpkSetErrorFmt(xpkObject xpk, int code, const char* fmt, ...);
```

规则：

1. 所有失败路径都必须调用 `xpkSetError*`。
2. 所有成功返回前都必须保持对象错误状态可预测。
3. 非对象上下文不得使用全局错误单例替代对象错误。

### 10.3 构造期例外

`xpkOpen` 在对象尚未成功建立前可能发生失败。

实现要求如下：

1. 优先尽早分配最小对象壳，以便后续失败能写入对象错误。
2. 仅当对象壳分配本身失败时，才允许无法携带对象错误文本。
3. 这类极早期失败不改变“对象错误为主”的整体设计。

## 11. 模块边界

### 11.1 base

负责常量、基础类型、错误、内存、时间、哈希包装。

### 11.2 model

负责运行时对象、条目结构、脏标记和状态机。

### 11.3 codec

负责压缩级别映射、压缩路由、hash 校验和 STORE 回退。

### 11.4 storage

负责单卷/分卷逻辑文件视图、跨卷读写映射和写队列。

### 11.5 format

负责 `xpkHead`、`Package Meta`、`Entry Table` 的编码与解码。

### 11.6 service

负责 open/save/build/write/meta/verify 的核心流程。

### 11.7 mode

负责 default/index/path 三种访问模型及路径归一化策略。

### 11.8 api

负责参数校验、对象可见 API 和错误返回。

## 12. 实现顺序

### Phase 0: 工程骨架

交付物：

1. `xpack.c` 空骨架。
2. `src/` 目录结构。
3. `src/api/public_decl.h`。
4. `tools/make_header/` 头文件生成器骨架。
5. 基础编译脚本。

通过标准：

1. 工程可以空实现编译通过。
2. `xpack.h` 可以成功生成。

### Phase 1: base + model

交付物：

1. 常量和公开类型。
2. `xpkObject` 内部结构。
3. 错误系统。
4. 脏标记与状态机 helper。

通过标准：

1. `xpkOpen` 可创建空对象。
2. `xpkClose` 可释放空对象。
3. 错误 API 可工作。

### Phase 2: codec

交付物：

1. 压缩级别映射表。
2. LZ4 / ZSTD / LZMA2 封装。
3. STORE 回退逻辑。

通过标准：

1. 压缩/解压 smoke test 通过。
2. hash 校验失败路径正确。

### Phase 3: storage

交付物：

1. 单卷读写。
2. 分卷虚拟 I/O。
3. 写队列。

通过标准：

1. 跨卷边界读写测试通过。
2. `volumeSize` 最小值校验通过。

### Phase 4: format

交付物：

1. 64B 头编码/解码。
2. Entry Table 编码/解码。
3. Package Meta 编码/解码。
4. normal/solid 布局解释。

通过标准：

1. 空包可读写。
2. 头字段与表字段 round-trip 正确。

### Phase 5: default 模式主路径

交付物：

1. `xpkAdd*` / `xpkRead*` / `xpkUpdate*` / `xpkRemove`。
2. `xpkSave`。
3. `xpkBuild`。

通过标准：

1. 默认模式完整增删改查通过。
2. 立即写入与缓存写入语义符合规范。

### Phase 6: Index 与 Path 模式

交付物：

1. `xpkIndex*`。
2. `xpkPath*`。
3. Linux / Win32 归一化规则。

通过标准：

1. `xlist` / `xdict` 驱动的访问路径通过。
2. Path 320B 条目编码正确。

### Phase 7: Solid 模式

交付物：

1. solid 读取路径。
2. solid 下元数据可写、数据段只读边界。
3. solid 下删除 / 重命名 / 属性修改路径。

通过标准：

1. solid 读路径通过。
2. solid 数据写接口全部拒绝。
3. solid 元数据修改和 `save` 通过。

### Phase 8: 验证与收口

交付物：

1. verify/stat API。
2. 头文件生成工具完成版。
3. 构建脚本与测试资产整理。

通过标准：

1. smoke / unit / spec / integration 测试可运行。
2. `xpack.h` 与实现保持同步。

## 13. 首批必须创建的文件

建议第一轮先落这些文件：

```text
xpack.c
src/api/public_decl.h
src/base/config.h
src/base/const.h
src/base/types.h
src/base/error.h
src/model/object.h
src/model/entry.h
src/model/dirty.h
tools/make_header/make_header.c
```

原因：

1. 这批文件决定 ABI。
2. 这批文件决定包含顺序。
3. 这批文件决定错误模型和对象模型。
4. 后续 codec/storage/format 都要建立在这批文件之上。

## 14. 明确禁止

1. 不再恢复旧的多 `.c` 项目实现结构。
2. 不在 `src/` 中混放公开头与实现头。
3. 不在 codec 以外直接调用 vendor API。
4. 不在 runtime 以外直接调用 xrt 文件/时间错误接口。
5. 不引入额外分卷元数据。
6. 不为旧格式、旧命名或旧半成品结构做兼容实现。
