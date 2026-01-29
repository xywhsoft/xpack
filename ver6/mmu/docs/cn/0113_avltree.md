## AVLTree 模块：

分类：数据结构（用于管理特性形态的数据）

AVLTree 为平衡二叉树数据结构管理器，得益于二分法的高性能，平衡二叉树主要应用于需要对数据进行快速查找的场景。

AVLTree 提供了两个模块：AVLTREE_BASE 和 AVLTREE，其中 AVLTREE_BASE 以 AVLTB 作为函数前缀，AVLTREE 以 AVLTree 作为函数前缀。

AVLTREE_BASE 与 AVLTREE 的主要区别在于，AVLTREE_BASE 不进行任何内存分配操作，只负责维护平衡二叉树数据；而 AVLTREE 模块内部集成了 MM256 对象管理内存，可以自行管理平衡二叉树节点元素的内存申请和释放。

AVLTREE_BASE 与 AVLTREE 在功能上的区别在于，AVLTREE_BASE 主要用于嵌入到其他数据结构中，作为其他数据结构的一部分进行工作，AVLTREE 则更适合独立进行工作。

二叉树在 插入、删除、查询、遍历 等性能上取得了非常好的平衡，而平衡二叉树最擅长的是数据查询，理论上它可以在10次循环内，从40亿个数据中找到所需的数据。

平衡二叉树和红黑树主要的区别在于，红黑树更重视插入、删除的性能，而平衡二叉树更重视查找的性能。

AVLTree 使用 MM256 管理内存。

### AVLTree_BaseStruct [数据结构]
``` c
// AVL树节点基础定义
typedef struct AVLTree_NodeBase {
	struct AVLTree_NodeBase* left;
	struct AVLTree_NodeBase* right;
	int height;
} AVLTree_NodeBase;

// 比较回调函数
typedef int (*AVLTree_CompProc)(void* pNode, void* pKey);

// 遍历回调函数
typedef int (*AVLTree_EachProc)(void* pNode, void* pArg);

// AVL树对象数据结构
typedef struct {
	AVLTree_NodeBase* RootNode;
	unsigned int Count;
} AVLTree_BaseStruct, *AVLTree_BaseObject;
```
AVLTree_BaseStruct 是 AVLTREE_BASE 对象的基本结构体数据，由于 AVLTREE_BASE 不进行任何内存管理，因此 AVLTree_BaseStruct 结构体内存必须由开发者申请，并使用 LLB_Init 函数进行初始化。

AVLTREE_BASE 必须使用已经初始化过的 AVLTree_BaseStruct 数据结构（使用 LLB_Init），否则在进行 API 调用时，可能产生无法预知的严重后果。

### AVLTree_BaseObject [对象指针]
AVLTree_BaseObject 是 AVLTree_BaseStruct 数据结构的指针。

### AVLTree_Struct [数据结构]
``` c
// 值释放回调函数
typedef void (*AVLTree_FreeProc)(void* objTree, void* pNode);

// AVL树对象数据结构
typedef struct {
	AVLTree_NodeBase* RootNode;
	unsigned int Count;
	AVLTree_CompProc CompProc;
	AVLTree_FreeProc FreeProc;
	MM256_Struct objMM;
	void* ExtData;
	AVLTree_NodeBase* NodeCache;
} AVLTree_Struct, *AVLTree_Object;
```
AVLTree_Struct 是 AVLTREE 对象的基本结构体数据，可以通过 AVLTree_Create 函数创建一个指向 AVLTree_Struct 结构体的指针（也就是 AVLTree_Object）。

AVLTREE 必须使用已经初始化过的 AVLTree_Struct 数据结构（使用 AVLTree_Create），否则在进行 API 调用时，可能产生无法预知的严重后果。

### AVLTree_Object [对象指针]
AVLTree_Object 是 AVLTree_Struct 数据结构的指针，通过 AVLTree_Create 创建的 AVLTREE 对象会返回 AVLTree_Object 对象指针。

