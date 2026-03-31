/*
	xPack 聚合实现入口

	负责引入 xrt、第三方压缩库与 src 下的全部实现模块，
	并在测试构建时挂接自测入口。
*/

#define XRT_IMPLEMENTATION
#include "lib/xrt.h"


// 第三方压缩库头文件
#include "lib/lz4/lz4.h"
#include "lib/lz4/lz4hc.h"
#include "lib/zstd/zstd.h"
#include "lib/lzma/Alloc.h"
#include "lib/lzma/Lzma2Dec.h"
#include "lib/lzma/Lzma2Enc.h"


// 打开 xPack 内部实现声明
#define XPACK_BUILD_CORE
#include "xpack.h"


// 基础适配层
#include "src/base/memory.h"
#include "src/base/hash.h"
#include "src/base/time.h"


// 核心状态与基础服务
#include "src/service/core.h"


// 编解码、存储与格式层
#include "src/codec/router.h"
#include "src/storage/fileio.h"
#include "src/format/layout.h"


// 包生命周期与读写服务
#include "src/service/open.h"
#include "src/service/write.h"
#include "src/service/save.h"
#include "src/service/build.h"
#include "src/service/verify.h"
#include "src/service/each.h"


// 对外 API 实现
#include "src/api/package_api.h"
#include "src/api/default_api.h"
#include "src/api/index_api.h"
#include "src/api/path_api.h"
#include "src/api/admin_api.h"


// 自测主入口
#if defined(XPACK_SELF_TEST_MAIN)
#include "tests/test_helpers.h"
#include "tests/selftest_main.h"
#endif
