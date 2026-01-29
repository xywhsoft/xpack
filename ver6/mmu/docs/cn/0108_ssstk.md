## SSSTK（Struct Static Stack）模块：

分类：数据结构（用于管理特性形态的数据）

SSSTK 为静态的结构体栈结构数据管理器，适用于管理数据形态为栈，栈数据单元为结构体，栈最大深度有限的场景。

SSSTK 在初始化时指定栈的最大深度，会一次性申请足够容纳所有成员的内存，当栈达到最大深度时，就无法再向里面压入数据了。

请注意，SSSTK 是建立在内存堆上的栈数据结构，性能并不能和建立在 SRAM 中的栈相提并论，仅用于“栈”这类抽象数据结构使用，SSSTK 可以管理固定长度的结构体，非常适合需要对数据进行分层级管理的场合，例如编译器语句结构深度的管理，括号匹配等应用场景。

### SSSTK_Struct [数据结构]
``` c
typedef struct {
	char* Memory;						// 栈数据内存指针
	unsigned int ItemLength;			// 栈成员占用内存长度
	unsigned int MaxCount;				// 栈最大可以容纳多少成员（栈深度）
	unsigned int Count;					// 栈中存在多少成员（栈顶位置）
} SSSTK_Struct, *SSSTK_Object;
```
SSSTK_Struct 是 SSSTK 对象的基本结构体数据，可以通过 SSSTK_Create 函数创建一个指向 SSSTK_Struct 结构体的指针（也就是 SSSTK_Object）。

SSSTK 必须使用已经初始化过的 SSSTK_Struct 数据结构（使用 SSSTK_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### SSSTK_Object [对象指针]
SSSTK_Object 是 SSSTK_Struct 数据结构的指针，通过 SSSTK_Create 创建的 SSSTK 对象会返回 SSSTK_Object 对象指针。

### SSSTK_Create - 创建栈
声明：SSSTK_Object SSSTK_Create(unsigned int iMaxCount, unsigned int iItemLength);
创建 SSSTK 对象并初始化数据，创建成功时返回一个 SSSTK_Object 对象指针，创建失败返回 NULL。
参数 iMaxCount 用于指定栈最大能够容纳多少个数据。
参数 iItemLength 用于指定管理器内，结构体单元的数据长度，通常使用 sizeof 获取对应结构体的内存占用长度。
由 SSSTK_Create 创建的对象，不再使用时应调用 SSSTK_Destroy 函数将 SSSTK 对象销毁，否则会产生内存泄漏。

### SSSTK_Destroy - 销毁栈
声明：void SSSTK_Destroy(SSSTK_Object objSTK);
销毁由 SSSTK_Create 函数创建的 SSSTK 对象。

### SSSTK_Push - 压栈
声明：void* SSSTK_Push(SSSTK_Object objSTK);
将数据压栈，返回这个数据的指针（因为 SSSTK 是附带内存管理功能的，压栈的内存来自 SSSTK 内部，因此压栈后返回数据指针，程序通过指针读写这块内存实现数据入栈），压栈失败返回 NULL。

### SSSTK_PushData - 压栈(复制数据)
声明：unsigned int SSSTK_PushData(SSSTK_Object objSTK, void* pData);
将数据压栈，返回栈深度。
这个函数和 SSSTK_Push 的区别在于，它会将 pData 指向的数据，复制到新的栈顶内存中（复制的数据长度为 SSSTK_Create 时 iItemLength 参数指定的长度）。

### SSSTK_Pop - 出栈
声明：void* SSSTK_Pop(SSSTK_Object objSTK);
将栈顶元素弹出，并返回这个元素的指针。

### SSSTK_Top - 获取栈顶对象
声明：void* SSSTK_Top(SSSTK_Object objSTK);
获取栈顶元素的指针。

### SSSTK_GetPos - 获取任意位置对象
声明：void* SSSTK_GetPos(SSSTK_Object objSTK, unsigned int iPos);
获取 iPos 位置元素的指针，位置从 1 开始。

### SSSTK_GetPos_Unsafe - 获取任意位置对象
声明：void* SSSTK_GetPos_Unsafe(SSSTK_Object objSTK, unsigned int iPos);
与 SSSTK_GetPos 相同，SSSTK_GetPos_Unsafe 不会进行范围安全检查。