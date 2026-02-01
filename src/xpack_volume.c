/*
 * xPack Ver7 - 分卷功能实现
 * 
 * 包含：分卷管理器、跨卷读写、卷文件操作
 */

#include "xpack_internal.h"
#include <string.h>
#include <stdlib.h>

// ============================================================================
// 卷文件名生成
// ============================================================================

void xpkVolumeGetName(xpkObject xpk, int index, char* outName, size_t nameSize) {
    if (!xpk || !outName || nameSize == 0) return;
    
    if (index == 0) {
        strncpy(outName, xpk->volume.basePath, nameSize);
        outName[nameSize - 1] = '\0';
        return;
    }
    
    size_t basePathLen = strlen(xpk->volume.basePath);
    if (basePathLen < 5) {
        snprintf(outName, nameSize, "%s.v%02d", xpk->volume.basePath, index);
        return;
    }
    
    char ext[4] = {0};
    strncpy(ext, xpk->volume.basePath + basePathLen - 3, 3);
    
    char basePathNoExt[256];
    strncpy(basePathNoExt, xpk->volume.basePath, basePathLen - 4);
    basePathNoExt[basePathLen - 4] = '\0';
    
    snprintf(outName, nameSize, "%s.%c%02d", basePathNoExt, ext[0], index);
}

// ============================================================================
// 分卷管理器初始化
// ============================================================================

int xpkVolumeInit(xpkObject xpk) {
    if (!xpk) return -1;
    
    memset(&xpk->volume, 0, sizeof(xpkVolume));
    xpk->volume.enabled = 0;
    xpk->volume.volumeSize = 0;
    xpk->volume.splitMode = 0;
    xpk->volume.currentVolume = 0;
    xpk->volume.currentOffset = 0;
    xpk->volume.totalSize = 0;
    
    for (int i = 0; i < XPK_MAX_VOLUMES; i++) {
        xpk->volume.volumes[i] = NULL;
    }
    
    return 0;
}

// ============================================================================
// 打开指定卷
// ============================================================================

int xpkVolumeOpen(xpkObject xpk, int index) {
    if (!xpk || index < 0 || index >= XPK_MAX_VOLUMES) return -1;
    
    if (xpk->volume.volumeOpen[index]) {
        return 0;
    }
    
    char volPath[512];
    xpkVolumeGetName(xpk, index, volPath, sizeof(volPath));
    
    xpk->volume.volumes[index] = xrtOpen((str)volPath, xpk->readonly, XRT_CP_BINARY);
    if (!xpk->volume.volumes[index]) {
        xpkSetError(1, "Failed to open volume file");
        return -1;
    }
    
    xpk->volume.volumeOpen[index] = 1;
    
    return 0;
}

// ============================================================================
// 关闭所有卷
// ============================================================================

int xpkVolumeCloseAll(xpkObject xpk) {
    if (!xpk) return -1;
    
    for (int i = 0; i < XPK_MAX_VOLUMES; i++) {
        if (xpk->volume.volumeOpen[i]) {
            xrtClose(xpk->volume.volumes[i]);
            xpk->volume.volumeOpen[i] = 0;
            xpk->volume.volumes[i] = NULL;
        }
    }
    
    // 只在多卷模式下设置 file 为 NULL
    if (xpk->volume.enabled) {
        xpk->file = NULL;
    }
    return 0;
}

// ============================================================================
// 创建新卷
// ============================================================================

