## PDSTK（Point Dynamic Stack）模块：

分类：数据结构（用于管理特性形态的数据）

PDSTK 为动态的指针栈结构数据管理器，适用于管理数据形态为栈，栈数据单元为指针，栈最大深度可自动扩容的场景。

PDSTK 在初始化时无需指定最大深度，当栈达到已申请内存所能容纳的最大深度时，会自动扩容，以容纳更多数据入栈。

请注意，PDSTK 是建立在内存堆上的栈数据结构，性能并不能和建立在 SRAM 中的栈相提并论，仅用于“栈”这类抽象数据结构使用，PDSTK 可以管理固定长度的结构体，非常适合需要对数据进行分层级管理的场合，例如编译器语句结构深度的管理，括号匹配等应用场景。

### PDSTK_Struct [数据结构]
``` c
typedef struct {
	unsigned int Count;							// 栈中存在多少成员（栈顶位置）
	PAMM_Struct MMU;							// MMU 管理器
	MMU_OnErrorProc OnError;					// 错误处理回调函数
} PDSTK_Struct, *PDSTK_Object;
```
PDSTK_Struct 是 PDSTK 对象的基本结构体数据，可以通过 PDSTK_Create 函数创建一个指向 PDSTK_Struct 结构体的指针（也就是 PDSTK_Object）。

PDSTK 必须使用已经初始化过的 PDSTK_Struct 数据结构（使用 PDSTK_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### PDSTK_Object [对象指针]
PDSTK_Object 是 PDSTK_Struct 数据结构的指针，通过 PDSTK_Create 创建的 PDSTK 对象会返回 PDSTK_Object 对象指针。

### PDSTK_Create - 创建栈
声明：PDSTK_Object PDSTK_Create();
创建 PDSTK 对象并初始化数据，创建成功时返回一个 PDSTK_Object 对象指针，创建失败返回 NULL。
由 PDSTK_Create 创建的对象，不再使用时应调用 PDSTK_Destroy 函数将 PDSTK 对象销毁，否则会产生内存泄漏。

### PDSTK_Destroy - 销毁栈
声明：void PDSTK_Destroy(PDSTK_Object objSTK);
销毁由 PDSTK_Create 函数创建的 PDSTK 对象。

### PDSTK_Init - 初始化栈
声明：void PDSTK_Init(PDSTK_Object objSTK);
这个函数和 PDSTK_Create 功能类似，它在不申请 PDSTK 对象内存的情况下，对已存在的 PDSTK_Struct 数据进行初始化，将 PDSTK_Struct 嵌入到其他数据结构中的情况下，应使用此函数进行数据初始化。
嵌入到其他数据结构的 PDSTK_Struct 不再使用时应调用 PDSTK_Unit 函数释放对象中的数据，否则会产生内存泄漏。

### PDSTK_Unit - 释放栈
声明：void PDSTK_Unit(PDSTK_Object objSTK);
释放 PDSTK 对象内的数据，它会将管理器成员数量清零，并释放管理器占用的内存。

### PDSTK_Push - 压栈
声明：unsigned int PDSTK_Push(PDSTK_Object objSTK, void* ptr);
将 ptr 压栈，返回栈深度。

### PDSTK_Pop - 出栈
声明：void* PDSTK_Pop(PDSTK_Object objSTK);
将栈顶元素弹出，并返回这个元素的指针。

### PDSTK_Top - 获取栈顶指针
声明：void* PDSTK_Top(PDSTK_Object objSTK);
获取栈顶元素的指针。

### PDSTK_GetPos - 获取任意位置指针
声明：void* PDSTK_GetPos(PDSTK_Object objSTK, unsigned int iPos);
获取 iPos 位置元素的指针，位置从 1 开始。

### PDSTK_GetPos_Unsafe - 获取任意位置指针
声明：void* PDSTK_GetPos_Unsafe(PDSTK_Object objSTK, unsigned int iPos);
与 PDSTK_GetPos 相同，PDSTK_GetPos_Unsafe 不会进行范围安全检查。