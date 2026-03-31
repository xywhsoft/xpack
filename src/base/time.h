/*
	xPack 内部时间适配模块

	负责统一封装当前时间获取接口。
*/

// 获取当前内部时间
static inline xtime xpkNowInternal(void)
{
	return xrtNow();
}