### AVLTree_GetNodeBase - 根据数据段获取 AVLTree_NodeBase 结构体指针
声明：void AVLTree_GetNodeBase(void* pData);
根据数据段获取 AVLTree_NodeBase 结构体指针，由于 AVLTree 节点可能用于存储比较复杂的数据，因此将一个节点分为数据段和信息段，信息段即 AVLTree_NodeBase 结构体，数据段则是开发者自己定义的数据结构。

### AVLTree_GetNodeData - 获取 AVLTree_NodeBase 结构体指针对应的数据段
声明：void AVLTree_GetNodeData(AVLTree_NodeBase* pNode);
根据信息段获取数据段结构体指针，由于 AVLTree 节点可能用于存储比较复杂的数据，因此将一个节点分为数据段和信息段，信息段即 AVLTree_NodeBase 结构体，数据段则是开发者自己定义的数据结构。

### AVLTree_GetRootData - 获取根节点数据段
声明：void AVLTree_GetNodeBase(AVLTree_BaseObject objAVLT);
获取 RootNode 的数据段结构体指针。

### AVLTB_Init - 初始化 AVLTree
声明：void AVLTB_Init(AVLTree_BaseObject objAVLT);
对 AVLTREE_BASE 数据进行初始化。

### AVLTB_Unit - 释放 AVLTree
声明：void AVLTB_Unit(AVLTree_BaseObject objAVLT);
对 AVLTREE_BASE 数据进行初始化（由于不具备内存管理功能，AVLTREE_BASE 的释放操作和初始化操作，实际上是相同的）。

### AVLTB_Insert - 向 AVLTree 中插入节点
声明：AVLTree_NodeBase* AVLTB_Insert(AVLTree_BaseObject objAVLT, AVLTree_CompProc procComp, void* pKey, AVLTree_NodeBase* pNewNode);
向 AVLTREE_BASE 中插入一个新节点（pNewNode），节点根据 procComp 回调的排序规则进行排序，节点的 Key 为 pKey（pKey可以是任意数据结构，会传递给 procComp 进行比较排序）。
如果新插入的节点已经存在了，则会返回已存在的节点指针（可判断返回值确定是否是新节点）。
AVLTB_Insert 会在数据插入后，自动进行树平衡操作。

### AVLTB_Remove - 从 AVLTree 中删除节点
声明：AVLTree_NodeBase* AVLTB_Remove(AVLTree_BaseObject objAVLT, AVLTree_CompProc procComp, void* pKey);
将 pKey 对应的节点从 AVLTREE_BASE 中删除，节点根据 procComp 回调的规则进行 Key 对比。
AVLTB_Remove 会在数据删除后，自动进行树平衡操作。

### AVLTB_Search - 在 AVLTree 中查找节点
声明：AVLTree_NodeBase* AVLTB_Search(AVLTree_BaseObject objAVLT, AVLTree_CompProc procComp, void* pKey);
在 AVLTREE_BASE 中查找 pKey 对应的节点，节点根据 procComp 回调的规则进行 Key 对比。

### AVLTB_RemoveAll - 删除所有节点
AVLTB_Unit 的别名，为了避免歧义时，可以使用 AVLTB_RemoveAll。

### AVLTB_Clear - 清空管理器
AVLTB_Unit 的别名，为了避免歧义时，可以使用 AVLTB_Clear。

### AVLTB_Walk - 遍历 AVLTree 所有节点
声明：void AVLTB_Walk(AVLTree_BaseObject objAVLT, AVLTree_EachProc recuProc, void* pArg);
通过回调函数 recuProc 遍历 AVLTREE_BASE 中所有节点，pArg 会作为 recuProc 的参数传递。

### AVLTB_WalkEx - 遍历 AVLTree 所有节点
声明：void AVLTB_Walk(AVLTree_BaseObject objAVLT, AVLTree_EachProc recuProc1, AVLTree_EachProc recuProc2, AVLTree_EachProc3 recuProc, void* pArg);
通过回调函数 recuProc1、recuProc2、recuProc3 遍历 AVLTREE_BASE 中所有节点，pArg 会作为 recuProc 的参数传递。
recuProc1：进入左节点时调用
recuProc2：中间调用（AVLTB_Walk的遍历规则）
recuProc3：进入后节点时调用

