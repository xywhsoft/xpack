/*
 * xPack Ver7 - 压缩数据模式测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <xrt/xrt.h>
#include "../src/xpack.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED\n"); \
        printf("    Assertion failed: %s\n", #cond); \
        printf("    At line %d\n", __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))
#define ASSERT_LE(a, b) ASSERT((a) <= (b))

TEST(compression_repeated_data) {
	xpkObject xpk = xpkOpen("test_07_repeated.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_repeated.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.05f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, data, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(data);
}

TEST(compression_random_data) {
	xpkObject xpk = xpkOpen("test_07_random.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_random.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);

	float ratio = (float)packedSize / size;
	ASSERT_GT(ratio, 0.95f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, data, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(data);
}

TEST(compression_zeroes) {
	xpkObject xpk = xpkOpen("test_07_zeroes.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 0, size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_zeroes.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.01f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, data, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(data);
}

TEST(compression_ones) {
	xpkObject xpk = xpkOpen("test_07_ones.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 0xFF, size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_ones.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.01f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, data, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(data);
}

TEST(compression_text_english) {
	xpkObject xpk = xpkOpen("test_07_text_english.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 50000;
	char* text = malloc(size);
	ASSERT_NOT_NULL(text);

	size_t pos = 0;
	while (pos < size - 100) {
		strcpy(text + pos, "The quick brown fox jumps over the lazy dog. ");
		pos += 44;
	}

	xpkAppendData(xpk, text, (uint32_t)pos, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_text_english.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)pos);

	float ratio = (float)packedSize / pos;
	ASSERT_LT(ratio, 0.3f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)pos);
	ASSERT_EQ(memcmp(extracted, text, pos), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(text);
}

TEST(compression_text_chinese) {
	xpkObject xpk = xpkOpen("test_07_text_chinese.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	const char* chineseText = "测试中文压缩性能，这是重复的文本内容。";
	size_t size = 50000;
	char* text = malloc(size);
	ASSERT_NOT_NULL(text);

	size_t pos = 0;
	size_t textLen = strlen(chineseText);
	while (pos < size - textLen) {
		strcpy(text + pos, chineseText);
		pos += textLen;
	}

	xpkAppendData(xpk, text, (uint32_t)pos, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_text_chinese.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)pos);

	float ratio = (float)packedSize / pos;
	ASSERT_LT(ratio, 0.3f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)pos);
	ASSERT_EQ(memcmp(extracted, text, pos), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(text);
}

TEST(compression_mixed_text) {
	xpkObject xpk = xpkOpen("test_07_text_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 50000;
	char* text = malloc(size);
	ASSERT_NOT_NULL(text);

	size_t pos = 0;
	while (pos < size - 100) {
		strcpy(text + pos, "Hello world 你好世界 1234567890");
		pos += 28;
	}

	xpkAppendData(xpk, text, (uint32_t)pos, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_text_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)pos);

	float ratio = (float)packedSize / pos;
	ASSERT_LT(ratio, 0.35f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)pos);
	ASSERT_EQ(memcmp(extracted, text, pos), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(text);
}

TEST(compression_binary_structured) {
	xpkObject xpk = xpkOpen("test_07_binary_structured.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	typedef struct {
		uint32_t id;
		char name[32];
		float value;
		int active;
	} Record;

	size_t count = 10000;
	size_t size = count * sizeof(Record);
	Record* records = malloc(size);
	ASSERT_NOT_NULL(records);

	for (size_t i = 0; i < count; i++) {
		records[i].id = (uint32_t)i;
		sprintf(records[i].name, "Record_%08zu", i);
		records[i].value = 100.0f + (float)i * 0.01f;
		records[i].active = (i % 2) == 0 ? 1 : 0;
	}

	xpkAppendData(xpk, records, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_binary_structured.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.5f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, records, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(records);
}

TEST(compression_alternating_pattern) {
	xpkObject xpk = xpkOpen("test_07_alternating.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)(i % 2 == 0 ? 0xAA : 0x55);
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_alternating.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.05f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, data, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(data);
}

TEST(compression_incremental_pattern) {
	xpkObject xpk = xpkOpen("test_07_incremental.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)(i % 256);
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_incremental.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.15f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(extracted, data, size), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(data);
}

TEST(compression_already_compressed) {
	xpkObject xpk = xpkOpen("test_07_already_compressed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 50000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}

	uint32_t packedSize1 = xpkCompressRouter(data, (uint32_t)size, 6);
	ASSERT_GT(packedSize1, 0);

	uint8_t* compressed = malloc(packedSize1);
	ASSERT_NOT_NULL(compressed);

	xpkCompressRouterTo(data, (uint32_t)size, 6, compressed, packedSize1);

	xpkAppendData(xpk, compressed, packedSize1, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_already_compressed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize2 = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize2, 0);

	float ratio = (float)packedSize2 / packedSize1;
	ASSERT_GT(ratio, 0.95f);

	free(compressed);
	free(data);
	xpkClose(xpk);
}

TEST(compression_json_data) {
	xpkObject xpk = xpkOpen("test_07_json.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 50000;
	char* json = malloc(size);
	ASSERT_NOT_NULL(json);

	size_t pos = 0;
	strcpy(json + pos, "{\"items\":[");
	pos += 10;

	int itemCount = 0;
	while (pos < size - 20) {
		sprintf(json + pos, "{\"id\":%d,\"name\":\"Item %d\"},", itemCount, itemCount);
		pos += (uint32_t)strlen(json + pos);
		itemCount++;
	}

	strcpy(json + pos - 1, "]}");

	xpkAppendData(xpk, json, (uint32_t)pos, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_json.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)pos);

	float ratio = (float)packedSize / pos;
	ASSERT_LT(ratio, 0.4f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)pos);
	ASSERT_EQ(memcmp(extracted, json, pos), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(json);
}

TEST(compression_xml_data) {
	xpkObject xpk = xpkOpen("test_07_xml.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 50000;
	char* xml = malloc(size);
	ASSERT_NOT_NULL(xml);

	size_t pos = 0;
	strcpy(xml + pos, "<root>");
	pos += 6;

	while (pos < size - 20) {
		sprintf(xml + pos, "<item id=\"%d\"><name>Item Name</name></item>", (int)pos);
		pos += (uint32_t)strlen(xml + pos);
	}

	strcpy(xml + pos, "</root>");
	pos += 7;

	xpkAppendData(xpk, xml, (uint32_t)pos, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_xml.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)pos);

	float ratio = (float)packedSize / pos;
	ASSERT_LT(ratio, 0.35f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)pos);
	ASSERT_EQ(memcmp(extracted, xml, pos), 0);
	xpkFree(extracted);

	xpkClose(xpk);
	free(xml);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Compression Data Patterns Test\n");
	printf("=================================================\n\n");

	printf("[Simple Pattern Tests]\n");
	RUN_TEST(compression_repeated_data);
	RUN_TEST(compression_random_data);
	RUN_TEST(compression_zeroes);
	RUN_TEST(compression_ones);
	printf("\n");

	printf("[Text Data Tests]\n");
	RUN_TEST(compression_text_english);
	RUN_TEST(compression_text_chinese);
	RUN_TEST(compression_mixed_text);
	printf("\n");

	printf("[Binary Data Tests]\n");
	RUN_TEST(compression_binary_structured);
	RUN_TEST(compression_alternating_pattern);
	RUN_TEST(compression_incremental_pattern);
	printf("\n");

	printf("[Structured Data Tests]\n");
	RUN_TEST(compression_json_data);
	RUN_TEST(compression_xml_data);
	printf("\n");

	printf("[Special Cases]\n");
	RUN_TEST(compression_already_compressed);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
