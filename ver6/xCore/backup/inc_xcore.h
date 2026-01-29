


// GC 申请内存
XXAPI ptr xCore_GC_malloc(size_t size)
{
	return tgc_alloc(xCore.objGC, size);
}

// GC 申请类内存
XXAPI ptr xCore_GC_calloc(size_t count, size_t size)
{
	return tgc_calloc(xCore.objGC, count, size);
}

// 重新申请 GC 内存
XXAPI ptr xCore_GC_realloc(ptr pmem, size_t size)
{
	return tgc_realloc(xCore.objGC, pmem, size);
}

// 释放 GC 内存
XXAPI void xCore_GC_free(ptr pmem)
{
	tgc_free(xCore.objGC, pmem);
}

// 暂停 GC 自动执行
XXAPI void xCore_GC_Pause(void)
{
	tgc_pause(xCore.objGC);
}

// 恢复 GC 自动执行
XXAPI void xCore_GC_Resume(void)
{
	tgc_pause(xCore.objGC);
}

// 执行一次 GC
XXAPI void xCore_GC_Run(void)
{
	tgc_pause(xCore.objGC);
}

// 添加一块内存到 GC
XXAPI ptr xCore_GC_Add(ptr pmem, ulong size, int flags, void(*dtor)(ptr))
{
	return tgc_add(xCore.objGC, pmem, size, flags, dtor);
}

// 从 GC 中移除一块内存
XXAPI void xCore_GC_Remove(ptr pmem)
{
	tgc_rem(xCore.objGC, pmem);
}

// 获取 GC 栈顶
XXAPI ptr xCore_GC_GetStackTop()
{
	tgc_t* gc = xCore.objGC;
	return gc->bottom;
}

// 设置栈顶
XXAPI void xCore_GC_SetStackTop(ptr pStack)
{
	tgc_t* gc = xCore.objGC;
	gc->bottom = pStack;
}

// 释放内存（ 释放 malloc 申请的内存，会先判断是否为 nullstring ）
XXAPI void xCore_free(ptr pmem)
{
	if ( pmem && (pmem != xCore.nullstring) ) { free(pmem); }
}

// 开启GC
XXAPI ptr xCore_GC_Start(ptr pStack)
{
	// 初始化GC
	if ( xCore.objGC ) { return xCore.objGC; }
	int GetStackPtr;
	if ( pStack == 0 ) { pStack = &GetStackPtr; }
	xCore.objGC = malloc(sizeof(tgc_t));
	if ( xCore.objGC == 0 ) { return 0; }
	tgc_start(xCore.objGC, pStack);
	// 替换 GC 函数
	xCore.malloc = xCore_GC_malloc;
	xCore.calloc = xCore_GC_calloc;
	xCore.realloc = xCore_GC_realloc;
	xCore.free = xCore_GC_free;
	return xCore.objGC;
}

// 关闭GC
XXAPI void xCore_GC_Stop()
{
	// 替换 GC 函数
	xCore.malloc = (ptr)malloc;
	xCore.calloc = (ptr)calloc;
	xCore.realloc = (ptr)realloc;
	xCore.free = xCore_free;
	if ( xCore.objGC ) {
		tgc_stop(xCore.objGC);
		free(xCore.objGC);
		xCore.objGC = 0;
	}
}


