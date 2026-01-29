## RBTree (Red Black Tree) 模块：

分类：数据结构（用于管理特性形态的数据）

RBTree 为平衡二叉树数据结构管理器，得益于二分法的高性能，平衡二叉树主要应用于需要对数据进行快速查找的场景。

RBTree 提供了两个模块：RBTREE_BASE 和 RBTREE，其中 RBTREE_BASE 以 RBTB 作为函数前缀，RBTREE 以 RBTree 作为函数前缀。

RBTREE_BASE 与 RBTREE 的主要区别在于，RBTREE_BASE 不进行任何内存分配操作，只负责维护平衡二叉树数据；而 RBTREE 模块内部集成了 MM256 对象管理内存，可以自行管理平衡二叉树节点元素的内存申请和释放。

RBTREE_BASE 与 RBTREE 在功能上的区别在于，RBTREE_BASE 主要用于嵌入到其他数据结构中，作为其他数据结构的一部分进行工作，RBTREE 则更适合独立进行工作。

红黑树特点和 AVLTree 类似，红黑树有更好的数据插入删除性能，数据查询性能较差，选择取舍的关键，在于数据是读多（用AVLTree）还是写多（用RBTree）。

RBTree 使用 MM256 管理内存。

### RBTree_BaseStruct [数据结构]
``` c
// 定义红黑树颜色
 #define MMU_RBT_RED		0
 #define MMU_RBT_BLACK	1

// 红黑树节点基础定义
typedef struct RBTree_NodeBase {
	struct RBTree_NodeBase* left;
	struct RBTree_NodeBase* right;
	intptr_t parent_color;
} RBTree_NodeBase;

// 比较回调函数
typedef int (*RBTree_CompProc)(void* pNode, void* pKey);

// 遍历回调函数
typedef int (*RBTree_EachProc)(void* pNode, void* pArg);

// 红黑树对象数据结构
typedef struct {
	RBTree_NodeBase* RootNode;
	unsigned int Count;
} RBTree_BaseStruct, *RBTree_BaseObject;
```
RBTree_BaseStruct 是 RBTREE_BASE 对象的基本结构体数据，由于 RBTREE_BASE 不进行任何内存管理，因此 RBTree_BaseStruct 结构体内存必须由开发者申请，并使用 LLB_Init 函数进行初始化。

RBTREE_BASE 必须使用已经初始化过的 RBTree_BaseStruct 数据结构（使用 LLB_Init），否则在进行 API 调用时，可能产生无法预知的严重后果。

### RBTree_BaseObject [对象指针]
RBTree_BaseObject 是 RBTree_BaseStruct 数据结构的指针。

### RBTree_Struct [数据结构]
``` c
// 值释放回调函数
typedef int (*RBTree_FreeProc)(void* objTree, void* pNode);

// 红黑树对象数据结构
typedef struct {
	RBTree_NodeBase* RootNode;
	unsigned int Count;
	RBTree_CompProc CompProc;
	RBTree_FreeProc FreeProc;
	MM256_Struct objMM;
	void* ExtData;
	RBTree_NodeBase* NodeCache;
} RBTree_Struct, *RBTree_Object;
```
RBTree_Struct 是 RBTREE 对象的基本结构体数据，可以通过 RBTree_Create 函数创建一个指向 RBTree_Struct 结构体的指针（也就是 RBTree_Object）。

RBTREE 必须使用已经初始化过的 RBTree_Struct 数据结构（使用 RBTree_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### RBTree_Object [对象指针]
RBTree_Object 是 RBTree_Struct 数据结构的指针，通过 RBTree_Create 创建的 RBTREE 对象会返回 RBTree_Object 对象指针。

### RBTree_GetNodeBase - 根据数据段获取 RBTree_NodeBase 结构体指针
声明：void RBTree_GetNodeBase(void* pData);
根据数据段获取 RBTree_NodeBase 结构体指针，由于 RBTree 节点可能用于存储比较复杂的数据，因此将一个节点分为数据段和信息段，信息段即 RBTree_NodeBase 结构体，数据段则是开发者自己定义的数据结构。

### RBTree_GetNodeData - 获取 RBTree_NodeBase 结构体指针对应的数据段
声明：void RBTree_GetNodeData(RBTree_NodeBase* pNode);
根据信息段获取数据段结构体指针，由于 RBTree 节点可能用于存储比较复杂的数据，因此将一个节点分为数据段和信息段，信息段即 RBTree_NodeBase 结构体，数据段则是开发者自己定义的数据结构。

### RBTree_GetRootData - 获取根节点数据段
声明：void RBTree_GetNodeBase(RBTree_BaseObject objRBT);
获取 RootNode 的数据段结构体指针。

### RBTB_Init - 初始化 RBTree
声明：void RBTB_Init(RBTree_BaseObject objRBT);
对 RBTREE_BASE 数据进行初始化。

### RBTB_Unit - 释放 RBTree
声明：void RBTB_Unit(RBTree_BaseObject objRBT);
对 RBTREE_BASE 数据进行初始化（由于不具备内存管理功能，RBTREE_BASE 的释放操作和初始化操作，实际上是相同的）。

### RBTB_Insert - 向 RBTree 中插入节点
声明：RBTree_NodeBase* RBTB_Insert(RBTree_BaseObject objRBT, RBTree_CompProc procComp, void* pKey, RBTree_NodeBase* pNewNode);
向 RBTREE_BASE 中插入一个新节点（pNewNode），节点根据 procComp 回调的排序规则进行排序，节点的 Key 为 pKey（pKey可以是任意数据结构，会传递给 procComp 进行比较排序）。
如果新插入的节点已经存在了，则会返回已存在的节点指针（可判断返回值确定是否是新节点）。
RBTB_Insert 会在数据插入后，自动进行树平衡操作。

