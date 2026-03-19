/*
 * xPack Ver7 - 分卷压缩基本功能测试 (31-33)
 */

#include "test_framework.h"

TEST(volume_mode_enable_disable) {
	xpkObject xpk = xpkOpen("test_31_vol_mode.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);
	
	ASSERT_EQ(xpkVolumeMode(xpk), 0);
	
	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeMode(xpk), 1);
	
	ASSERT_EQ(xpkVolumeModeSet(xpk, 0), 0);
	ASSERT_EQ(xpkVolumeMode(xpk), 0);
	
	ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeMode(xpk), 1);
	
	xpkClose(xpk);
}

TEST(volume_size_configuration) {
	xpkObject xpk = xpkOpen("test_31_vol_size.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);
	
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 1024), 0);
	ASSERT_EQ(xpkVolumeSize(xpk), 1024);
	
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 2048), 0);
	ASSERT_EQ(xpkVolumeSize(xpk), 2048);
	
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 4096), 0);
	ASSERT_EQ(xpkVolumeSize(xpk), 4096);
	
	ASSERT_EQ(xpkVolumeSizeSet(xpk, 0), 0);
	ASSERT_EQ(xpkVolumeSize(xpk), 0);
	
	xpkClose(xpk);
}

TEST(volume_split_mode) {
	xpkObject xpk = xpkOpen("test_31_vol_split.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);
	
	ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 0), 0);
	ASSERT_EQ(xpkVolumeSplitMode(xpk), 0);
	
	ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 1), 0);
	ASSERT_EQ(xpkVolumeSplitMode(xpk), 1);
	
	ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 0), 0);
	ASSERT_EQ(xpkVolumeSplitMode(xpk), 0);
	
	xpkClose(xpk);
}

void register_31_volume_basic_tests(void) {
	TEST_REGISTER(volume_mode_enable_disable, CAT_VOLUME, "Test enabling and disabling volume mode");
	TEST_REGISTER(volume_size_configuration, CAT_VOLUME, "Test volume size configuration");
	TEST_REGISTER(volume_split_mode, CAT_VOLUME, "Test volume split mode");
}
