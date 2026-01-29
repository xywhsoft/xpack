


#include "../xCore/xCore.h"
#include "xFile.h"



int main(int argc, char** argv)
{
	xCoreInit();
	//xFile_CopyDirW(xCore_A2W("D:\\Users\\fuq\\Desktop\\华夏基金交易所指令机器人\\res\\输入数据\\", 0), xCore_A2W("C:\\uibot\\恒生O32机器人\\输入数据\\", 0), L"*", TRUE);
	printf("%d\n", xFile_PathExistsA("c:\\sdasdsad"));
	printf("%d\n", xFile_PathExistsA("c:\\users"));
	printf("%d\n", xFile_PathExistsA("c:\\12.json"));
	printf("%d\n", xFile_PathExistsA("c:\\1.json"));
	printf("%d\n", xFile_ExistsA("c:\\sdasdsad"));
	printf("%d\n", xFile_ExistsA("c:\\users"));
	printf("%d\n", xFile_ExistsA("c:\\12.json"));
	printf("%d\n", xFile_ExistsA("c:\\1.json"));
	printf("%d\n", xFile_FolderExistsA("c:\\sdasdsad"));
	printf("%d\n", xFile_FolderExistsA("c:\\users"));
	printf("%d\n", xFile_FolderExistsA("c:\\12.json"));
	printf("%d\n", xFile_FolderExistsA("c:\\1.json"));
	return 0;
}