int xpkVolumeCreateNext(xpkObject xpk) {
    if (!xpk || !xpk->volume.enabled) return -1;
    
    int nextIndex = xpk->volume.currentVolume + 1;
    if (nextIndex >= XPK_MAX_VOLUMES) {
        xpkSetError(12, "Maximum volume count exceeded");
        return -1;
    }
    
    char volPath[512];
    xpkVolumeGetName(xpk, nextIndex, volPath, sizeof(volPath));
    
    xpk->volume.volumes[nextIndex] = xrtOpen((str)volPath, 0, XRT_CP_BINARY);
    if (!xpk->volume.volumes[nextIndex]) {
        xpkSetError(1, "Failed to create volume");
        return -1;
    }
    
    xpkVolumeInfo volInfo;
    volInfo.volumeCount = nextIndex + 1;
    volInfo.volumeIndex = nextIndex;
    
    xpkHead headCopy = xpk->head;
    headCopy.flag.volumeMode = 1;
    headCopy.flag.splitMode = xpk->volume.splitMode;
    headCopy.headExtSize = sizeof(xpkVolumeInfo);
    
    if (xrtPut(xpk->volume.volumes[nextIndex], &headCopy, sizeof(xpkHead)) != (int)sizeof(xpkHead)) {
        xrtClose(xpk->volume.volumes[nextIndex]);
        xpk->volume.volumes[nextIndex] = NULL;
        xpkSetError(2, "Failed to write volume header");
        return -1;
    }
    
    if (xrtPut(xpk->volume.volumes[nextIndex], &volInfo, sizeof(xpkVolumeInfo)) != (int)sizeof(xpkVolumeInfo)) {
        xrtClose(xpk->volume.volumes[nextIndex]);
        xpk->volume.volumes[nextIndex] = NULL;
        xpkSetError(2, "Failed to write volume info");
        return -1;
    }
    
    xpk->volume.volumeOpen[nextIndex] = 1;
    xpk->volume.currentVolume = nextIndex;
    xpk->volume.currentOffset = XPK_VOL_HEADER_SIZE;
    xpk->volume.volumeOffsets[nextIndex] = xpk->volume.totalSize;
    
    return 0;
}

// ============================================================================
// 检查当前卷容量
// ============================================================================

int xpkVolumeCheckCapacity(xpkObject xpk, uint32_t dataSize) {
    if (!xpk || !xpk->volume.enabled) return 1;
    if (xpk->volume.volumeSize == 0) return 1;
    
    uint32_t remaining = xpk->volume.volumeSize - xpk->volume.currentOffset;
    return (dataSize <= remaining) ? 1 : 0;
}

// ============================================================================
// 写入数据到卷
// ============================================================================

int xpkVolumeWriteData(xpkObject xpk, const void* data, uint32_t size) {
    if (!xpk || !data || size == 0) return -1;
    
    if (!xpk->volume.enabled) {
        if (!xpk->file) return -1;
        if (xrtPut(xpk->file, (void*)data, size) != (int)size) {
            xpkSetError(2, "Failed to write data");
            return -1;
        }
        return 0;
    }
    
    uint32_t bytesWritten = 0;
    const uint8_t* srcData = (const uint8_t*)data;
    
    while (bytesWritten < size) {
        xfile volFile = xpk->volume.volumes[xpk->volume.currentVolume];
        if (!volFile) {
            xpkSetError(13, "Volume file not available");
            return -1;
        }
        
        uint32_t remaining = size - bytesWritten;
        uint32_t volRemaining;
        
        if (xpk->volume.volumeSize == 0) {
            volRemaining = remaining;
        } else {
            volRemaining = xpk->volume.volumeSize - xpk->volume.currentOffset;
        }
        
        uint32_t writeSize = (remaining < volRemaining) ? remaining : volRemaining;
        
        xrtSeek(volFile, xpk->volume.currentOffset, XRT_SEEK_SET);
        if (xrtPut(volFile, (void*)(srcData + bytesWritten), writeSize) != (int)writeSize) {
            xpkSetError(2, "Failed to write volume data");
            return -1;
        }
        
        bytesWritten += writeSize;
        xpk->volume.currentOffset += writeSize;
        xpk->volume.totalSize += writeSize;
        
        if (xpk->volume.volumeSize > 0 && xpk->volume.currentOffset >= xpk->volume.volumeSize) {
            if (xpkVolumeCreateNext(xpk) != 0) {
                return -1;
            }
        }
    }
    
    return 0;
}

