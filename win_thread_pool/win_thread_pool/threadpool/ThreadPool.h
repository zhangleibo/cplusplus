#pragma once

#include <vector>
#include <algorithm>
#include "Thread.h"
#include "BoundedBlockingQueue.h"
#include "InterfaceAdapter.h"

class CThreadPool : CNonCopyable
{
public:
    // class worker
    class CWorker : public IThread
    {
    public:
        CWorker(CThreadPool* pool) : m_pThreadPool(pool)
        {

        }

        virtual ~CWorker(){}
        virtual void Run()
        {
            while (true)
            {
                if (!m_pThreadPool)
                    return;

                CIAdapter* pObj = m_pThreadPool->m_pBlockQueue.Pop();

                if (NULL == pObj)
                {
                    break;
                }

				pObj->Run();

                delete pObj;
                pObj = NULL;
            }
        }

    private:
        CThreadPool *m_pThreadPool;
    };

    //class thread pool
    CThreadPool(size_t threadMaxNum) : m_pVecThread(threadMaxNum),
        m_pBlockQueue(threadMaxNum)
    {
        for (size_t i = 0; i < m_pVecThread.size(); ++i)
        {
            m_pVecThread[i] = new CWorker(this);
        }
    }

    ~CThreadPool()
    {
        Stop();
    }

    void Start()
    {
        for (size_t i = 0; i < m_pVecThread.size(); ++i)
        {
            if (m_pVecThread[i])
            {
                m_pVecThread[i]->Start();
            }
        }
    }

    void Stop()
    {
        for (size_t i = 0; i < m_pVecThread.size(); ++i)
        {
            m_pBlockQueue.Push(NULL);//clear queue
        }

        for (size_t i = 0; i < m_pVecThread.size(); ++i)
        {
            if (m_pVecThread[i])
            {
                m_pVecThread[i]->Stop();
                delete m_pVecThread[i];
            }
        }

        m_pVecThread.clear();
    }
    // add task
    void Add(CIAdapter* task)
    {
        if (NULL != task)
        {
            m_pBlockQueue.Push(task);
        }
    }
private:
    friend class CWorker;
    std::vector<IThread*> m_pVecThread;
    CBoundedBlockingQueue<CIAdapter*> m_pBlockQueue;
};
