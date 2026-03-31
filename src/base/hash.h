/*
	xPack 内部哈希适配模块

	负责统一封装数据哈希接口。
*/

// 计算内部 32 位哈希值
static inline uint32_t xpkHash32Internal(const void* data, uint64_t size)
{
	return xrtHash32((ptr)data, (size_t)size);
}