// ============================================================================
// 从卷读取数据
// ============================================================================

void* xpkVolumeReadData(xpkObject xpk, uint32_t globalOffset, uint32_t size, uint32_t* outSize) {
    if (!xpk || outSize == 0) return NULL;
    
    *outSize = 0;
    
    if (!xpk->volume.enabled) {
        if (!xpk->file) {
            xpkSetError(13, "File not available");
            return NULL;
        }
        
        xrtSeek(xpk->file, globalOffset, XRT_SEEK_SET);
        
        uint8_t* buffer = (uint8_t*)malloc(size);
        if (!buffer) {
            xpkSetError(3, "Failed to allocate read buffer");
            return NULL;
        }
        
        size_t bytesRead;
        void* data = xrtGet(xpk->file, size, &bytesRead);
        if (!data || bytesRead != size) {
            free(buffer);
            xpkSetError(2, "Failed to read data");
            return NULL;
        }
        
        memcpy(buffer, data, size);
        *outSize = size;
        return buffer;
    }
    
    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) {
        xpkSetError(3, "Failed to allocate read buffer");
        return NULL;
    }
    
    uint32_t bytesToRead = size;
    uint32_t bytesRead = 0;
    uint32_t readOffset = globalOffset;
    
    while (bytesRead < size) {
        int volIndex = xpkVolumeFromOffset(xpk, readOffset);
        if (volIndex < 0 || volIndex >= xpk->volume.currentVolume + 1) {
            free(buffer);
            xpkSetError(14, "Volume data incomplete");
            return NULL;
        }
        
        xfile volFile = xpk->volume.volumes[volIndex];
        if (!volFile) {
            if (xpkVolumeOpen(xpk, volIndex) != 0) {
                free(buffer);
                return NULL;
            }
            volFile = xpk->volume.volumes[volIndex];
        }
        
        uint32_t volOffset = readOffset - xpk->volume.volumeOffsets[volIndex];
        uint32_t volEnd = (volIndex < xpk->volume.currentVolume) 
            ? xpk->volume.volumeOffsets[volIndex + 1]
            : xpk->volume.totalSize;
        uint32_t volRemaining = volEnd - readOffset;
        
        uint32_t readSize = (bytesToRead < volRemaining) ? bytesToRead : volRemaining;
        
        xrtSeek(volFile, volOffset, XRT_SEEK_SET);
        size_t actualRead;
        void* data = xrtGet(volFile, readSize, &actualRead);
        if (!data || actualRead != readSize) {
            free(buffer);
            xpkSetError(2, "Failed to read volume data");
            return NULL;
        }
        
        memcpy(buffer + bytesRead, data, readSize);
        bytesRead += readSize;
        bytesToRead -= readSize;
        readOffset += readSize;
    }
    
    *outSize = size;
    return buffer;
}

// ============================================================================
// 根据全局偏移计算所在的卷
// ============================================================================

int xpkVolumeFromOffset(xpkObject xpk, uint32_t globalOffset) {
    if (!xpk || !xpk->volume.enabled) return 0;
    
    for (int i = 0; i <= xpk->volume.currentVolume; i++) {
        uint32_t nextOffset = (i < xpk->volume.currentVolume) 
            ? xpk->volume.volumeOffsets[i + 1]
            : xpk->volume.totalSize;
        if (globalOffset < nextOffset) {
            return i;
        }
    }
    
    return xpk->volume.currentVolume;
}

// ============================================================================
// 获取卷文件路径
// ============================================================================

const char* xpkVolumeGetPath(xpkObject xpk, int index) {
    if (!xpk || index < 0 || index > xpk->volume.currentVolume) {
        return NULL;
    }
    
    static char pathBuffer[512];
    xpkVolumeGetName(xpk, index, pathBuffer, sizeof(pathBuffer));
    return pathBuffer;
}
