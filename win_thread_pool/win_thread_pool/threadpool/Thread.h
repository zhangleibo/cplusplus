#pragma once
#include <windows.h>
#include <process.h>
#include "NonCopyable.h"

class IThread : CNonCopyable
{
public:
    IThread() : m_hThread(NULL)
    {

    }

    virtual ~IThread()
    {
        Release();
    }

    void Release()
    {
        if (m_hThread)
        {
            ::CloseHandle(m_hThread);
			m_hThread = NULL;
        }
    }

    void Start()
    {
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, Proc, (void*)this, 0, NULL);
    }

    void Stop()
    {
        ::WaitForSingleObject(m_hThread, INFINITE);
        Release();
    }

    virtual void Run() = 0;//线程运行函数
private:
    static unsigned int __stdcall Proc(void *pData)
    {
        IThread* pthis = reinterpret_cast<IThread*>(pData);
        if (pthis)
        {
            pthis->Run();
        }

        return -1;
    }

    HANDLE m_hThread;
};