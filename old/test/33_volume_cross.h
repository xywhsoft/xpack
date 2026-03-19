/*
 * xPack Ver7 - 分卷压缩跨卷测试 (33)
 */

#include "test_framework.h"

TEST(volume_multiple_files_across_volumes) {
	xpkObject xpk = xpkOpen("test_33_vol_multi.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 2048), 0);
	ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 0), 0);

	char data[3][1024];
	for (int i = 0; i < 3; i++) {
		memset(data[i], 'A' + i, sizeof(data[i]));
		sprintf(data[i], "File %d - ", i);
	}

	uint32_t pos1 = xpkAppendData(xpk, data[0], 1024, 6);
	ASSERT_NE(pos1, UINT32_MAX);

	uint32_t pos2 = xpkAppendData(xpk, data[1], 1024, 6);
	ASSERT_NE(pos2, UINT32_MAX);

	uint32_t pos3 = xpkAppendData(xpk, data[2], 1024, 6);
	ASSERT_NE(pos3, UINT32_MAX);

	ASSERT_EQ(xpkCount(xpk), 3);
	ASSERT_EQ(xpkSave(xpk), 0);

	int volCount = xpkVolumeCount(xpk);
	ASSERT_GT(volCount, 0);

	xpkClose(xpk);

	xpk = xpkOpen("test_33_vol_multi.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkVolumeMode(xpk), 1);
	ASSERT_EQ(xpkCount(xpk), 3);

	for (int i = 0; i < 3; i++) {
		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, 1024);
		ASSERT_EQ(memcmp(extracted, data[i], outSize), 0);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(volume_read_write_cross_volumes) {
	xpkObject xpk = xpkOpen("test_33_vol_rw.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 1024), 0);

	for (int i = 0; i < 5; i++) {
		char data[512];
		sprintf(data, "Data block %d - ", i);
		memset(data + strlen(data), '0' + i, 512 - strlen(data));

		uint32_t pos = xpkAppendData(xpk, data, 512, 6);
		ASSERT_NE(pos, UINT32_MAX);
	}

	ASSERT_EQ(xpkCount(xpk), 5);
	ASSERT_EQ(xpkSave(xpk), 0);

	xpkClose(xpk);

	xpk = xpkOpen("test_33_vol_rw.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 5);

	for (int i = 0; i < 5; i++) {
		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, 512);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(volume_statistics) {
	xpkObject xpk = xpkOpen("test_33_vol_stats.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 2048), 0);

	for (int i = 0; i < 3; i++) {
		char data[768];
		memset(data, 'S' + i, sizeof(data));
		uint32_t pos = xpkAppendData(xpk, data, sizeof(data), 6);
		ASSERT_NE(pos, UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int volCount = xpkVolumeCount(xpk);
	ASSERT_GT(volCount, 0);
	ASSERT_EQ(xpkCount(xpk), 3);

	xpkClose(xpk);
}

TEST(volume_disable_with_data) {
	xpkObject xpk = xpkOpen("test_33_vol_disable.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 1024), 0);

	char data[512];
	memset(data, 'X', sizeof(data));
	uint32_t pos = xpkAppendData(xpk, data, 512, 6);
	ASSERT_NE(pos, UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);

	int volCountBefore = xpkVolumeCount(xpk);
	ASSERT_GT(volCountBefore, 0);

	xpkClose(xpk);

	xpk = xpkOpen("test_33_vol_disable.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkVolumeMode(xpk), 1);
	ASSERT_EQ(xpkCount(xpk), 1);

	xpkClose(xpk);
}

void register_33_volume_cross_tests(void) {
	TEST_REGISTER(volume_multiple_files_across_volumes, CAT_VOLUME, "Test multiple files across volumes");
	TEST_REGISTER(volume_read_write_cross_volumes, CAT_VOLUME, "Test read/write across volumes");
	TEST_REGISTER(volume_statistics, CAT_VOLUME, "Test volume statistics");
	TEST_REGISTER(volume_disable_with_data, CAT_VOLUME, "Test disable volume mode with data");
}
