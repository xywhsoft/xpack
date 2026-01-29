## SDSTK（Struct Dynamic Stack）模块：

分类：数据结构（用于管理特性形态的数据）

SDSTK 为动态的结构体栈结构数据管理器，适用于管理数据形态为栈，栈数据单元为结构体，栈最大深度可自动扩容的场景。

SDSTK 在初始化时无需指定最大深度，当栈达到已申请内存所能容纳的最大深度时，会自动扩容，以容纳更多数据入栈。

请注意，SDSTK 是建立在内存堆上的栈数据结构，性能并不能和建立在 SRAM 中的栈相提并论，仅用于“栈”这类抽象数据结构使用，SDSTK 可以管理固定长度的结构体，非常适合需要对数据进行分层级管理的场合，例如编译器语句结构深度的管理，括号匹配等应用场景。

### SDSTK_Struct [数据结构]
``` c
typedef struct {
	unsigned int ItemLength;					// 栈成员占用内存长度
	unsigned int Count;							// 栈中存在多少成员（栈顶位置）
	PAMM_Struct MMU;							// MMU 管理器
	MMU_OnErrorProc OnError;					// 错误处理回调函数
} SDSTK_Struct, *SDSTK_Object;
```
SDSTK_Struct 是 SDSTK 对象的基本结构体数据，可以通过 SDSTK_Create 函数创建一个指向 SDSTK_Struct 结构体的指针（也就是 SDSTK_Object）。

SDSTK 必须使用已经初始化过的 SDSTK_Struct 数据结构（使用 SDSTK_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### SDSTK_Object [对象指针]
SDSTK_Object 是 SDSTK_Struct 数据结构的指针，通过 SDSTK_Create 创建的 SDSTK 对象会返回 SDSTK_Object 对象指针。

### SDSTK_Create - 创建栈
声明：SDSTK_Object SDSTK_Create(unsigned int iItemLength);
创建 SDSTK 对象并初始化数据，创建成功时返回一个 SDSTK_Object 对象指针，创建失败返回 NULL。
参数 iItemLength 用于指定管理器内，结构体单元的数据长度，通常使用 sizeof 获取对应结构体的内存占用长度。
由 SDSTK_Create 创建的对象，不再使用时应调用 SDSTK_Destroy 函数将 SDSTK 对象销毁，否则会产生内存泄漏。

### SDSTK_Destroy - 销毁栈
声明：void SDSTK_Destroy(SDSTK_Object objSTK);
销毁由 SDSTK_Create 函数创建的 SDSTK 对象。

### SDSTK_Init - 初始化栈
声明：void SDSTK_Init(SDSTK_Object objSTK, unsigned int iItemLength);
这个函数和 SDSTK_Create 功能类似，它在不申请 SDSTK 对象内存的情况下，对已存在的 SDSTK_Struct 数据进行初始化，将 SDSTK_Struct 嵌入到其他数据结构中的情况下，应使用此函数进行数据初始化。
嵌入到其他数据结构的 SDSTK_Struct 不再使用时应调用 SDSTK_Unit 函数释放对象中的数据，否则会产生内存泄漏。

### SDSTK_Unit - 释放栈
声明：void SDSTK_Unit(SDSTK_Object objSTK);
释放 SDSTK 对象内的数据，它会将管理器成员数量清零，并释放管理器占用的内存。

### SDSTK_Push - 压栈
声明：void* SDSTK_Push(SDSTK_Object objSTK);
将数据压栈，返回这个数据的指针（因为 SDSTK 是附带内存管理功能的，压栈的内存来自 SDSTK 内部，因此压栈后返回数据指针，程序通过指针读写这块内存实现数据入栈），压栈失败返回 NULL。

### SDSTK_PushData - 压栈(复制数据)
声明：unsigned int SDSTK_PushData(SDSTK_Object objSTK, void* pData);
将数据压栈，返回栈深度。
这个函数和 SDSTK_Push 的区别在于，它会将 pData 指向的数据，复制到新的栈顶内存中（复制的数据长度为 SDSTK_Create 时 iItemLength 参数指定的长度）。

### SDSTK_Pop - 出栈
声明：void* SDSTK_Pop(SDSTK_Object objSTK);
将栈顶元素弹出，并返回这个元素的指针。

### SDSTK_Top - 获取栈顶对象
声明：void* SDSTK_Top(SDSTK_Object objSTK);
获取栈顶元素的指针。

### SDSTK_GetPos - 获取任意位置对象
声明：void* SDSTK_GetPos(SDSTK_Object objSTK, unsigned int iPos);
获取 iPos 位置元素的指针，位置从 1 开始。

### SDSTK_GetPos_Unsafe - 获取任意位置对象
声明：void* SDSTK_GetPos_Unsafe(SDSTK_Object objSTK, unsigned int iPos);
与 SDSTK_GetPos 相同，SDSTK_GetPos_Unsafe 不会进行范围安全检查。