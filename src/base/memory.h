#ifndef XPK_BASE_MEMORY_H
#define XPK_BASE_MEMORY_H

static inline void* xpkAllocInternal(size_t size)
{
	return xrtMalloc(size);
}

static inline void xpkFreeInternal(void* ptr)
{
	xrtFree(ptr);
}

#endif