### RBTB_Remove - 从 RBTree 中删除节点
声明：RBTree_NodeBase* RBTB_Remove(RBTree_BaseObject objRBT, RBTree_CompProc procComp, void* pKey);
将 pKey 对应的节点从 RBTREE_BASE 中删除，节点根据 procComp 回调的规则进行 Key 对比。
RBTB_Remove 会在数据删除后，自动进行树平衡操作。

### RBTB_Search - 在 RBTree 中查找节点
声明：RBTree_NodeBase* RBTB_Search(RBTree_BaseObject objRBT, RBTree_CompProc procComp, void* pKey);
在 RBTREE_BASE 中查找 pKey 对应的节点，节点根据 procComp 回调的规则进行 Key 对比。

### RBTB_RemoveAll - 删除所有节点
RBTB_Unit 的别名，为了避免歧义时，可以使用 RBTB_RemoveAll。

### RBTB_Clear - 清空管理器
RBTB_Unit 的别名，为了避免歧义时，可以使用 RBTB_Clear。

### RBTB_Walk - 遍历 RBTree 所有节点
声明：void RBTB_Walk(RBTree_BaseObject objRBT, RBTree_EachProc recuProc, void* pArg);
通过回调函数 recuProc 遍历 RBTREE_BASE 中所有节点，pArg 会作为 recuProc 的参数传递。

### RBTB_WalkEx - 遍历 RBTree 所有节点
声明：void RBTB_Walk(RBTree_BaseObject objRBT, RBTree_EachProc recuProc1, RBTree_EachProc recuProc2, RBTree_EachProc3 recuProc, void* pArg);
通过回调函数 recuProc1、recuProc2、recuProc3 遍历 RBTREE_BASE 中所有节点，pArg 会作为 recuProc 的参数传递。
recuProc1：进入左节点时调用
recuProc2：中间调用（RBTB_Walk的遍历规则）
recuProc3：进入后节点时调用

### RBTree_Create - 创建 RBTree
声明：RBTree_Object RBTree_Create(unsigned int iItemLength, RBTree_CompProc procComp);
创建 RBTREE 对象并初始化数据，创建成功时返回一个 RBTree_Object 对象指针，创建失败返回 NULL。
由 RBTree_Create 创建的对象，不再使用时应调用 RBTree_Destroy 函数将 RBTREE 对象销毁，否则会产生内存泄漏。

### RBTree_Destroy - 销毁 RBTree
声明：void RBTree_Destroy(RBTree_Object objRBT);
销毁由 RBTree_Create 函数创建的 RBTREE 对象。

### RBTree_Init - 初始化 RBTree
声明：void RBTree_Init(RBTree_Object objRBT, unsigned int iItemLength, RBTree_CompProc procComp);
这个函数和 RBTree_Create 功能类似，它在不申请 RBTREE 对象内存的情况下，对已存在的 RBTree_Struct 数据进行初始化，将 RBTree_Struct 嵌入到其他数据结构中的情况下，应使用此函数进行数据初始化。
嵌入到其他数据结构的 RBTree_Struct 不再使用时应调用 RBTree_Unit 函数释放对象中的数据，否则会产生内存泄漏。

### RBTree_Unit - 释放 RBTree
声明：void RBTree_Unit(RBTree_Object objRBT);
释放 RBTREE 对象内的数据，它会将管理器成员数量清零，并释放管理器占用的内存。

### RBTree_Insert - 向 RBTree 中插入节点
声明：void* RBTree_Insert(RBTree_Object objRBT, void* pKey, int* bNew);
向 RBTREE 中插入一个新节点，节点的 Key 为 pKey（pKey可以是任意数据结构，会传递给 procComp 进行比较排序）。
参数 bNew 可传递一个 int 指针，如果这个节点已经存在了，则 bNew 返回 FALSE，否则返回 TRUE。
RBTree_Insert 会在数据插入后，自动进行树平衡操作。

### RBTree_Remove - 从 RBTree 中删除节点
声明：int RBTree_Remove(RBTree_Object objRBT, void* pKey);
将 pKey 对应的节点从 RBTREE 中删除，节点根据 procComp 回调的规则进行 Key 对比。
RBTree_Remove 会在数据删除后，自动进行树平衡操作。

### RBTree_Search - 在 RBTree 中查找节点
声明：void* RBTree_Search(RBTree_Object objRBT, void* pKey);
在 RBTREE 中查找 pKey 对应的节点，节点根据 procComp 回调的规则进行 Key 对比。

### RBTree_RemoveAll - 删除所有节点
RBTree_Unit 的别名，为了避免歧义时，可以使用 RBTree_RemoveAll。

### RBTree_Clear - 清空管理器
RBTree_Unit 的别名，为了避免歧义时，可以使用 RBTree_Clear。

### RBTree_Walk - 遍历 RBTree 所有节点
声明：void RBTree_Walk(RBTree_Object objRBT, RBTree_EachProc recuProc, void* pArg);
通过回调函数 recuProc 遍历 RBTREE 中所有节点，pArg 会作为 recuProc 的参数传递。

### RBTree_WalkEx - 遍历 RBTree 所有节点
声明：void RBTree_WalkEx(RBTree_Object objRBT, RBTree_EachProc recuProc1, RBTree_EachProc recuProc2, RBTree_EachProc3 recuProc, void* pArg);
RBTree_EachProc3 recuProc, void* pArg);
通过回调函数 recuProc1、recuProc2、recuProc3 遍历 RBTREE 中所有节点，pArg 会作为 recuProc 的参数传递。
recuProc1：进入左节点时调用
recuProc2：中间调用（RBTB_Walk的遍历规则）
recuProc3：进入后节点时调用
