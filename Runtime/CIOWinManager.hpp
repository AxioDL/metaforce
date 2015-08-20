#ifndef __RETRO_CIOWINMANAGER_HPP__
#define __RETRO_CIOWINMANAGER_HPP__

#include <memory>
#include "CIOWin.hpp"

namespace Retro
{

class CIOWinManager
{
    struct IOWinPQNode
    {
        std::shared_ptr<CIOWin> m_iowin;
        int m_prio;
        CIOWinManager::IOWinPQNode* m_prev;
        IOWinPQNode(std::weak_ptr<CIOWin> iowin, int prio,
                    CIOWinManager::IOWinPQNode* prev)
        : m_iowin(iowin), m_prio(prio), m_prev(prev) {}
        std::shared_ptr<CIOWin> GetIOWin() const {return m_iowin;}
    };
    bool OnIOWinMessage(const CArchitectureMessage& msg);
};

}

#endif // __RETRO_CIOWINMANAGER_HPP__
