#include "Ftp.h"

CFtp::CFtp(TCHAR ip[], TCHAR user[], TCHAR passWord[]) : m_ftpParam(ip, user, passWord)
{
	m_hInetOpen = NULL;
	m_hInetConn = NULL;
	m_hConnThread = NULL;
}

CFtp::~CFtp(void)
{
	if (m_hInetOpen != NULL)
	{
		InternetCloseHandle(m_hInetOpen);
		m_hInetOpen = NULL;
	}
	if (m_hInetConn != NULL)
	{
		InternetCloseHandle(m_hInetConn);
		m_hInetConn = NULL;
	}

	if (m_hConnThread != NULL)
	{
		WaitForSingleObject (m_hConnThread, 1000*30);
		CloseHandle(m_hConnThread);
		m_hConnThread = NULL;
	}
}

/**
	@name    ConnFtpServer
	@brief	 ����FTP������
	@param[in]        void
	@return   �ɹ�true,ʧ��false
	*/
bool CFtp::ConnFtpServer(void)
{

	WaitForSingleObject(m_hConnThread, 1000*60);
	m_hConnThread = (HANDLE)_beginthreadex(NULL, 0, ThreadConnFtpProxy, this, 0, NULL);
	DWORD   dwTimeout = 1000*3;	// ����FTP��ʱʱ��
	if (WaitForSingleObject(m_hConnThread, dwTimeout ) == WAIT_TIMEOUT)
	{
		TerminateThread(m_hConnThread, -1);
		if (m_hConnThread != NULL)
		{
			CloseHandle(m_hConnThread);
			m_hConnThread = NULL;
		}
		return false;
	}

	if (m_hConnThread != NULL)
	{
		CloseHandle(m_hConnThread);
		m_hConnThread = NULL;
	}
	return true;
}

/**
	@name    UploadDiskFile
	@brief	 ��FTP�ϴ������ļ�
	@param[in]        LPCTSTR pszFilePath,	�����ļ�·��
	@param[in]        LPCTSTR pszUploadDir,	�ϴ�����FTPĿ¼����б��/��ͷ���� /test
	@return           �ɹ�true,ʧ��false
	*/
bool CFtp::UploadDiskFile(LPCTSTR pszFilePath, LPCTSTR pszUploadDir)
{
	if (pszFilePath == NULL || m_ftpParam.iFtpEnable == 0)
	{
		return false;
	}
	if (!IsFileExists(pszFilePath))
	{
		return false;
	}
	if (!ConnFtpServer())
	{
		return false;
	}

	// �����ϴ�Ŀ¼
	tstring strUploadDir;
	if (_tcslen(pszUploadDir) > 0)
	{
		strUploadDir = pszUploadDir;
	}
	else    
	{
		strUploadDir = m_ftpParam.szFtpUploadDir;    // Ĭ���ϴ�Ŀ¼
	}
	if (strUploadDir.size() <= 0)
	{
		strUploadDir = _T("/result/");
	}
	strUploadDir[0] = _T('/');		// ȷ����б��/��ͷ
	if (strUploadDir[strUploadDir.size() - 1] != _T('/'))
	{
		strUploadDir += _T('/');	// ȷ����б��/��β
	}
	
	
	// ȷ���ϴ�Ŀ¼���� strUploadDir = /test
	tstring strCurDir = _T("/");
	FtpSetCurrentDirectory(m_hInetConn, strCurDir.c_str());
	tstring strTmpDir = strUploadDir;
	bool bRet = true;
	while(strTmpDir.find(_T('/')) != tstring::npos && bRet)
	{
		size_t iStart = strTmpDir.find(_T('/'));
		strTmpDir.erase(iStart, 1);
		size_t iEnd = strTmpDir.find(_T('/'));
		tstring strDir = strTmpDir.substr(iStart, iEnd - iStart);
		//bRet = FtpCreateDirectory(m_hInetConn, strDir.c_str()) > 0 ? true : false;
		FtpCreateDirectory(m_hInetConn, strDir.c_str()) > 0 ? true : false;
		strCurDir += strDir;
		strCurDir += _T('/');
		FtpSetCurrentDirectory(m_hInetConn, strCurDir.c_str());
		if (iEnd == strTmpDir.size() - 1)
		{
			break;
		}
	}
	FtpSetCurrentDirectory(m_hInetConn, strUploadDir.c_str());

	// �����ϴ������ļ�����,�� D:\test.txt
	tstring strDiskFilePath = pszFilePath;
	tstring strDiskFileName;
	tstring::size_type iIndex = strDiskFilePath.rfind('\\');
	if (iIndex != tstring::npos && iIndex >= 0)
	{
		strDiskFileName = strDiskFilePath.substr(iIndex+1, strDiskFilePath.size() - 1);
	}

	// �ϴ������ļ�
	tstring strTmpFile = strUploadDir;
	strTmpFile += strDiskFileName;
	bRet = FtpPutFile(m_hInetConn, pszFilePath, strTmpFile.c_str(), FTP_TRANSFER_TYPE_BINARY, NULL) > 0 ? true : false;

	// ��ȡ������Ϣ
	if (!bRet)
	{
		DWORD iErrorCode = GetLastError();
		TCHAR szErr[1024] = {0};
		DWORD dwLen = 1024;
		InternetGetLastResponseInfo(&iErrorCode, szErr, &dwLen);
		//OutputDebugString(szErr);
	}

	return bRet;
}


/**
	@name    UploadMemoryFile
	@brief	 ��FTP�ϴ��ڴ��ļ�
	@param[in]        PBYTE pbFile,			�ڴ��ļ�������ָ��
	@param[in]        int iFileLen,			�ڴ��ļ�����
	@param[in]        LPCTSTR pszFileName,	�����ļ�����
	@param[in]        LPCTSTR pszUploadDir, �ϴ�����FTPĿ¼����б�ܿ�ͷ����/result/2014/10/22
	@return           bool
	*/
