


// test Tree struct
typedef struct {
	Tree_NodeBase NodeInfo;
	int idx;
	char Title[32];
} Tree_Test_Struct, *Tree_Test_Object;



void test_tree_eachnode(TreeLL_BaseObject objList, int iLevel)
{
	Tree_Test_Object objNode = (Tree_Test_Object)objList->FirstNode;
	while ( objNode ) {
		for ( int i = 0; i < iLevel; i++ ) {
			printf("    ");
		}
		printf("%s (%d) [%d]\n", objNode->Title, objNode->idx, objNode->NodeInfo.Childs.Count);
		test_tree_eachnode(&(objNode->NodeInfo.Childs), iLevel + 1);
		objNode = (Tree_Test_Object)objNode->NodeInfo.Next;
	}
}



void test_tree()
{
	system("cls");
	MMU_Init();
	
	
	
	// subject 1 : create object
	printf("Tree test subject 1 : create object\n\n");
	Tree_Object objTree = Tree_Create(sizeof(Tree_Test_Struct) - sizeof(Tree_NodeBase));
	if ( objTree ) {
		printf("Tree object : %p\t\t\t\t\tpass! √\n", objTree);
		printf("\tCount : %d\t\t\t\t=> 0\n", objTree->Count);
		printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objTree->NodeMM.ItemLength, sizeof(Tree_Test_Struct));
		printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objTree->NodeMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
		printf("\tMM.arrMMU.Count : %d\t\t\t=> 0\n", objTree->NodeMM.arrMMU.Count);
		printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 0\n", objTree->NodeMM.arrMMU.PageMMU.Count);
		printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 0\n", objTree->NodeMM.arrMMU.PageMMU.AllocCount);
		printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objTree->NodeMM.arrMMU.PageMMU.AllocStep);
		if ( objTree->NodeMM.arrMMU.PageMMU.Memory ) {
			printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objTree->NodeMM.arrMMU.PageMMU.Memory);
		} else {
			printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objTree->NodeMM.arrMMU.PageMMU.Memory);
		}
	} else {
		printf("Tree object : %p\t\t\t\t\tfail! ×\n", objTree);
	}
	
	printf("\nTree nodes : \n");
	test_tree_eachnode(&objTree->Root, 1);
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	// subject 2 : insert node
	printf("Tree test subject 2 : insert node\n\n");
	Tree_Test_Object objParent1 = (Tree_Test_Object)Tree_InsertNode(objTree, NULL, TREE_INS_NEXT);
	if ( objParent1 ) {
		objParent1->idx = 1;
		sprintf(objParent1->Title, "New node idx = %d", objParent1->idx);
	}
	Tree_Test_Object objParent2 = (Tree_Test_Object)Tree_InsertNode(objTree, NULL, TREE_INS_NEXT);
	if ( objParent2 ) {
		objParent2->idx = 4;
		sprintf(objParent2->Title, "New node idx = %d", objParent2->idx);
	}
	Tree_Test_Object objParent3 = (Tree_Test_Object)Tree_InsertNode(objTree, NULL, TREE_INS_NEXT);
	if ( objParent3 ) {
		objParent3->idx = 7;
		sprintf(objParent3->Title, "New node idx = %d", objParent3->idx);
	}
	Tree_Test_Object objNode = (Tree_Test_Object)Tree_InsertNode(objTree, (Tree_NodeBase*)objParent1, TREE_INS_LASTCHILD);
	if ( objNode ) {
		objNode->idx = 2;
		sprintf(objNode->Title, "Sub node idx = %d", objNode->idx);
	}
	objNode = (Tree_Test_Object)Tree_InsertNode(objTree, (Tree_NodeBase*)objParent1, TREE_INS_LASTCHILD);
	if ( objNode ) {
		objNode->idx = 3;
		sprintf(objNode->Title, "Sub node idx = %d", objNode->idx);
	}
	objNode = (Tree_Test_Object)Tree_InsertNode(objTree, (Tree_NodeBase*)objParent2, TREE_INS_LASTCHILD);
	if ( objNode ) {
		objNode->idx = 5;
		sprintf(objNode->Title, "Sub node idx = %d", objNode->idx);
	}
	objNode = (Tree_Test_Object)Tree_InsertNode(objTree, (Tree_NodeBase*)objParent2, TREE_INS_LASTCHILD);
	if ( objNode ) {
		objNode->idx = 6;
		sprintf(objNode->Title, "Sub node idx = %d", objNode->idx);
	}
	objNode = (Tree_Test_Object)Tree_InsertNode(objTree, (Tree_NodeBase*)objParent3, TREE_INS_LASTCHILD);
	if ( objNode ) {
		objNode->idx = 8;
		sprintf(objNode->Title, "Sub node idx = %d", objNode->idx);
	}
	objNode = (Tree_Test_Object)Tree_InsertNode(objTree, (Tree_NodeBase*)objParent3, TREE_INS_LASTCHILD);
	if ( objNode ) {
		objNode->idx = 9;
		sprintf(objNode->Title, "Sub node idx = %d", objNode->idx);
	}
	
	printf("\nTree state : \n");
	printf("\tCount : %d\t\t\t\t=> 9\n", objTree->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objTree->NodeMM.ItemLength, sizeof(Tree_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objTree->NodeMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objTree->NodeMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objTree->NodeMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objTree->NodeMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objTree->NodeMM.arrMMU.PageMMU.AllocStep);
	if ( objTree->NodeMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objTree->NodeMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objTree->NodeMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nTree nodes : \n");
	test_tree_eachnode(&objTree->Root, 1);
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	/*
	// subject 3 : insert first item
	printf("LList test subject 3 : insert first item\n\n");
	for ( int i = 0; i < 5; i++ ) {
		objItem = (LList_Test_Object)LList_InsertPrev(objLL, NULL);
		objItem->iVal = (i + 1) * 10;
		sprintf(objItem->sVal, "insert first idx : %d", objItem->iVal);
		printf("LList insert first item : %d (%s)\n", objItem->iVal, objItem->sVal);
	}
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 10\n", objLL->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objLL->objMM.ItemLength, sizeof(LList_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objLL->objMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objLL->objMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objLL->objMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocStep);
	if ( objLL->objMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objLL->objMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objLL->objMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	// subject 4 : insert prev item
	printf("LList test subject 4 : insert prev item\n\n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	for ( int i = 0; i < 3; i++ ) {
		objItem = (LList_Test_Object)objItem->Base.Next;
	}
	for ( int i = 0; i < 5; i++ ) {
		objItem = (LList_Test_Object)LList_InsertPrev(objLL, (void*)objItem);
		objItem->iVal = (i + 1) * 100;
		sprintf(objItem->sVal, "insert prev idx : %d", objItem->iVal);
		printf("LList insert prev item : %d (%s)\n", objItem->iVal, objItem->sVal);
	}
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 15\n", objLL->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objLL->objMM.ItemLength, sizeof(LList_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objLL->objMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objLL->objMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objLL->objMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocStep);
	if ( objLL->objMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objLL->objMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objLL->objMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	// subject 5 : insert next item
	printf("LList test subject 5 : insert next item\n\n");
	objItem = (LList_Test_Object)objLL->LastNode;
	for ( int i = 0; i < 3; i++ ) {
		objItem = (LList_Test_Object)objItem->Base.Prev;
	}
	for ( int i = 0; i < 5; i++ ) {
		objItem = (LList_Test_Object)LList_InsertNext(objLL, (void*)objItem);
		objItem->iVal = (i + 1) * 1000;
		sprintf(objItem->sVal, "insert next idx : %d", objItem->iVal);
		printf("LList insert next item : %d (%s)\n", objItem->iVal, objItem->sVal);
	}
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 20\n", objLL->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objLL->objMM.ItemLength, sizeof(LList_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objLL->objMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objLL->objMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objLL->objMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocStep);
	if ( objLL->objMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objLL->objMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objLL->objMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	// subject 6 : remove last item
	printf("LList test subject 6 : remove last item\n\n");
	for ( int i = 0; i < 2; i++ ) {
		LList_Remove(objLL, objLL->LastNode);
		printf("LList remove last node.\n");
	}
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 18\n", objLL->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objLL->objMM.ItemLength, sizeof(LList_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objLL->objMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objLL->objMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objLL->objMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocStep);
	if ( objLL->objMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objLL->objMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objLL->objMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	// subject 7 : remove first item
	printf("LList test subject 7 : remove first item\n\n");
	for ( int i = 0; i < 2; i++ ) {
		LList_Remove(objLL, objLL->FirstNode);
		printf("LList remove first node.\n");
	}
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 16\n", objLL->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objLL->objMM.ItemLength, sizeof(LList_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objLL->objMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objLL->objMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objLL->objMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocStep);
	if ( objLL->objMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objLL->objMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objLL->objMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	// subject 8 : remove item
	printf("LList test subject 8 : remove item\n\n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	for ( int i = 0; i < 6; i++ ) {
		objItem = (LList_Test_Object)objItem->Base.Next;
	}
	for ( int i = 0; i < 4; i++ ) {
		LList_Test_Object objNext = (LList_Test_Object)objItem->Base.Next;
		LList_Remove(objLL, (void*)objItem);
		objItem = objNext;
		printf("LList remove node.\n");
	}
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 12\n", objLL->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objLL->objMM.ItemLength, sizeof(LList_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objLL->objMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 1\n", objLL->objMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 1\n", objLL->objMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objLL->objMM.arrMMU.PageMMU.AllocStep);
	if ( objLL->objMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objLL->objMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objLL->objMM.arrMMU.PageMMU.Memory);
	}
	
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	printf("\n\n\n");
	system("pause");
	system("cls");
	*/
	
	
	// subject X : struct unit & destroy
	printf("Tree test subject X : struct unit & destroy\n\n");
	Tree_Unit(objTree);
	printf("Tree object (%p) already unit!\n\n", objTree);
	
	printf("\nLList state : \n");
	printf("\tCount : %d\t\t\t\t=> 0\n", objTree->Count);
	printf("\tMM.ItemLength : %d\t\t\t=> %d\n", objTree->NodeMM.ItemLength, sizeof(Tree_Test_Struct));
	printf("\tMM.arrMMU.ItemLength : %d\t\t=> %d\n", objTree->NodeMM.arrMMU.ItemLength, sizeof(MMU256_LLNode));
	printf("\tMM.arrMMU.Count : %d\t\t\t=> 0\n", objTree->NodeMM.arrMMU.Count);
	printf("\tMM.arrMMU.PageMMU.Count : %d\t\t=> 0\n", objTree->NodeMM.arrMMU.PageMMU.Count);
	printf("\tMM.arrMMU.PageMMU.AllocCount : %d\t=> 0\n", objTree->NodeMM.arrMMU.PageMMU.AllocCount);
	printf("\tMM.arrMMU.PageMMU.AllocStep : %d\t=> 64\n", objTree->NodeMM.arrMMU.PageMMU.AllocStep);
	if ( objTree->NodeMM.arrMMU.PageMMU.Memory ) {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tfail! ×\n", objTree->NodeMM.arrMMU.PageMMU.Memory);
	} else {
		printf("\tMM.arrMMU.PageMMU.Memory : %p\t\tpass! √\n", objTree->NodeMM.arrMMU.PageMMU.Memory);
	}
	/*
	printf("\nLList values (first to last) : \n");
	objItem = (LList_Test_Object)objLL->FirstNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Next;
		idx++;
	}
	
	printf("\nLList values (last to first) : \n");
	objItem = (LList_Test_Object)objLL->LastNode;
	idx = 1;
	printf("\tidx\tptr\t\t\tiVal\tsVal\n");
	while ( objItem ) {
		printf("\t%d\t%p\t%d\t%s\n", idx, objItem, objItem->iVal, objItem->sVal);
		objItem = (LList_Test_Object)objItem->Base.Prev;
		idx++;
	}
	*/
	Tree_Destroy(objTree);
	printf("\nLList object (%p) already destroyed!\n", objTree);
	
	printf("\n\n\n");
	system("pause");
	system("cls");
	
	
	
	MMU_Unit();
}


