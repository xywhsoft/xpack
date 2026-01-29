## PSSTK（Point Static Stack）模块：

分类：数据结构（用于管理特性形态的数据）

PSSTK 为静态的指针栈结构数据管理器，适用于管理数据形态为栈，栈数据单元为指针，栈最大深度有限的场景。

PSSTK 在初始化时指定栈的最大深度，会一次性申请足够容纳所有成员的内存，当栈达到最大深度时，就无法再向里面压入数据了。

请注意，PSSTK 是建立在内存堆上的栈数据结构，性能并不能和建立在 SRAM 中的栈相提并论，仅用于“栈”这类抽象数据结构使用，PSSTK 可以管理固定长度的结构体，非常适合需要对数据进行分层级管理的场合，例如编译器语句结构深度的管理，括号匹配等应用场景。

### PSSTK_Struct [数据结构]
``` c
typedef struct {
	void** Memory;						// 栈数据内存指针
	unsigned int MaxCount;				// 栈最大可以容纳多少成员（栈深度）
	unsigned int Count;					// 栈中存在多少成员（栈顶位置）
} PSSTK_Struct, *PSSTK_Object;
```
PSSTK_Struct 是 PSSTK 对象的基本结构体数据，可以通过 PSSTK_Create 函数创建一个指向 PSSTK_Struct 结构体的指针（也就是 PSSTK_Object）。

PSSTK 必须使用已经初始化过的 PSSTK_Struct 数据结构（使用 PSSTK_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### PSSTK_Object [对象指针]
PSSTK_Object 是 PSSTK_Struct 数据结构的指针，通过 PSSTK_Create 创建的 PSSTK 对象会返回 PSSTK_Object 对象指针。

### PSSTK_Create - 创建栈
声明：PSSTK_Object PSSTK_Create(unsigned int iMaxCount);
创建 PSSTK 对象并初始化数据，创建成功时返回一个 PSSTK_Object 对象指针，创建失败返回 NULL。
参数 iMaxCount 用于指定栈最大能够容纳多少个指针。
由 PSSTK_Create 创建的对象，不再使用时应调用 PSSTK_Destroy 函数将 PSSTK 对象销毁，否则会产生内存泄漏。

### PSSTK_Destroy - 销毁栈
声明：void PSSTK_Destroy(PSSTK_Object objSTK);
销毁由 PSSTK_Create 函数创建的 PSSTK 对象。

### PSSTK_Push - 压栈
声明：unsigned int PSSTK_Push(PSSTK_Object objSTK, void* ptr);
将 ptr 压栈，返回栈深度。

### PSSTK_Pop - 出栈
声明：void* PSSTK_Pop(PSSTK_Object objSTK);
将栈顶元素弹出，并返回这个元素的指针。

### PSSTK_Top - 获取栈顶指针
声明：void* PSSTK_Top(PSSTK_Object objSTK);
获取栈顶元素的指针。

### PSSTK_GetPos - 获取任意位置指针
声明：void* PSSTK_GetPos(PSSTK_Object objSTK, unsigned int iPos);
获取 iPos 位置元素的指针，位置从 1 开始。

### PSSTK_GetPos_Unsafe - 获取任意位置指针
声明：void* PSSTK_GetPos_Unsafe(PSSTK_Object objSTK, unsigned int iPos);
与 PSSTK_GetPos 相同，PSSTK_GetPos_Unsafe 不会进行范围安全检查。