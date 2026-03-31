static inline uint32_t xpkHash32Internal(const void* data, uint64_t size)
{
	return xrtHash32((ptr)data, (size_t)size);
}
