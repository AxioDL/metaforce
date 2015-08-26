#include "CIOWinManager.hpp"
#include "CArchitectureMessage.hpp"

namespace Retro
{

bool CIOWinManager::OnIOWinMessage(const CArchitectureMessage& msg)
{
    switch (msg.GetType())
    {
    case MsgRemoveIOWin:
    {
        const CArchMsgParmVoidPtr& parm = MakeMsg::GetParmDeleteIOWin(msg);
        rstl::rc_ptr<CIOWin> iow = FindIOWin(*static_cast<const std::string*>(parm.x4_parm1));
        if (iow)
            RemoveIOWin(iow);
        return false;
    }
    case MsgCreateIOWin:
    {
        const CArchMsgParmInt32Int32VoidPtr& parm = MakeMsg::GetParmCreateIOWin(msg);
        rstl::rc_ptr<CIOWin> iow(static_cast<CIOWin*>(parm.xc_parm3));
        AddIOWin(iow, parm.x4_parm1, parm.x8_parm2);
        return false;
    }
    case MsgChangeIOWinPriority:
    {
        const CArchMsgParmInt32Int32VoidPtr& parm = MakeMsg::GetParmChangeIOWinPriority(msg);
        rstl::rc_ptr<CIOWin> iow = FindIOWin(*static_cast<const std::string*>(parm.xc_parm3));
        if (iow)
            ChangeIOWinPriority(iow, parm.x4_parm1, parm.x8_parm2);
        return false;
    }
    case MsgRemoveAllIOWins:
    {
        RemoveAllIOWins();
        return true;
    }
    default: break;
    }
    return false;
}

void CIOWinManager::Draw() const
{
    IOWinPQNode* node = x0_rootDraw;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        iow->PreDraw();
        if (!iow->GetIsContinueDraw())
            break;
        node = node->x8_next;
    }
    node = x0_rootDraw;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        iow->Draw();
        if (!iow->GetIsContinueDraw())
            break;
        node = node->x8_next;
    }
}

bool CIOWinManager::DistributeOneMessage(const CArchitectureMessage& msg,
                                         CArchitectureQueue& queue)
{
    CIOWinManager::IOWinPQNode* node = x4_rootPump;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        CIOWin::EMessageReturn mret = iow->OnMessage(msg, x8_internalQueue);

        while (x8_internalQueue)
        {
            CArchitectureMessage msg = x8_internalQueue.Pop();
            if (msg.GetTarget() == TargetIOWinManager)
            {
                if (OnIOWinMessage(msg))
                {
                    x8_internalQueue.Clear();
                    queue.Clear();
                    return true;
                }
            }
            else
                queue.Push(std::move(msg));
        }

        switch (mret)
        {
        case CIOWin::MsgRetRemoveIOWinAndExit:
        case CIOWin::MsgRetRemoveIOWin:
            RemoveIOWin(iow);
        default: break;
        }

        switch (mret)
        {
        case CIOWin::MsgRetExit:
        case CIOWin::MsgRetRemoveIOWinAndExit:
            return false;
        default: break;
        }

        node = node->x8_next;
    }

    return false;
}

void CIOWinManager::PumpMessages(CArchitectureQueue& queue)
{
    while (queue)
    {
        CArchitectureMessage msg = queue.Pop();
        if (DistributeOneMessage(msg, queue))
            break;
    }
}

rstl::rc_ptr<CIOWin> CIOWinManager::FindIOWin(const std::string& name)
{
    size_t findHash = std::hash<std::string>()(name);

    CIOWinManager::IOWinPQNode* node = x4_rootPump;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        if (iow->GetNameHash() == findHash)
            return iow;
        node = node->x8_next;
    }

    node = x0_rootDraw;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        if (iow->GetNameHash() == findHash)
            return iow;
        node = node->x8_next;
    }

    return rstl::rc_ptr<CIOWin>();
}

void CIOWinManager::ChangeIOWinPriority(rstl::ncrc_ptr<CIOWin> chIow, int pumpPrio, int drawPrio)
{
    CIOWinManager::IOWinPQNode* node = x4_rootPump;
    CIOWinManager::IOWinPQNode* prevNode = nullptr;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        if (iow == chIow)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            node->x4_prio = pumpPrio;
            CIOWinManager::IOWinPQNode* testNode = x4_rootPump;
            CIOWinManager::IOWinPQNode* testPrevNode = nullptr;
            while (testNode->x4_prio > pumpPrio)
            {
                testPrevNode = testNode;
                testNode = testNode->x8_next;
            }
            node->x8_next = testNode;
            if (testPrevNode)
                testPrevNode->x8_next = node;
            else
                x4_rootPump = node;
            break;
        }
        prevNode = node;
        node = node->x8_next;
    }

    node = x0_rootDraw;
    prevNode = nullptr;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        if (iow == chIow)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            node->x4_prio = drawPrio;
            CIOWinManager::IOWinPQNode* testNode = x0_rootDraw;
            CIOWinManager::IOWinPQNode* testPrevNode = nullptr;
            while (testNode->x4_prio > drawPrio)
            {
                testPrevNode = testNode;
                testNode = testNode->x8_next;
            }
            node->x8_next = testNode;
            if (testPrevNode)
                testPrevNode->x8_next = node;
            else
                x0_rootDraw = node;
            break;
        }
        prevNode = node;
        node = node->x8_next;
    }
}

void CIOWinManager::RemoveAllIOWins()
{
    while (x0_rootDraw)
        RemoveIOWin(x0_rootDraw->GetIOWin());
    while (x4_rootPump)
        RemoveIOWin(x4_rootPump->GetIOWin());
}

void CIOWinManager::RemoveIOWin(rstl::ncrc_ptr<CIOWin> chIow)
{
    CIOWinManager::IOWinPQNode* node = x4_rootPump;
    CIOWinManager::IOWinPQNode* prevNode = nullptr;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        if (iow == chIow)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            else
                x4_rootPump = node->x8_next;
            delete node;
            break;
        }
        prevNode = node;
        node = node->x8_next;
    }

    node = x0_rootDraw;
    prevNode = nullptr;
    while (node)
    {
        rstl::rc_ptr<CIOWin> iow = node->GetIOWin();
        if (iow == chIow)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            else
                x0_rootDraw = node->x8_next;
            delete node;
            break;
        }
        prevNode = node;
        node = node->x8_next;
    }
}

void CIOWinManager::AddIOWin(rstl::ncrc_ptr<CIOWin> chIow, int pumpPrio, int drawPrio)
{
    CIOWinManager::IOWinPQNode* node = x4_rootPump;
    CIOWinManager::IOWinPQNode* prevNode = nullptr;
    while (node && pumpPrio > node->x4_prio)
    {
        prevNode = node;
        node = node->x8_next;
    }
    CIOWinManager::IOWinPQNode* newNode = new CIOWinManager::IOWinPQNode(chIow, pumpPrio, node);
    if (prevNode)
        prevNode->x8_next = newNode;
    else
        x4_rootPump = newNode;

    node = x0_rootDraw;
    prevNode = nullptr;
    while (node && drawPrio > node->x4_prio)
    {
        prevNode = node;
        node = node->x8_next;
    }
    newNode = new CIOWinManager::IOWinPQNode(chIow, drawPrio, node);
    if (prevNode)
        prevNode->x8_next = newNode;
    else
        x0_rootDraw = newNode;
}

}
