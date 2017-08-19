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
	@brief	 连接FTP服务器
	@param[in]        void
	@return   成功true,失败false
	*/
bool CFtp::ConnFtpServer(void)
{

	WaitForSingleObject(m_hConnThread, 1000*60);
	m_hConnThread = (HANDLE)_beginthreadex(NULL, 0, ThreadConnFtpProxy, this, 0, NULL);
	DWORD   dwTimeout = 1000*3;	// 连接FTP超时时间
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
	@brief	 往FTP上传磁盘文件
	@param[in]        LPCTSTR pszFilePath,	磁盘文件路径
	@param[in]        LPCTSTR pszUploadDir,	上传到的FTP目录，以斜杠/开头，如 /test
	@return           成功true,失败false
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

	// 解析上传目录
	tstring strUploadDir;
	if (_tcslen(pszUploadDir) > 0)
	{
		strUploadDir = pszUploadDir;
	}
	else    
	{
		strUploadDir = m_ftpParam.szFtpUploadDir;    // 默认上传目录
	}
	if (strUploadDir.size() <= 0)
	{
		strUploadDir = _T("/result/");
	}
	strUploadDir[0] = _T('/');		// 确保以斜杠/开头
	if (strUploadDir[strUploadDir.size() - 1] != _T('/'))
	{
		strUploadDir += _T('/');	// 确保以斜杠/结尾
	}
	
	
	// 确保上传目录存在 strUploadDir = /test
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

	// 解析上传磁盘文件名称,如 D:\test.txt
	tstring strDiskFilePath = pszFilePath;
	tstring strDiskFileName;
	tstring::size_type iIndex = strDiskFilePath.rfind('\\');
	if (iIndex != tstring::npos && iIndex >= 0)
	{
		strDiskFileName = strDiskFilePath.substr(iIndex+1, strDiskFilePath.size() - 1);
	}

	// 上传磁盘文件
	tstring strTmpFile = strUploadDir;
	strTmpFile += strDiskFileName;
	bRet = FtpPutFile(m_hInetConn, pszFilePath, strTmpFile.c_str(), FTP_TRANSFER_TYPE_BINARY, NULL) > 0 ? true : false;

	// 获取错误信息
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
	@brief	 往FTP上传内存文件
	@param[in]        PBYTE pbFile,			内存文件缓冲区指针
	@param[in]        int iFileLen,			内存文件长度
	@param[in]        LPCTSTR pszFileName,	保存文件名称
	@param[in]        LPCTSTR pszUploadDir, 上传到的FTP目录，以斜杠开头，如/result/2014/10/22
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

	// 解析上传目录
	tstring strUploadDir;
	if (_tcslen(pszUploadDir) > 0)
	{
		strUploadDir = pszUploadDir;
	}
	else    
	{
		strUploadDir = m_ftpParam.szFtpUploadDir;    // 默认上传目录
	}
	if (strUploadDir.size() <= 0)
	{
		strUploadDir = _T("/result/");
	}
	strUploadDir[0] = _T('/');					        // 确保以斜杠/开头
	if (strUploadDir[strUploadDir.size() - 1] != _T('/'))
	{
		strUploadDir += _T('/');	// 确保以斜杠/结尾
	}

	// 确保上传目录存在 strUploadDir = /test 
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

	// 解析上传内存文件名称
	tstring strMemoryFileName = pszFileName;

	// 上传内存文件
	HINTERNET hFtpFile = NULL;
	tstring strTmpFile = strUploadDir;
	strTmpFile += strMemoryFileName;
	hFtpFile = FtpOpenFile(m_hInetConn, strTmpFile.c_str(), GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY, NULL);
	if (hFtpFile != NULL)
	{
		DWORD dwWrite = 0;
		bRet = InternetWriteFile(hFtpFile, pbFile, iFileLen, &dwWrite) > 0 ? true : false;
	}

	// 获取错误信息
	if (!bRet)
	{
		DWORD iErrorCode = GetLastError();
		TCHAR szErr[1024] = {0};
		DWORD dwLen = 1024;
		InternetGetLastResponseInfo(&iErrorCode, szErr, &dwLen);
		//OutputDebugString(szErr);
	}

	// 释放内存
	if (hFtpFile != NULL)
	{
		InternetCloseHandle(hFtpFile);
		hFtpFile = NULL;
	}

	return bRet;
}


/**
	@name    IsFileExists
	@brief   判断文件是否存在 
	@param[in]:        LPCTSTR pszFilePath  文件路径
	@return            文件存在返回true, 否则返回false
	*/
bool CFtp::IsFileExists(LPCTSTR pszFilePath)
{
	if (pszFilePath == NULL)
	{
		return false;
	}
	HANDLE hRet = CreateFile(pszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	bool bRet = false;
	if (hRet != INVALID_HANDLE_VALUE )   // 文件存在 
	{
		bRet = true;
	}
	CloseHandle(hRet);
	return bRet;
}

/**
	@name    ThreadConnFtp
	@brief	 连接FTP线程
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

	// 连接FTP 服务器
	//HINTERNET hInetOpen = InternetOpen(_T("CN"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, INTERNET_FLAG_ASYNC);  // 异步
	m_hInetOpen = InternetOpen(_T("CN"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);  // 同步 
	if (m_hInetOpen == NULL)
	{
		return false;
	}

	DWORD dwFlags = 0;
	if(m_ftpParam.iFtpIsPassive == 1)
	{
		dwFlags = INTERNET_FLAG_PASSIVE;
	}
	//DWORD dwTimeOut = 1000*3;	// 超时值，经测试在WIN7有效，在XP无效,现已改成由线程控制
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
