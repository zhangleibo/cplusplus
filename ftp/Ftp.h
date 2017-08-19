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

// FTP���ò���
typedef struct tagFtpParam
{
	int iFtpEnable;             // 1����FTP�ϴ���0�ǹر�FTP�ϴ�
	TCHAR szFtpServerIp[32];    // FTP������IP
	TCHAR szFtpUser[20];        // FTP��½�˺�
	TCHAR szFtpPass[30];        // FTP��½����
	int iFtpPort;               // FTP����˿�, Ĭ��Ϊ21
	int iFtpIsPassive;          // 1��ʾFTPΪ����ģʽ��0��ʾFTPΪ����ģʽ
	TCHAR szFtpUploadDir[30];   // FTP�ϴ�Ŀ¼���� /test

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
	@brief	 ��FTP�ϴ������ļ�
	@param[in]        LPCTSTR pszFilePath,	�����ļ�·��
	@param[in]        LPCTSTR pszUploadDir,	�ϴ�����FTPĿ¼����б��/��ͷ���� /result/2014/10/22
	@return           �ɹ�true,ʧ��false
	*/
	bool UploadDiskFile(LPCTSTR pszFilePath, LPCTSTR pszUploadDir);



	/**
	@name    UploadMemoryFile
	@brief	 ��FTP�ϴ��ڴ��ļ�
	@param[in]        PBYTE pbFile,			�ڴ��ļ�������ָ��
	@param[in]        int iFileLen,			�ڴ��ļ�����
	@param[in]        LPCTSTR pszFileName,	�����ļ�����
	@param[in]        LPCTSTR pszUploadDir, �ϴ�����FTPĿ¼����б�ܿ�ͷ����/result/2014/10/22
	@return           bool
	*/
	bool UploadMemoryFile(PBYTE pbFile, int iFileLen, LPCTSTR pszFileName, LPCTSTR pszUploadDir);

	/**
	@name    IsFileExists
	@brief   �ж��ļ��Ƿ���� 
	@param[in]:        LPCTSTR pszFilePath  �ļ�·��
	@return            �ļ����ڷ���true, ���򷵻�false
	*/
	bool IsFileExists(LPCTSTR pszFilePath);


public:

	FtpParam m_ftpParam;	// ftp���ò���

private:

	/**
	@name    ConnFtpServer
	@brief	 ����FTP������
	@param[in]        void
	@return   �ɹ�true,ʧ��false
	*/
	bool ConnFtpServer(void);

	// ����FTP�̴߳���
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
	@brief	 ����FTP�߳�
	@param[in]        void
	@return           bool
	*/
	bool ThreadConnFtp(void);

private:
	HINTERNET m_hInetOpen;	// ����򿪾��
	HINTERNET m_hInetConn;	// �������Ӿ��
	HANDLE m_hConnThread;	// �����߳̾��
};
