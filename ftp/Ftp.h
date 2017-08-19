#pragma once
#pragma warning(disable:4995)

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <process.h>
#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")

#include <vector>
#include <string>
using std::vector;
using std::string;
using std::wstring;

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

// FTP配置参数
typedef struct tagFtpParam
{
	int iFtpEnable;             // 1开启FTP上传，0是关闭FTP上传
	TCHAR szFtpServerIp[32];    // FTP服务器IP
	TCHAR szFtpUser[20];        // FTP登陆账号
	TCHAR szFtpPass[30];        // FTP登陆密码
	int iFtpPort;               // FTP服务端口, 默认为21
	int iFtpIsPassive;          // 1表示FTP为被动模式，0表示FTP为主动模式
	TCHAR szFtpUploadDir[30];   // FTP上传目录，如 /test

	tagFtpParam(TCHAR ip[], TCHAR user[], TCHAR passWord[])
	{
		iFtpEnable = 1;
		ZeroMemory(szFtpServerIp, sizeof(szFtpServerIp));
		ZeroMemory(szFtpUser, sizeof(szFtpUser));
		ZeroMemory(szFtpPass, sizeof(szFtpPass));
		iFtpPort = 21;
		iFtpIsPassive = 1;
		ZeroMemory(szFtpUploadDir, sizeof(szFtpUploadDir));

		StringCbCopy(szFtpServerIp, sizeof(szFtpServerIp), ip);
		StringCbCopy(szFtpUser, sizeof(szFtpUser), user);
		StringCbCopy(szFtpPass, sizeof(szFtpPass), passWord);
		StringCbCopy(szFtpUploadDir, sizeof(szFtpUploadDir), _T("/log"));
	}

}FtpParam;


class CFtp
{
public:

	CFtp(TCHAR ip[], TCHAR user[], TCHAR passWord[]);

	~CFtp(void);


	/**
	@name    UploadDiskFile
	@brief	 往FTP上传磁盘文件
	@param[in]        LPCTSTR pszFilePath,	磁盘文件路径
	@param[in]        LPCTSTR pszUploadDir,	上传到的FTP目录，以斜杠/开头，如 /result/2014/10/22
	@return           成功true,失败false
	*/
	bool UploadDiskFile(LPCTSTR pszFilePath, LPCTSTR pszUploadDir);



	/**
	@name    UploadMemoryFile
	@brief	 往FTP上传内存文件
	@param[in]        PBYTE pbFile,			内存文件缓冲区指针
	@param[in]        int iFileLen,			内存文件长度
	@param[in]        LPCTSTR pszFileName,	保存文件名称
	@param[in]        LPCTSTR pszUploadDir, 上传到的FTP目录，以斜杠开头，如/result/2014/10/22
	@return           bool
	*/
	bool UploadMemoryFile(PBYTE pbFile, int iFileLen, LPCTSTR pszFileName, LPCTSTR pszUploadDir);

	/**
	@name    IsFileExists
	@brief   判断文件是否存在 
	@param[in]:        LPCTSTR pszFilePath  文件路径
	@return            文件存在返回true, 否则返回false
	*/
	bool IsFileExists(LPCTSTR pszFilePath);


public:

	FtpParam m_ftpParam;	// ftp配置参数

private:

	/**
	@name    ConnFtpServer
	@brief	 连接FTP服务器
	@param[in]        void
	@return   成功true,失败false
	*/
	bool ConnFtpServer(void);

	// 连接FTP线程代理
	static unsigned int _stdcall ThreadConnFtpProxy(PVOID pParam)
	{
		CFtp *pThis = (CFtp*)pParam;
		if (pThis != NULL)
		{
			pThis->ThreadConnFtp();
		}
		return 0;
	}


	/**
	@name    ThreadConnFtp
	@brief	 连接FTP线程
	@param[in]        void
	@return           bool
	*/
	bool ThreadConnFtp(void);

private:
	HINTERNET m_hInetOpen;	// 网络打开句柄
	HINTERNET m_hInetConn;	// 网络连接句柄
	HANDLE m_hConnThread;	// 连接线程句柄
};
