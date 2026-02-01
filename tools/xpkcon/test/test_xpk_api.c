#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/xpack.h"

int main() {
    xpkObject xpk = xpkOpen("test_api.xpk", 0, 0);
    if (xpk == NULL) {
        printf("xpkOpen failed\n");
        return 1;
    }

    printf("xpkOpen succeeded\n");

    FILE* f = fopen("test_input1.txt", "w");
    if (f) {
        fprintf(f, "Hello, World!");
        fclose(f);
    }

    uint32_t pos = xpkAppendFile(xpk, "test_input1.txt", 7);
    if (pos == UINT32_MAX) {
        printf("xpkAppendFile failed\n");
        xpkClose(xpk);
        return 1;
    }

    printf("xpkAppendFile succeeded, pos=%u\n", pos);

    if (xpkSave(xpk) != 0) {
        printf("xpkSave failed\n");
    } else {
        printf("xpkSave succeeded\n");
    }

    xpkClose(xpk);

    remove("test_input1.txt");
    remove("test_api.xpk");

    return 0;
}
