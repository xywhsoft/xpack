


// AVL树比较函数
int xTable_Proc_CompareA(AVL_NODE *node, void *key)
{
	pTableNode pNode = (pTableNode)node;
	#if defined(__x86_64__) || defined(_WIN64)
		uint64 iHashKey = XXH64(key, strlen(key), 0);
	#else
		uint32 iHashKey = XXH32(key, strlen(key), 0);
	#endif
	if ( pNode->Hash < iHashKey ) {
		return -1;
	} else if ( pNode->Hash > iHashKey ) {
		return 1;
	} else {
		return strcmp((astr)pNode->Key, (astr)key);
	}
}
int xTable_Proc_CompareW(AVL_NODE *node, void *key)
{
	pTableNode pNode = (pTableNode)node;
	#if defined(__x86_64__) || defined(_WIN64)
		uint64 iHashKey = XXH64(key, strlen(key), 0);
	#else
		uint32 iHashKey = XXH32(key, strlen(key), 0);
	#endif
	if ( pNode->Hash < iHashKey ) {
		return -1;
	} else if ( pNode->Hash > iHashKey ) {
		return 1;
	} else {
		return wcscmp((wstr)pNode->Key, (wstr)key);
	}
}

// AVL树销毁函数
int xTable_Proc_Free(AVL_NODE *pNode)
{
	free(((pTableNode)pNode)->Key);
	free(pNode);
	return AVL_RET_OK;
}

// AVL树遍历函数
int xTable_Proc_Scan(AVL_NODE *pNode, void * pArg)
{
	((TBL_PROC_SCAN)pArg)((pTableNode)pNode);
	return AVL_RET_OK;
}



// 创建表
XXAPI xTableObject xTable_Create()
{
	xTableObject objTable = malloc(sizeof(ptr));
	if ( objTable ) {
		*objTable = NULL;
	}
	return objTable;
}

// 销毁表
XXAPI int xTable_Destroy(xTableObject tblRoot)
{
	return avl_destroy(*tblRoot, xTable_Proc_Free);
}

// 插入数据
XXAPI pTableNode xTable_InsertA(xTableObject tblRoot, astr sKey, ptr pVal)
{
	pTableNode pNode = malloc(sizeof(xTableNode));
	pNode->Key = (str)xCore_CopyStringA(sKey, 0);
	#if defined(__x86_64__) || defined(_WIN64)
		pNode->Hash = XXH64(sKey, strlen(sKey), 0);
	#else
		pNode->Hash = XXH32(sKey, strlen(sKey), 0);
	#endif
	pNode->Val = pVal;
	int iRet = avl_insert(tblRoot, &pNode->avlNode, sKey, xTable_Proc_CompareA);
	if ( iRet == AVL_RET_OK ) {
		return pNode;
	} else {
		free(pNode);
		return NULL;
	}
}
XXAPI pTableNode xTable_InsertW(xTableObject tblRoot, wstr sKey, ptr pVal)
{
	pTableNode pNode = malloc(sizeof(xTableNode));
	pNode->Key = (str)xCore_CopyStringW(sKey, 0);
	#if defined(__x86_64__) || defined(_WIN64)
		pNode->Hash = XXH64(sKey, wcslen(sKey), 0);
	#else
		pNode->Hash = XXH32(sKey, wcslen(sKey), 0);
	#endif
	pNode->Val = pVal;
	int iRet = avl_insert(tblRoot, &pNode->avlNode, sKey, xTable_Proc_CompareW);
	if ( iRet == AVL_RET_OK ) {
		return pNode;
	} else {
		free(pNode);
		return NULL;
	}
}

// 查找数据
XXAPI pTableNode xTable_SearchA(xTableObject tblRoot, astr sKey)
{
	return (pTableNode)avl_search(*tblRoot, sKey, xTable_Proc_CompareA);
}
XXAPI pTableNode xTable_SearchW(xTableObject tblRoot, wstr sKey)
{
	return (pTableNode)avl_search(*tblRoot, sKey, xTable_Proc_CompareW);
}

// 读取数据
XXAPI ptr xTable_GetValA(xTableObject tblRoot, astr sKey)
{
	pTableNode pNode = (pTableNode)avl_search(*tblRoot, sKey, xTable_Proc_CompareA);
	if ( pNode ) {
		return pNode->Val;
	} else {
		return NULL;
	}
}
XXAPI ptr xTable_GetValW(xTableObject tblRoot, wstr sKey)
{
	pTableNode pNode = (pTableNode)avl_search(*tblRoot, sKey, xTable_Proc_CompareW);
	if ( pNode ) {
		return pNode->Val;
	} else {
		return NULL;
	}
}

// 修改数据
XXAPI pTableNode xTable_SetValA(xTableObject tblRoot, astr sKey, ptr pVal)
{
	pTableNode pNode = (pTableNode)avl_search(*tblRoot, sKey, xTable_Proc_CompareA);
	if ( pNode ) {
		pNode->Val = pVal;
		return pNode;
	} else {
		return xTable_InsertA(tblRoot, sKey, pVal);
	}
}
XXAPI pTableNode xTable_SetValW(xTableObject tblRoot, wstr sKey, ptr pVal)
{
	pTableNode pNode = (pTableNode)avl_search(*tblRoot, sKey, xTable_Proc_CompareW);
	if ( pNode ) {
		pNode->Val = pVal;
		return pNode;
	} else {
		return xTable_InsertW(tblRoot, sKey, pVal);
	}
}

// 删除数据
XXAPI int xTable_RemoveA(xTableObject tblRoot, astr sKey)
{
	 pTableNode pNode = (pTableNode)avl_delete(tblRoot, sKey, xTable_Proc_CompareA);
	 if ( pNode ) {
		xTable_Proc_Free((AVL_NODE*)pNode);
		return -1;
	 }
	 return 0;
}
XXAPI int xTable_RemoveW(xTableObject tblRoot, wstr sKey)
{
	 pTableNode pNode = (pTableNode)avl_delete(tblRoot, sKey, xTable_Proc_CompareW);
	 if ( pNode ) {
		xTable_Proc_Free((AVL_NODE*)pNode);
		return -1;
	 }
	 return 0;
}

// 遍历表
XXAPI int xTable_Scan(xTableObject tblRoot, TBL_PROC_SCAN procScan)
{
	int iCount = 0;
	avl_tree_walk(*tblRoot, NULL, NULL, NULL, NULL, xTable_Proc_Scan, procScan);
	return iCount;
}


