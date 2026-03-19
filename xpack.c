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

#include "src/service/core.h"

#include "lib/lz4/lz4.h"
#include "lib/lz4/lz4hc.h"
#include "lib/zstd/zstd.h"
#include "lib/lzma/Alloc.h"
#include "lib/lzma/Lzma2Dec.h"
#include "lib/lzma/Lzma2Enc.h"

#include "src/codec/router.h"
#include "src/storage/fileio.h"
#include "src/format/layout.h"
#include "src/service/open.h"
#include "src/service/write.h"
#include "src/service/save.h"
#include "src/service/build.h"
#include "src/service/verify.h"
#include "src/service/each.h"

#include "src/api/package_api.h"
#include "src/api/default_api.h"
#include "src/api/index_api.h"
#include "src/api/path_api.h"
#include "src/api/admin_api.h"

#if defined(XPACK_SELF_TEST_MAIN)
#include "tests/test_helpers.h"
#include "tests/selftest_main.h"
#elif !defined(XPACK_NO_MAIN)
int main(void)
{
	return 0;
}
#endif