bool CFtp::UploadMemoryFile(PBYTE pbFile, int iFileLen, LPCTSTR pszFileName, LPCTSTR pszUploadDir)
{
	if (pbFile == NULL || iFileLen < 1 || m_ftpParam.iFtpEnable == 0)
	{
		return false;
	}
	if (pszFileName == NULL)
	{
		return false;
	}
	if (!ConnFtpServer())
	{
		return false;
	}

	// �����ϴ�Ŀ¼
	tstring strUploadDir;
	if (_tcslen(pszUploadDir) > 0)
	{
		strUploadDir = pszUploadDir;
	}
	else    
	{
		strUploadDir = m_ftpParam.szFtpUploadDir;    // Ĭ���ϴ�Ŀ¼
	}
	if (strUploadDir.size() <= 0)
	{
		strUploadDir = _T("/result/");
	}
	strUploadDir[0] = _T('/');					        // ȷ����б��/��ͷ
	if (strUploadDir[strUploadDir.size() - 1] != _T('/'))
	{
		strUploadDir += _T('/');	// ȷ����б��/��β
	}

	// ȷ���ϴ�Ŀ¼���� strUploadDir = /test 
	tstring strCurDir = _T("/");
	FtpSetCurrentDirectory(m_hInetConn, strCurDir.c_str());
	tstring strTmpDir = strUploadDir;
	bool bRet = true;
	while(strTmpDir.find(_T('/')) != tstring::npos && bRet)
	{
		size_t iStart = strTmpDir.find(_T('/'));
		strTmpDir.erase(iStart, 1);
		size_t iEnd = strTmpDir.find(_T('/'));
		tstring strDir = strTmpDir.substr(iStart, iEnd - iStart);
		bRet = FtpCreateDirectory(m_hInetConn, strDir.c_str()) > 0 ? true : false;
		strCurDir += strDir;
		strCurDir += _T('/');
		FtpSetCurrentDirectory(m_hInetConn, strCurDir.c_str());
		if (iEnd == strTmpDir.size() - 1)
		{
			break;
		}
	}
	FtpSetCurrentDirectory(m_hInetConn, strUploadDir.c_str());

	// �����ϴ��ڴ��ļ�����
	tstring strMemoryFileName = pszFileName;

	// �ϴ��ڴ��ļ�
	HINTERNET hFtpFile = NULL;
	tstring strTmpFile = strUploadDir;
	strTmpFile += strMemoryFileName;
	hFtpFile = FtpOpenFile(m_hInetConn, strTmpFile.c_str(), GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY, NULL);
	if (hFtpFile != NULL)
	{
		DWORD dwWrite = 0;
		bRet = InternetWriteFile(hFtpFile, pbFile, iFileLen, &dwWrite) > 0 ? true : false;
	}

	// ��ȡ������Ϣ
	if (!bRet)
	{
		DWORD iErrorCode = GetLastError();
		TCHAR szErr[1024] = {0};
		DWORD dwLen = 1024;
		InternetGetLastResponseInfo(&iErrorCode, szErr, &dwLen);
		//OutputDebugString(szErr);
	}

	// �ͷ��ڴ�
	if (hFtpFile != NULL)
	{
		InternetCloseHandle(hFtpFile);
		hFtpFile = NULL;
	}

	return bRet;
}


/**
	@name    IsFileExists
	@brief   �ж��ļ��Ƿ���� 
	@param[in]:        LPCTSTR pszFilePath  �ļ�·��
	@return            �ļ����ڷ���true, ���򷵻�false
	*/
bool CFtp::IsFileExists(LPCTSTR pszFilePath)
{
	if (pszFilePath == NULL)
	{
		return false;
	}
	HANDLE hRet = CreateFile(pszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	bool bRet = false;
	if (hRet != INVALID_HANDLE_VALUE )   // �ļ����� 
	{
		bRet = true;
	}
	CloseHandle(hRet);
	return bRet;
}

/**
	@name    ThreadConnFtp
	@brief	 ����FTP�߳�
	@param[in]        void
	@return           bool
	*/
bool CFtp::ThreadConnFtp(void)
{
	if (m_hInetOpen != NULL)
	{
		InternetCloseHandle(m_hInetOpen);
		m_hInetOpen = NULL;
	}
	if (m_hInetConn != NULL)
	{
		InternetCloseHandle(m_hInetConn);
		m_hInetConn = NULL;
	}

	// ����FTP ������
	//HINTERNET hInetOpen = InternetOpen(_T("CN"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, INTERNET_FLAG_ASYNC);  // �첽
	m_hInetOpen = InternetOpen(_T("CN"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);  // ͬ�� 
	if (m_hInetOpen == NULL)
	{
		return false;
	}

	DWORD dwFlags = 0;
	if(m_ftpParam.iFtpIsPassive == 1)
	{
		dwFlags = INTERNET_FLAG_PASSIVE;
	}
	//DWORD dwTimeOut = 1000*3;	// ��ʱֵ����������WIN7��Ч����XP��Ч,���Ѹĳ����߳̿���
	//InternetSetOption(m_hInetOpen, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));
	m_hInetConn = InternetConnect(m_hInetOpen, 
									m_ftpParam.szFtpServerIp, 
									m_ftpParam.iFtpPort,
									m_ftpParam.szFtpUser,
									m_ftpParam.szFtpPass,
									INTERNET_SERVICE_FTP,
									dwFlags,
									NULL);

	if (m_hInetConn == NULL)
	{
		return false;
	}
	return true;
}
