#ifndef __RETRO_CARCHITECTUREQUEUE_HPP__
#define __RETRO_CARCHITECTUREQUEUE_HPP__

#include <list>
#include "CArchitectureMessage.hpp"

namespace Retro
{

class CArchitectureQueue
{
    std::list<CArchitectureMessage> m_list;
public:
    void PushMessage(CArchitectureMessage&& msg)
    {
        m_list.push_back(std::move(msg));
    }
    const CArchitectureMessage& PeekMessage() const
    {
        return m_list.front();
    }
    void PopMessage()
    {
        m_list.pop_front();
    }
};

}

#endif // __RETRO_CARCHITECTUREQUEUE_HPP__
