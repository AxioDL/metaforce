#include <memory>
#include "COsContext.hpp"
#include "CBasics.hpp"
#include "CTweaks.hpp"
#include "CMemory.hpp"
#include "CMemoryCardSys.hpp"
#include "CResFactory.hpp"
#include "CSimplePool.hpp"

namespace Retro
{
namespace MP1
{

class CGameGlobalObjects : public Common::TOneStatic<CGameGlobalObjects>
{
    Common::CMemoryCardSys m_memoryCardSys;
    Common::CResFactory m_resFactory;
    Common::CSimplePool m_simplePool;
public:
    void PostInitialize(Common::COsContext& osctx, Common::CMemorySys& memSys)
    {
    }
};

class CMain : public Common::COsContext
{
    Common::CMemorySys m_memSys;
    Common::CTweaks m_tweaks;
    bool m_run = true;
public:
    CMain()
    : m_memSys(*this, Common::CMemorySys::GetGameAllocator())
    {
        OpenWindow("", 0, 0, 640, 480);
    }
    void InitializeSubsystems()
    {
    }
    int RsMain(int argc, const char* argv[])
    {
        CGameGlobalObjects* globalObjs = new CGameGlobalObjects;
        InitializeSubsystems();
        globalObjs->PostInitialize(*this, m_memSys);
        while (m_run)
        {

        }
        return 0;
    }
};

}
}

int main(int argc, const char* argv[])
{
    Retro::MP1::CMain main;
    return main.RsMain(argc, argv);
}
