## LLIST（Linked List）模块：

分类：数据结构（用于管理特性形态的数据）

LLIST 为双向链表数据结构管理器，适用于管理数据形态为链表，链表元素容量无限制的场景。

LLIST 提供了两个模块：LLIST_BASE 和 LLIST，其中 LLIST_BASE 以 LLB 作为函数前缀，LLIST 以 LList 作为函数前缀。

LLIST_BASE 与 LLIST 的主要区别在于，LLIST_BASE 不进行任何内存分配操作，只负责维护链表数据；而 LLIST 模块内部集成了 MM256 对象管理内存，可以自行管理链表节点元素的内存申请和释放。

LLIST_BASE 与 LLIST 在功能上的区别在于，LLIST_BASE 主要用于嵌入到其他数据结构中，作为其他数据结构的一部分进行工作，LLIST 则更适合独立进行工作。

MMU 没有提供单向链表，因为单向链表实现起来比较简单，虽然双向链表也并不复杂，但作为一种基础且常用的数据结构，还是有实现必要的。

### LList_BaseStruct [数据结构]
``` c
// 双向链表节点基础定义
typedef struct LList_NodeBase {
	struct LList_NodeBase* Prev;
	struct LList_NodeBase* Next;
} LList_NodeBase;

// 链表对象数据结构
typedef struct {
	LList_NodeBase* FirstNode;
	LList_NodeBase* LastNode;
	unsigned int Count;
} LList_BaseStruct, *LList_BaseObject;
```
LList_BaseStruct 是 LLIST_BASE 对象的基本结构体数据，由于 LLIST_BASE 不进行任何内存管理，因此 LList_BaseStruct 结构体内存必须由开发者申请，并使用 LLB_Init 函数进行初始化。

LLIST_BASE 必须使用已经初始化过的 LList_BaseStruct 数据结构（使用 LLB_Init），否则在进行 API 调用时，可能产生无法预知的严重后果。

### LList_BaseObject [对象指针]
LList_BaseObject 是 LList_BaseStruct 数据结构的指针。

### LList_Struct [数据结构]
``` c
typedef struct {
	LList_NodeBase* FirstNode;
	LList_NodeBase* LastNode;
	unsigned int Count;
	MM256_Struct objMM;
} LList_Struct, *LList_Object;
```
LList_Struct 是 LLIST 对象的基本结构体数据，可以通过 LLIST_Create 函数创建一个指向 LList_Struct 结构体的指针（也就是 LLIST_Object）。

LLIST 必须使用已经初始化过的 LList_Struct 数据结构（使用 LLIST_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### LLIST_Object [对象指针]
LLIST_Object 是 LList_Struct 数据结构的指针，通过 LLIST_Create 创建的 LLIST 对象会返回 LLIST_Object 对象指针。

### LLB_Init - 初始化链表
声明：void LLB_Init(LList_BaseObject objLL);
对 LLIST_BASE 数据进行初始化。

### LLB_Unit - 释放链表
声明：void LLB_Unit(LList_BaseObject objLL);
对 LLIST_BASE 数据进行初始化（由于不具备内存管理功能，LLIST_BASE 的释放操作和初始化操作，实际上是相同的）。

### LLB_InsertPrev - 节点前插入
声明：void LLB_InsertPrev(LList_BaseObject objLLB, LList_NodeBase* objNode, LList_NodeBase* objNewNode);
向 objNode 节点的前面，插入新节点（objNewNode）。
当 objNode 为 NULL 时，将节点插入到链表最前端（替代 FirstNode）。

### LLB_InsertNext - 节点后插入
声明：void LLB_InsertNext(LList_BaseObject objLLB, LList_NodeBase* objNode, LList_NodeBase* objNewNode);
向 objNode 节点的后面，插入新节点（objNewNode）。
当 objNode 为 NULL 时，将节点插入到链表末尾（替代 LastNode）。

### LLB_Remove - 删除节点
声明：void LLB_Remove(LList_BaseObject objLLB, LList_NodeBase* objNode);
删除 objNode 节点，将 objNode 的前后节点互联。
由于 LLIST_BASE 不具备内存管理功能，删除后 objNode 对象占用的内存，需要调用方释放。

### LLB_RemoveAll - 删除所有成员
LLB_Unit 的别名，为了避免歧义时，可以使用 LLB_RemoveAll。

### LLB_Clear - 清空管理器
LLB_Unit 的别名，为了避免歧义时，可以使用 LLB_Clear。

### LList_Create - 创建链表
声明：LList_Object LList_Create(unsigned int iItemLength);
创建 LLIST 对象并初始化数据，创建成功时返回一个 LList_Object 对象指针，创建失败返回 NULL。
由 LList_Create 创建的对象，不再使用时应调用 LList_Destroy 函数将 LLIST 对象销毁，否则会产生内存泄漏。

### LList_Destroy - 销毁链表
声明：void LList_Destroy(LList_Object objLL);
销毁由 LList_Create 函数创建的 LLIST 对象。

### LList_Init - 初始化链表
声明：void LList_Init(LList_Object objLL, unsigned int iItemLength);
这个函数和 LList_Create 功能类似，它在不申请 LLIST 对象内存的情况下，对已存在的 LList_Struct 数据进行初始化，将 LList_Struct 嵌入到其他数据结构中的情况下，应使用此函数进行数据初始化。
嵌入到其他数据结构的 LList_Struct 不再使用时应调用 LList_Unit 函数释放对象中的数据，否则会产生内存泄漏。

### LList_Unit - 释放链表
声明：void LList_Unit(LList_Object objLL);
释放 LLIST 对象内的数据，它会将管理器成员数量清零，并释放管理器占用的内存。

### LList_InsertPrev - 节点前插入
声明：LList_NodeBase* LList_InsertPrev(LList_Object objLL, LList_NodeBase* objNode);
向 objNode 节点的前面，插入新节点，并返回新节点的结构体指针。
当 objNode 为 NULL 时，将节点插入到链表最前端（替代 FirstNode）。

### LList_InsertNext - 节点后插入
声明：LList_NodeBase* LList_InsertNext(LList_Object objLL, LList_NodeBase* objNode);
向 objNode 节点的后面，插入新节点，并返回新节点的结构体指针。
当 objNode 为 NULL 时，将节点插入到链表末尾（替代 LastNode）。

### LList_Remove - 删除节点
声明：void LList_Remove(LList_Object objLL, LList_NodeBase* objNode);
删除 objNode 节点，将 objNode 的前后节点互联。

### LList_RemoveAll - 删除所有成员
LList_Unit 的别名，为了避免歧义时，可以使用 LList_RemoveAll。

### LList_Clear - 清空管理器
LList_Unit 的别名，为了避免歧义时，可以使用 LList_Clear。