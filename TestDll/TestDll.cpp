// TestDll.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<Windows.h>

int main()
{

	char strHost[] = "127.0.0.1";  //"192.168.128.129";//     //声明上线地址


	int  nPort = 8888;                     //声明上线端口
	//载入服务端dll
	HMODULE hServerDll = LoadLibrary("MainDll.dll");
	//声明导出函数类型--查看上一节导出的TestRun函数
	typedef void(_cdecl* TestRunT)(char* strHost, int nPort);
	//寻找dll中导出函数
	TestRunT pTestRunT = (TestRunT)GetProcAddress(hServerDll, "TestRun");
	//判断函数是否为空
	if (pTestRunT != NULL)
	{
		pTestRunT(strHost, nPort);   //调用这个函数
	}

	//system("pause");

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
