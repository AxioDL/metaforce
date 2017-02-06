#include "CIOWinManager.hpp"
#include "CArchitectureMessage.hpp"
#include "CIOWin.hpp"

namespace urde
{

bool CIOWinManager::OnIOWinMessage(const CArchitectureMessage& msg)
{
    switch (msg.GetType())
    {
    case EArchMsgType::RemoveIOWin:
    {
        const CArchMsgParmVoidPtr& parm = MakeMsg::GetParmDeleteIOWin(msg);
        CIOWin* iow = FindIOWin(*static_cast<const std::string*>(parm.x4_parm1));
        if (iow)
            RemoveIOWin(iow);
        return false;
    }
    case EArchMsgType::CreateIOWin:
    {
        const CArchMsgParmInt32Int32IOWin& parm = MakeMsg::GetParmCreateIOWin(msg);
        AddIOWin(parm.xc_parm3, parm.x4_parm1, parm.x8_parm2);
        return false;
    }
    case EArchMsgType::ChangeIOWinPriority:
    {
        const CArchMsgParmInt32Int32VoidPtr& parm = MakeMsg::GetParmChangeIOWinPriority(msg);
        CIOWin* iow = FindIOWin(*static_cast<const std::string*>(parm.xc_parm3));
        if (iow)
            ChangeIOWinPriority(iow, parm.x4_parm1, parm.x8_parm2);
        return false;
    }
    case EArchMsgType::RemoveAllIOWins:
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
    for (IOWinPQNode* node = x0_drawRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        iow->PreDraw();
        if (!iow->GetIsContinueDraw())
            break;
    }
    for (IOWinPQNode* node = x0_drawRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        iow->Draw();
        if (!iow->GetIsContinueDraw())
            break;
    }
}

bool CIOWinManager::DistributeOneMessage(const CArchitectureMessage& msg,
                                         CArchitectureQueue& queue)
{
    CArchitectureMessage tmpMsg = msg;
    for (IOWinPQNode* node = x4_pumpRoot ; node ;)
    {
        IOWinPQNode* next = node->x8_next;
        CIOWin* iow = node->GetIOWin();
        CIOWin::EMessageReturn mret = iow->OnMessage(tmpMsg, x8_localGatherQueue);

        while (x8_localGatherQueue)
        {
            tmpMsg = x8_localGatherQueue.Pop();
            if (tmpMsg.GetTarget() == EArchMsgTarget::IOWinManager)
            {
                if (OnIOWinMessage(tmpMsg))
                {
                    x8_localGatherQueue.Clear();
                    queue.Clear();
                    return true;
                }
            }
            else
                queue.Push(std::move(tmpMsg));
        }

        switch (mret)
        {
        case CIOWin::EMessageReturn::RemoveIOWinAndExit:
        case CIOWin::EMessageReturn::RemoveIOWin:
            RemoveIOWin(iow);
        default: break;
        }

        switch (mret)
        {
        case CIOWin::EMessageReturn::Exit:
        case CIOWin::EMessageReturn::RemoveIOWinAndExit:
            return false;
        default: break;
        }

        node = next;
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

CIOWin* CIOWinManager::FindIOWin(const std::string& name)
{
    size_t findHash = std::hash<std::string>()(name);

    for (IOWinPQNode* node = x4_pumpRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        if (iow->GetNameHash() == findHash)
            return iow;
    }

    for (IOWinPQNode* node = x0_drawRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        if (iow->GetNameHash() == findHash)
            return iow;
    }

    return nullptr;
}

std::shared_ptr<CIOWin> CIOWinManager::FindAndShareIOWin(const std::string& name)
{
    size_t findHash = std::hash<std::string>()(name);

    for (IOWinPQNode* node = x4_pumpRoot ; node ; node = node->x8_next)
    {
        std::shared_ptr<CIOWin> iow = node->ShareIOWin();
        if (iow->GetNameHash() == findHash)
            return iow;
    }

    for (IOWinPQNode* node = x0_drawRoot ; node ; node = node->x8_next)
    {
        std::shared_ptr<CIOWin> iow = node->ShareIOWin();
        if (iow->GetNameHash() == findHash)
            return iow;
    }

    return std::shared_ptr<CIOWin>();
}

void CIOWinManager::ChangeIOWinPriority(CIOWin* toChange, int pumpPrio, int drawPrio)
{
    IOWinPQNode* prevNode = nullptr;
    for (IOWinPQNode* node = x4_pumpRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        if (iow == toChange)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            node->x4_prio = pumpPrio;
            IOWinPQNode* testNode = x4_pumpRoot;
            IOWinPQNode* testPrevNode = nullptr;
            while (testNode->x4_prio > pumpPrio)
            {
                testPrevNode = testNode;
                testNode = testNode->x8_next;
            }
            node->x8_next = testNode;
            if (testPrevNode)
                testPrevNode->x8_next = node;
            else
                x4_pumpRoot = node;
            break;
        }
        prevNode = node;
    }

    prevNode = nullptr;
    for (IOWinPQNode* node = x0_drawRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        if (iow == toChange)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            node->x4_prio = drawPrio;
            IOWinPQNode* testNode = x0_drawRoot;
            IOWinPQNode* testPrevNode = nullptr;
            while (testNode->x4_prio > drawPrio)
            {
                testPrevNode = testNode;
                testNode = testNode->x8_next;
            }
            node->x8_next = testNode;
            if (testPrevNode)
                testPrevNode->x8_next = node;
            else
                x0_drawRoot = node;
            break;
        }
        prevNode = node;
    }
}

void CIOWinManager::RemoveAllIOWins()
{
    for (IOWinPQNode* node = x0_drawRoot ; node ;)
    {
        IOWinPQNode* n = node;
        node = n->x8_next;
        delete n;
    }
    x0_drawRoot = nullptr;
    for (IOWinPQNode* node = x4_pumpRoot ; node ;)
    {
        IOWinPQNode* n = node;
        node = n->x8_next;
        delete n;
    }
    x4_pumpRoot = nullptr;
}

void CIOWinManager::RemoveIOWin(CIOWin* chIow)
{
    IOWinPQNode* prevNode = nullptr;
    for (IOWinPQNode* node = x4_pumpRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        if (iow == chIow)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            else
                x4_pumpRoot = node->x8_next;
            delete node;
            break;
        }
        prevNode = node;
    }

    prevNode = nullptr;
    for (IOWinPQNode* node = x0_drawRoot ; node ; node = node->x8_next)
    {
        CIOWin* iow = node->GetIOWin();
        if (iow == chIow)
        {
            if (prevNode)
                prevNode->x8_next = node->x8_next;
            else
                x0_drawRoot = node->x8_next;
            delete node;
            break;
        }
        prevNode = node;
    }
}

void CIOWinManager::AddIOWin(std::weak_ptr<CIOWin> chIow, int pumpPrio, int drawPrio)
{
    IOWinPQNode* node;
    IOWinPQNode* prevNode = nullptr;
    for (node = x4_pumpRoot ; node && pumpPrio < node->x4_prio ; node = node->x8_next)
        prevNode = node;
    IOWinPQNode* newNode = new IOWinPQNode(chIow, pumpPrio, node);
    if (prevNode)
        prevNode->x8_next = newNode;
    else
        x4_pumpRoot = newNode;

    prevNode = nullptr;
    for (node = x0_drawRoot ; node && drawPrio < node->x4_prio ; node = node->x8_next)
        prevNode = node;
    newNode = new IOWinPQNode(chIow, drawPrio, node);
    if (prevNode)
        prevNode->x8_next = newNode;
    else
        x0_drawRoot = newNode;
}

}
