#include <Windows.h>  
#include <iostream>  
using namespace std;

/*
int main()
{
	HANDLE hMutex = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpShareMemory = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;

	//create share memory  
	hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		1024 * 1024,
		L"ShareMemoryTest");
	if (NULL == hFileMapping)
	{
		cout << "CreateFileMapping fail:" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	lpShareMemory = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,      //memory start address  
		0);     //all memory space  
	if (NULL == lpShareMemory)
	{
		cout << "MapViewOfFile" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	//373  
	hMutex = CreateMutex(NULL, FALSE, L"SM_Mutex");
	if (NULL == hMutex || ERROR_ALREADY_EXISTS == GetLastError())
	{
		cout << "CreateMutex" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}//多个线程互斥访问  

	//send data  
	hServerWriteOver = CreateEvent(NULL,
		TRUE,
		FALSE,
		L"ServerWriteOver");
	hClientReadOver = CreateEvent(NULL,
		TRUE,
		FALSE,
		L"ClientReadOver");
	if (NULL == hServerWriteOver ||
		NULL == hClientReadOver)
	{
		cout << "CreateEvent" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	char p = 0;
	char* q = (char*)lpShareMemory;
	do
	{
		p = getchar();
		if (WaitForSingleObject(hClientReadOver, 5 * 1000) != WAIT_OBJECT_0)
			goto SERVER_SHARE_MEMORY_END;
		q[0] = p;
		if (!ResetEvent(hClientReadOver)) goto SERVER_SHARE_MEMORY_END;//把指定的事件对象设置为无信号状态  
		if (!SetEvent(hServerWriteOver)) goto SERVER_SHARE_MEMORY_END;//把指定的事件对象设置为有信号状态  
	} while (p != '\n');

SERVER_SHARE_MEMORY_END:
	//release share memory  
	if (NULL != hServerWriteOver)   CloseHandle(hServerWriteOver);
	if (NULL != hClientReadOver)    CloseHandle(hClientReadOver);
	if (NULL != lpShareMemory)      UnmapViewOfFile(lpShareMemory);
	if (NULL != hFileMapping)       CloseHandle(hFileMapping);
	if (NULL != hMutex)             ReleaseMutex(hMutex);
	return 0;
}
*/

#include "AutoMutex.h"
#include "MySemaphore.h"
const TCHAR g_MapMemName[] = L"Global\\NameSpaceMapShare";
const TCHAR g_MutexName[] = L"Global\\MutexSpaceName";
const char g_WriterName[] = "Global\\WriterName";
const char g_ReaderName[] = "Global\\ReaderName";

#define MAX_SIZE_MAP 1024 * 1024
typedef void(*CallBackFun)(const char*);

class CMemSvr
{
public:
	CMemSvr() : m_hMap(NULL), m_pData(NULL), m_mutex(FALSE, g_MutexName)
	{

	}

	~CMemSvr()
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

	bool Create()
	{
		bool bOk = false;
		do
		{
			//create share memory  
			m_hMap = CreateFileMapping(INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0,
				MAX_SIZE_MAP,
				g_MapMemName);
			if (NULL == m_hMap)
			{
				break;
			}

			m_pData = (char*)MapViewOfFile(m_hMap,
				FILE_MAP_ALL_ACCESS,
				0,
				0,      //memory start address  
				0);     //all memory space  
			if (NULL == m_pData)
			{
				break;
			}

			bOk = true;

		} while (0);

		return bOk;
	}

	bool Run(CallBackFun fun)
	{
		if (NULL == fun/* || !Create()*/)
		{
			return false;
		}

		//创建读写信号量
		if (!m_Writer.CreateSem(0, 1, g_WriterName) || 
			!m_Reader.CreateSem(0, 1, g_ReaderName))
		{
			return false;
		}

		do
		{
			m_Writer.Notify();

			//多进程间同步
			if (WAIT_FAILED == m_Reader.Wait())
			{
				break;
			}

			//互斥访问
			{
				CLock lock(m_mutex);
				fun(m_pData);
			}

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