// SystemManager.h: interface for the CSystemManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEMMANAGER_H__26C71561_C37D_44F2_B69C_DAF907C04CBE__INCLUDED_)
#define AFX_SYSTEMMANAGER_H__26C71561_C37D_44F2_B69C_DAF907C04CBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"

class CSystemManager : public CManager  
{
public:
	BOOL m_bHow;
	CSystemManager(CClientSocket *pClient, BOOL bHow);
	virtual ~CSystemManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

	static bool DebugPrivilege(const char *PName,BOOL bEnable);
	static BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam);
	static void ShutdownWindows(DWORD dwReason);
private:
	LPBYTE getProcessList();
	LPBYTE getWindowsList();
	void SendProcessList();
	void SendWindowsList();
	void SendDialupassList();
	void KillProcess(LPBYTE lpBuffer, UINT nSize);
public:
	void CloseWindow(LPBYTE buf);
	void TestWindow(LPBYTE buf);
	
};

#endif // !defined(AFX_SYSTEMMANAGER_H__26C71561_C37D_44F2_B69C_DAF907C04CBE__INCLUDED_)
