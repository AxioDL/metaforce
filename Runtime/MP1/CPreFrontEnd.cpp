#include "CPreFrontEnd.hpp"
#include "CResLoader.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1.hpp"

namespace urde
{
namespace MP1
{

CPreFrontEnd::CPreFrontEnd()
: CIOWin("Pre front-end window")
{

}

CIOWin::EMessageReturn CPreFrontEnd::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue&)
{
    if (msg.GetType() != EArchMsgType::TimerTick)
        return EMessageReturn::Normal;

    CMain* m = static_cast<CMain*>(g_Main);
    if (CResLoader* loader = g_ResFactory->GetResLoader())
        if (loader->AreAllPaksLoaded())
            return EMessageReturn::Exit;
    if (!x14_resourceTweaksRegistered)
    {
        m->RegisterResourceTweaks();
        x14_resourceTweaksRegistered = true;
    }
    m->MemoryCardInitializePump();
    if (!g_MemoryCardSys)
        return EMessageReturn::Exit;
    if (!m->LoadAudio())
        return EMessageReturn::Exit;
    return EMessageReturn::RemoveIOWinAndExit;
}

}
}
