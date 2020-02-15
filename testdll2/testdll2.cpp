// testdll2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

int main(int argc, char* argv[])
{
		char strHost[] = "192.168.128.1";//"127.0.0.1";          //声明上线地址
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
	printf("Hello World!\n");
	return 0;
}

