#ifndef __RETRO_CIOWINMANAGER_HPP__
#define __RETRO_CIOWINMANAGER_HPP__

#include <memory>
#include <list>
#include "CIOWin.hpp"
#include "rstl.hpp"
#include "CArchitectureQueue.hpp"

namespace Retro
{

class CIOWinManager
{
    struct IOWinPQNode
    {
        rstl::rc_ptr<CIOWin> x0_iowin;
        int x4_prio;
        CIOWinManager::IOWinPQNode* x8_next;
        IOWinPQNode(rstl::ncrc_ptr<CIOWin> iowin, int prio,
                    CIOWinManager::IOWinPQNode* next)
        : x0_iowin(iowin), x4_prio(prio), x8_next(next) {}
        rstl::rc_ptr<CIOWin> GetIOWin() const {return rstl::rc_ptr<CIOWin>(x0_iowin);}
    };
    IOWinPQNode* x0_rootDraw = nullptr;
    IOWinPQNode* x4_rootPump = nullptr;
    CArchitectureQueue x8_internalQueue;
public:
    bool OnIOWinMessage(const CArchitectureMessage& msg);
    void Draw() const;
    bool DistributeOneMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void PumpMessages(CArchitectureQueue& queue);
    rstl::rc_ptr<CIOWin> FindIOWin(const std::string& name);
    void ChangeIOWinPriority(rstl::ncrc_ptr<CIOWin>, int pumpPrio, int drawPrio);
    void RemoveAllIOWins();
    void RemoveIOWin(rstl::ncrc_ptr<CIOWin>);
    void AddIOWin(rstl::ncrc_ptr<CIOWin>, int pumpPrio, int drawPrio);
};



}

#endif // __RETRO_CIOWINMANAGER_HPP__
