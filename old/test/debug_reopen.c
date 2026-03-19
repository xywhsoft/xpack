#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpack.h"

int main(void) {
    xrtInit();
    
    xrtFileDelete("test_debug.xpk");
    
    xpkObject xpk = xpkOpen("test_debug", 0, 0);
    xpkTypeSet(xpk, XPK_TYPE_WIN32);
    
    uint8_t writeData[1024];
    for (int i = 0; i < 1024; i++) writeData[i] = (uint8_t)(i & 0xFF);
    
    printf("Before RawWrite: head.dataOffset = %llu\n", (unsigned long long)xpkGetHead(xpk)->dataOffset);
    printf("XPK_HEAD_SIZE = %d\n", XPK_HEAD_SIZE);
    
    // 写入数据到 XPK_HEAD_SIZE 位置
    if (xpkRawWrite(xpk, XPK_HEAD_SIZE, sizeof(writeData), writeData) != 0) {
        printf("Write failed\n");
        return 1;
    }
    printf("RawWrite to offset %d succeeded\n", XPK_HEAD_SIZE);
    
    printf("After RawWrite: head.dataOffset = %llu\n", (unsigned long long)xpkGetHead(xpk)->dataOffset);
    
    // 保存
    printf("Calling xpkSave...\n");
    if (xpkSave(xpk) != 0) {
        printf("Save failed: %s\n", xpkLastErrorMsg());
        return 1;
    }
    printf("After Save: head.dataOffset = %llu, metaCompSize = %u\n", 
           (unsigned long long)xpkGetHead(xpk)->dataOffset,
           xpkGetHead(xpk)->metaCompSize);
    
    xpkClose(xpk);
    
    // 重新打开
    xpk = xpkOpen("test_debug", 0, 1);
    if (!xpk) {
        printf("Reopen failed: %s\n", xpkLastErrorMsg());
        return 1;
    }
    
    printf("After reopen: head.dataOffset = %llu, metaCompSize = %u\n",
           (unsigned long long)xpkGetHead(xpk)->dataOffset,
           xpkGetHead(xpk)->metaCompSize);
    
    uint8_t readData[1024];
    if (xpkRawRead(xpk, XPK_HEAD_SIZE, sizeof(readData), readData) != 0) {
        printf("Read failed\n");
        return 1;
    }
    
    // 检查数据
    int mismatch = 0;
    for (int i = 0; i < 1024; i++) {
        if (readData[i] != (uint8_t)(i & 0xFF)) {
            if (mismatch < 3) {
                printf("Mismatch at %d: expected 0x%02X, got 0x%02X\n", i, (uint8_t)(i & 0xFF), readData[i]);
            }
            mismatch++;
        }
    }
    
    if (mismatch > 0) {
        printf("Total mismatches: %d / 1024\n", mismatch);
        printf("\n问题分析:\n");
        printf("  RawWrite 写入位置: XPK_HEAD_SIZE (%d)\n", XPK_HEAD_SIZE);
        printf("  xpkSave 写入 Meta 位置: head.dataOffset (%llu)\n", (unsigned long long)xpkGetHead(xpk)->dataOffset);
        printf("  => Meta 数据覆盖了 RawWrite 写入的数据!\n");
    } else {
        printf("Data matches!\n");
    }
    
    xpkClose(xpk);
    xrtFileDelete("test_debug.xpk");
    
    return mismatch > 0 ? 1 : 0;
}
