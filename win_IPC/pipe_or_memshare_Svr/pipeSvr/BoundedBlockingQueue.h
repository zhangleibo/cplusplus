#pragma once

#include "NonCopyable.h"
#include "BoundedQueue.h"
#include "AutoMutex.h"
#include "MySemaphore.h"

template<typename T>
class CBoundedBlockingQueue : CNonCopyable
{
public:
    explicit CBoundedBlockingQueue(size_t maxSize) : m_boundedQueue(maxSize)
    {
		m_NotEmpty.CreateSem(0, maxSize);
		m_NotFull.CreateSem(maxSize, maxSize);
    }

    ~CBoundedBlockingQueue()
    {

    }

    void Push(const T& t)
    {
		m_NotFull.Wait();
		{
			CLock lock(m_mutex);
			assert(!m_boundedQueue.Full());
			m_boundedQueue.Push(t);
		}
		m_NotEmpty.Notify();
    }

    T Pop()
    {
		T res;
		m_NotEmpty.Wait();
		{
			CLock lock(m_mutex);
			assert(!m_boundedQueue.Empty());
			res = m_boundedQueue.Pop();
		}
		m_NotFull.Notify();

        return res;
    }
private:
	CMutex m_mutex;
	CSemaphore m_NotEmpty;
	CSemaphore m_NotFull;
    CBoundedQueue<T> m_boundedQueue;
};