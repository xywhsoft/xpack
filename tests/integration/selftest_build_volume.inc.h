	/* build compaction / temp-path occupancy / package rebuild safety */
#ifdef __linux__
	if ( bRunStress ) {
		iEachCount = procTestCountOpenFd();
	}
#endif
#include "build_compact.inc.h"
#ifdef __linux__
	if ( bRunStress && procTestCountOpenFd() != iEachCount ) {
		return 5901;
	}
#endif

	/* codec mapping / fallback / lz4-zstd-lzma roundtrip */
#ifdef __linux__
	if ( bRunStress ) {
		iEachCount = procTestCountOpenFd();
	}
#endif
#include "codec_levels.inc.h"
#ifdef __linux__
	if ( bRunStress && procTestCountOpenFd() != iEachCount ) {
		return 5902;
	}
#endif

	/* volume mode / volume build / stale volume cleanup */
#ifdef __linux__
	if ( bRunStress ) {
		iEachCount = procTestCountOpenFd();
	}
#endif
#include "volume_mode.inc.h"
#ifdef __linux__
	if ( bRunStress && procTestCountOpenFd() != iEachCount ) {
		return 5903;
	}
#endif

	/* replace original / save rollback / replace cleanup guards */
#ifdef __linux__
	if ( bRunStress ) {
		iEachCount = procTestCountOpenFd();
	}
#endif
#include "replace_rollback.inc.h"
#ifdef __linux__
	if ( bRunStress && procTestCountOpenFd() != iEachCount ) {
		return 5904;
	}
#endif
