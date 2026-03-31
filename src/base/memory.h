/*
	xPack 内部内存适配模块

	负责统一封装内部申请与释放接口。
*/

// 分配内部内存
static inline void* xpkAllocInternal(size_t size)
{
	return xrtMalloc(size);
}

// 释放内部内存
static inline void xpkFreeInternal(void* ptr)
{
	xrtFree(ptr);
}
