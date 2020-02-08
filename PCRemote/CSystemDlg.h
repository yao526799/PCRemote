#pragma once


// CSystemDlg 对话框

class CSystemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSystemDlg)

public:
	CSystemDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);    // 标准构造函数
	virtual ~CSystemDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:

	BOOL m_bHow;
	CListCtrl m_list_process;
	
private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
public:
	virtual BOOL OnInitDialog();

	
public:
	void OnReceiveComplete();
	void AdjustList();
private:
	void ShowProcessList();

	void GetProcessList();
	void GetWindowsList();
	void GetDialupassList();
	void ShowWindowsList();
	
public:

	afx_msg void OnKillprocess();
	afx_msg void OnRefreshPslist();
	afx_msg void OnNMRClickListProcess(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnClose();
	afx_msg void OnWindowReturn();
	afx_msg void OnWindowMin();
	afx_msg void OnWindowMax();
	afx_msg void OnWindowHide();
	afx_msg void OnWindowReflush();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
