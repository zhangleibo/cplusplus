#include <Windows.h>  
#include <iostream>  
using namespace std;

#include "AutoMutex.h"
#include "MySemaphore.h"
const TCHAR g_MapMemName[] = L"Global\\NameSpaceMapShare";
const TCHAR g_MutexName[] = L"Global\\MutexSpaceName";
const char g_WriterName[] = "Global\\WriterName";
const char g_ReaderName[] = "Global\\ReaderName";

#define MAX_SIZE_MAP 1024 * 1024

class CMemClient
{
public:
	CMemClient() : m_hMap(NULL), m_pData(NULL), m_mutex(FALSE, g_MutexName)
	{

	}

	~CMemClient()
	{
		if (NULL == m_pData)
		{
			UnmapViewOfFile(m_pData);
			m_pData = NULL;
		}
		if (NULL == m_hMap)
		{
			::CloseHandle(m_hMap);
			m_hMap = NULL;
		}
	}

	bool Open()
	{
		bool bOk = false;
		do
		{
			//open share memory  
			m_hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS,
				FALSE,
				g_MapMemName);
			if (NULL == m_hMap)
			{
				break;
			}

			m_pData = (char*)MapViewOfFile(m_hMap,
				FILE_MAP_ALL_ACCESS,
				0,
				0,
				0);
			if (NULL == m_pData)
			{
				break;
			}

			bOk = true;

		} while (0);

		return bOk;
	}

	bool WriteData(char* pData)
	{
		if (NULL == pData || !Open())
		{
			return false;
		}

		//创建读写信号量
		if (!m_Writer.CreateSem(0, 1, g_WriterName) ||
			!m_Reader.CreateSem(0, 1, g_ReaderName))
		{
			return false;
		}

		m_Reader.Notify();
		do
		{
			//多进程间同步
			DWORD dwRet = m_Writer.Wait(3 * 1000);
			if (WAIT_FAILED == dwRet)
			{
				break;
			}
			else if (WAIT_TIMEOUT == dwRet)
			{
				break;
			}

			//互斥访问
			{
				CLock lock(m_mutex);
				memcpy_s(m_pData, strlen(pData), pData, strlen(pData));
			}

			printf(pData);
			m_Reader.Notify();

		} while (true);

		return true;
	}
private:
	HANDLE m_hMap;
	char* m_pData;
	CMutex m_mutex;
	CSemaphore m_Reader;
	CSemaphore m_Writer;
};
