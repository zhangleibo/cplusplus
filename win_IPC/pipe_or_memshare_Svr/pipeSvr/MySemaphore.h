#pragma once

#include <wtypes.h>

class CSemaphore
{
public:
	CSemaphore() : m_sem(NULL)
	{

	}

	~CSemaphore()
	{
		if (NULL != m_sem)
		{
			::CloseHandle(m_sem);
		}
	}

	bool CreateSem(long lInitCnt, long lMaxCnt, const char* pName = NULL)
	{
		m_sem = ::CreateSemaphoreA(NULL, lInitCnt, lMaxCnt, pName);

		return NULL != m_sem;
	}

	DWORD Wait(DWORD dwMillisec = INFINITE)
	{
		return ::WaitForSingleObject(m_sem, dwMillisec);
	}

	bool Notify()
	{
		return TRUE == ::ReleaseSemaphore(m_sem, 1, NULL);
	}
private:
	HANDLE m_sem;
};
