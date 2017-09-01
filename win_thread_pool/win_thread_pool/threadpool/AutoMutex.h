#pragma once

#include <wtypes.h>

//接口类
class IMyLock
{
public:
	virtual ~IMyLock(){};
	virtual void Lock() const = 0;
	virtual void Unlock() const = 0;
};
//互斥对象锁类
class CMutex : public IMyLock
{
public:
	CMutex()
	{
		m_mutex = ::CreateMutex(NULL, FALSE, NULL);
	}
	~CMutex()
	{
		if (NULL != m_mutex)
			::CloseHandle(m_mutex);
	}
	virtual void Lock() const
	{
		::WaitForSingleObject(m_mutex, INFINITE);
	}
	virtual void Unlock() const
	{
		::ReleaseMutex(m_mutex);
	}
private:
	HANDLE m_mutex;
};
//锁
class CLock
{
public:
	CLock(const IMyLock& lock) : m_lock(lock)
	{
		m_lock.Lock();
	}
	~CLock()
	{
		m_lock.Unlock();
	}
private:
	const IMyLock& m_lock;
};
