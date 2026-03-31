static inline void* xpkAllocInternal(size_t size)
{
	return xrtMalloc(size);
}

static inline void xpkFreeInternal(void* ptr)
{
	xrtFree(ptr);
}
