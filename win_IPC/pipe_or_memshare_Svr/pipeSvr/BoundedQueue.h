#pragma once

#include <vector>
#include <assert.h>

template<typename T>
class CBoundedQueue
{
public:
    explicit CBoundedQueue(size_t capacity) : m_cycleQueue(capacity),
        m_capacity(capacity)
    {
        m_head = 0;
        m_tail = 0;
        m_size = 0;
        assert(m_capacity);
    }

    ~CBoundedQueue()
    {

    }

    void Clear()
    {
        m_head = 0;
        m_tail = 0;
        m_size = 0;
    }

    bool Empty() const
    {
        return !m_size;
    }

    bool Full() const
    {
        return m_capacity == m_size;
    }

    size_t Size() const
    {
        return m_size;
    }

    size_t Capacity() const
    {
        return m_capacity;
    }

    void Push(const T& t)
    {
        assert(!Full());
        m_cycleQueue[m_tail] = t;
        m_tail = (m_tail + 1) % m_capacity;
        m_size++;
    }

    T Pop()
    {
        assert(!Empty());
        size_t oldPos = m_head;
        m_head = (m_head + 1) % m_capacity;
        m_size--;
        return m_cycleQueue[oldPos];
    }

private:
    size_t m_head;
    size_t m_tail;
    size_t m_size;
    const size_t m_capacity;
    std::vector<T> m_cycleQueue;
};