// CFileManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "PCRemote.h"
#include "CFileManagerDlg.h"
#include "afxdialogex.h"


// CFileManagerDlg 对话框

IMPLEMENT_DYNAMIC(CFileManagerDlg, CDialog)

CFileManagerDlg::CFileManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_FILE, pParent)
{

}

CFileManagerDlg::~CFileManagerDlg()
{
}

void CFileManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFileManagerDlg, CDialog)
END_MESSAGE_MAP()


// CFileManagerDlg 消息处理程序
