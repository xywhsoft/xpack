


#ifndef XXRTL_StructMemoryManage
	#define XXRTL_StructMemoryManage
	
	
	
	// 管理器结构体定义
	typedef struct {
		char* StructMemory;						// 管理器内存指针
		unsigned int StructLenght;				// 成员占用内存长度
		unsigned int StructCount;				// 管理器中存在多少成员
		unsigned int AllocCount;				// 已经申请的结构数量
		unsigned int AllocStep;					// 预分配内存步长
	} SmmStruct, *SmmObject;
	
	// 创建结构化内存管理器
	SmmObject SMM_Create(unsigned int iItemLenght, unsigned int PreassignStep);

	// 重置结构化内存管理器
	void SMM_ReInit(SmmObject pObject);

	// 销毁结构化内存管理器
	void SMM_Destroy(SmmObject pObject);

	// 分配内存
	int SMM_Malloc(SmmObject pObject, unsigned int iCount);

	// 中间插入成员
	unsigned int SMM_Insert(SmmObject pObject, unsigned int iPos, unsigned int iCount);

	// 末尾添加成员
	unsigned int SMM_Append(SmmObject pObject, unsigned int iCount);

	// 交换成员
	int SMM_Swap(SmmObject pObject, unsigned int iPosA, unsigned int iPosB);

	// 删除成员
	int SMM_Delete(SmmObject pObject, unsigned int iPos, unsigned int iCount);

	// 获取成员指针
	void* SMM_GetPtr(SmmObject pObject, unsigned int iPos);
	
	
	
#endif


