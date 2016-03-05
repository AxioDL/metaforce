#ifndef __PSHAG_CARCHITECTUREQUEUE_HPP__
#define __PSHAG_CARCHITECTUREQUEUE_HPP__

#include <list>
#include "CArchitectureMessage.hpp"

namespace urde
{

class CArchitectureQueue
{
    std::list<CArchitectureMessage> m_list;
public:
    void Push(CArchitectureMessage&& msg)
    {
        m_list.push_back(std::move(msg));
    }
    CArchitectureMessage Pop()
    {
        CArchitectureMessage msg = std::move(m_list.front());
        m_list.pop_front();
        return msg;
    }
    void Clear() {m_list.clear();}
    operator bool() {return m_list.size() != 0;}
};

}

#endif // __PSHAG_CARCHITECTUREQUEUE_HPP__