### AVLTree_Create - 创建 AVLTree
声明：AVLTree_Object AVLTree_Create(unsigned int iItemLength, AVLTree_CompProc procComp);
创建 AVLTREE 对象并初始化数据，创建成功时返回一个 AVLTree_Object 对象指针，创建失败返回 NULL。
由 AVLTree_Create 创建的对象，不再使用时应调用 AVLTree_Destroy 函数将 AVLTREE 对象销毁，否则会产生内存泄漏。

### AVLTree_Destroy - 销毁 AVLTree
声明：void AVLTree_Destroy(AVLTree_Object objAVLT);
销毁由 AVLTree_Create 函数创建的 AVLTREE 对象。

### AVLTree_Init - 初始化 AVLTree
声明：void AVLTree_Init(AVLTree_Object objAVLT, unsigned int iItemLength, AVLTree_CompProc procComp);
这个函数和 AVLTree_Create 功能类似，它在不申请 AVLTREE 对象内存的情况下，对已存在的 AVLTree_Struct 数据进行初始化，将 AVLTree_Struct 嵌入到其他数据结构中的情况下，应使用此函数进行数据初始化。
嵌入到其他数据结构的 AVLTree_Struct 不再使用时应调用 AVLTree_Unit 函数释放对象中的数据，否则会产生内存泄漏。

### AVLTree_Unit - 释放 AVLTree
声明：void AVLTree_Unit(AVLTree_Object objAVLT);
释放 AVLTREE 对象内的数据，它会将管理器成员数量清零，并释放管理器占用的内存。

### AVLTree_Insert - 向 AVLTree 中插入节点
声明：void* AVLTree_Insert(AVLTree_Object objAVLT, void* pKey, int* bNew);
向 AVLTREE 中插入一个新节点，节点的 Key 为 pKey（pKey可以是任意数据结构，会传递给 procComp 进行比较排序）。
参数 bNew 可传递一个 int 指针，如果这个节点已经存在了，则 bNew 返回 FALSE，否则返回 TRUE。
AVLTree_Insert 会在数据插入后，自动进行树平衡操作。

### AVLTree_Remove - 从 AVLTree 中删除节点
声明：int AVLTree_Remove(AVLTree_Object objAVLT, void* pKey);
将 pKey 对应的节点从 AVLTREE 中删除，节点根据 procComp 回调的规则进行 Key 对比。
AVLTree_Remove 会在数据删除后，自动进行树平衡操作。

### AVLTree_Search - 在 AVLTree 中查找节点
声明：void* AVLTree_Search(AVLTree_Object objAVLT, void* pKey);
在 AVLTREE 中查找 pKey 对应的节点，节点根据 procComp 回调的规则进行 Key 对比。

### AVLTree_RemoveAll - 删除所有节点
AVLTree_Unit 的别名，为了避免歧义时，可以使用 AVLTree_RemoveAll。

### AVLTree_Clear - 清空管理器
AVLTree_Unit 的别名，为了避免歧义时，可以使用 AVLTree_Clear。

### AVLTree_Walk - 遍历 AVLTree 所有节点
声明：void AVLTree_Walk(AVLTree_Object objAVLT, AVLTree_EachProc recuProc, void* pArg);
通过回调函数 recuProc 遍历 AVLTREE 中所有节点，pArg 会作为 recuProc 的参数传递。

### AVLTree_WalkEx - 遍历 AVLTree 所有节点
声明：void AVLTree_WalkEx(AVLTree_Object objAVLT, AVLTree_EachProc recuProc1, AVLTree_EachProc recuProc2, AVLTree_EachProc3 recuProc, void* pArg);
AVLTree_EachProc3 recuProc, void* pArg);
通过回调函数 recuProc1、recuProc2、recuProc3 遍历 AVLTREE 中所有节点，pArg 会作为 recuProc 的参数传递。
recuProc1：进入左节点时调用
recuProc2：中间调用（AVLTB_Walk的遍历规则）
recuProc3：进入后节点时调用
