/*
 * xxhash compatibility layer for ZSTD
 * 
 * This is a minimal xxhash API wrapper using xrt library's xrtHash64 (rapidhash).
 * Only provides the subset of xxhash API required by ZSTD core compression.
 * 
 * Copyright (c) 2025 xPack Project
 * BSD 2-Clause License
 */

#ifndef XXHASH_COMPAT_H
#define XXHASH_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Include xrt for xrtHash64 */
#include <xrt/xrt.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Type definitions
 * ============================================================================ */

typedef uint64_t XXH64_hash_t;
typedef int XXH_errorcode;

#define XXH_OK    0
#define XXH_ERROR 1

/* XXH64 state structure - minimal for ZSTD compatibility */
typedef struct {
    uint64_t total_len;
    uint64_t seed;
    uint8_t  buffer[32];
    uint32_t bufsize;
} XXH64_state_t;

/* ============================================================================
 * xxhash API implementation using xrt
 * ============================================================================ */

/* One-shot hash - used by LDM and dictID */
static inline XXH64_hash_t XXH64(const void* input, size_t length, XXH64_hash_t seed) {
    /* xrtHash64_WithSeed uses rapidhash internally */
    return xrtHash64_WithSeed((ptr)input, length, seed);
}

/* Streaming API - stub implementation (checksum disabled in xpack) */
static inline XXH_errorcode XXH64_reset(XXH64_state_t* statePtr, XXH64_hash_t seed) {
    if (statePtr == NULL) return XXH_ERROR;
    memset(statePtr, 0, sizeof(*statePtr));
    statePtr->seed = seed;
    return XXH_OK;
}

static inline XXH_errorcode XXH64_update(XXH64_state_t* statePtr, const void* input, size_t length) {
    if (statePtr == NULL) return XXH_ERROR;
    /* Accumulate data for final hash */
    statePtr->total_len += length;
    if (length <= sizeof(statePtr->buffer)) {
        memcpy(statePtr->buffer, input, length);
        statePtr->bufsize = (uint32_t)length;
    }
    return XXH_OK;
}

static inline XXH64_hash_t XXH64_digest(const XXH64_state_t* statePtr) {
    if (statePtr == NULL) return 0;
    /* Return hash of accumulated data using xrt */
    return xrtHash64_WithSeed((ptr)statePtr->buffer, statePtr->bufsize, statePtr->seed);
}

/* State management - not needed for xpack */
static inline XXH64_state_t* XXH64_createState(void) {
    return NULL;
}

static inline XXH_errorcode XXH64_freeState(XXH64_state_t* statePtr) {
    (void)statePtr;
    return XXH_OK;
}

static inline void XXH64_copyState(XXH64_state_t* dst, const XXH64_state_t* src) {
    if (dst && src) memcpy(dst, src, sizeof(*dst));
}

#ifdef __cplusplus
}
#endif

#endif /* XXHASH_COMPAT_H */
