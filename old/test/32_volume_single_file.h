/*
 * xPack Ver7 - 分卷压缩单文件测试 (32)
 */

#include "test_framework.h"

TEST(volume_single_file_small) {
	xpkObject xpk = xpkOpen("test_32_vol_single_small.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 4096), 0);

	const char* data = "Small file data for volume test";
	uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	ASSERT_NE(pos, UINT32_MAX);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkSave(xpk), 0);

	xpkClose(xpk);

	xpk = xpkOpen("test_32_vol_single_small.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkVolumeMode(xpk), 1);
	ASSERT_EQ(xpkCount(xpk), 1);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, strlen(data));
	ASSERT_EQ(memcmp(extracted, data, outSize), 0);
	xpkFree(extracted);

	xpkClose(xpk);
}

TEST(volume_single_file_medium) {
	xpkObject xpk = xpkOpen("test_32_vol_single_medium.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 2048), 0);

	char data[1024];
	memset(data, 'M', sizeof(data));
	sprintf(data, "Medium file data for volume test - ");

	uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	ASSERT_NE(pos, UINT32_MAX);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkSave(xpk), 0);

	xpkClose(xpk);

	xpk = xpkOpen("test_32_vol_single_medium.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, strlen(data));
	ASSERT_EQ(memcmp(extracted, data, outSize), 0);
	xpkFree(extracted);

	xpkClose(xpk);
}

TEST(volume_single_file_large) {
	xpkObject xpk = xpkOpen("test_32_vol_single_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 4096), 0);

	char data[4096];
	memset(data, 'L', sizeof(data));

	uint32_t pos = xpkAppendData(xpk, data, sizeof(data), 6);
	ASSERT_NE(pos, UINT32_MAX);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkSave(xpk), 0);

	xpkClose(xpk);

	xpk = xpkOpen("test_32_vol_single_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, sizeof(data));
	ASSERT_EQ(memcmp(extracted, data, outSize), 0);
	xpkFree(extracted);

	xpkClose(xpk);
}

void register_32_volume_single_file_tests(void) {
	TEST_REGISTER(volume_single_file_small, CAT_VOLUME, "Test single small file in volume mode");
	TEST_REGISTER(volume_single_file_medium, CAT_VOLUME, "Test single medium file in volume mode");
	TEST_REGISTER(volume_single_file_large, CAT_VOLUME, "Test single large file in volume mode");
}
