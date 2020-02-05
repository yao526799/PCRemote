
// PCRemoteDlg.h: 头文件
//

#pragma once
#include "TrueColorToolBar.h"
#include "include/IOCPServer.h"


// CPCRemoteDlg 对话框
class CPCRemoteDlg : public CDialogEx
{
// 构造
public:
	CPCRemoteDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PCREMOTE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	int iCount;
	CStatusBar  m_wndStatusBar;              //状态条
	CTrueColorToolBar m_ToolBar;
	NOTIFYICONDATA nid;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CList_Online;
	CListCtrl m_CList_Message;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	int InitList();
	// //add to online list
	void AddList(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing);
	// add msg to msglist
	void ShowMessage(bool bIsOK, CString strMsg);
	void Activate(UINT nPort, UINT nMaxConnections);
	//重要的回调函数
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	void test();

	afx_msg void OnNMRClickOnline(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOnlineAudio();
	afx_msg void OnOnlineCmd();
	afx_msg void OnOnlineDesktop();
	afx_msg void OnOnlineFile();
	afx_msg void OnOnlineProcess();
	afx_msg void OnOnlineServer();
	afx_msg void OnOnlineVideo();
	afx_msg void OnOnlineWindow();
	afx_msg void OnOnlineDelete();
	afx_msg void OnMainClose();
	afx_msg void OnMainAbout();
	//自定义消息
	afx_msg void OnIconNotify(WPARAM wParam, LPARAM lParam);
	void CreatStatusBar();
	void CreateToolBar();
	afx_msg void OnNotifyClose();
	afx_msg void OnNotifyShow();
//	afx_msg void OnClose();
	afx_msg void OnDestroy();
	void ListenPort();
	afx_msg void OnMainSet();
};
