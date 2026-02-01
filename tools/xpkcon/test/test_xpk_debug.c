#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/xpack.h"

int main() {
    printf("Step 1: Testing xrtFileGetAll\n");
    
    FILE* f = fopen("test_input1.txt", "w");
    if (f) {
        fprintf(f, "Hello, World!");
        fclose(f);
    }
    
    size_t fileSize = 0;
    void* fileData = xrtFileGetAll((unsigned char*)"test_input1.txt", &fileSize);
    if (!fileData) {
        printf("xrtFileGetAll failed\n");
        return 1;
    }
    
    printf("xrtFileGetAll succeeded, size=%zu, data=%s\n", fileSize, (char*)fileData);
    free(fileData);
    
    printf("\nStep 2: Testing xpkOpen\n");
    
    xpkObject xpk = xpkOpen("test_debug.xpk", 0, 0);
    if (xpk == NULL) {
        printf("xpkOpen failed\n");
        return 1;
    }

    printf("xpkOpen succeeded\n");

    printf("\nStep 3: Testing xpkAppendFile\n");
    
    uint32_t pos = xpkAppendFile(xpk, "test_input1.txt", 7);
    if (pos == UINT32_MAX) {
        printf("xpkAppendFile failed\n");
        xpkClose(xpk);
        return 1;
    }

    printf("xpkAppendFile succeeded, pos=%u\n", pos);

    printf("\nStep 4: Testing xpkSave\n");
    
    if (xpkSave(xpk) != 0) {
        printf("xpkSave failed\n");
    } else {
        printf("xpkSave succeeded\n");
    }

    xpkClose(xpk);

    remove("test_input1.txt");
    remove("test_debug.xpk");

    return 0;
}
