#include <memory>
#include "COsContext.hpp"
#include "CBasics.hpp"
#include "CTweaks.hpp"
#include "CMemory.hpp"
#include "CMemoryCardSys.hpp"
#include "CResFactory.hpp"
#include "CSimplePool.hpp"

class CGameGlobalObjects : public TOneStatic<CGameGlobalObjects>
{
    CMemoryCardSys m_memoryCardSys;
    CResFactory m_resFactory;
    CSimplePool m_simplePool;
public:
    void PostInitialize(COsContext& osctx, CMemorySys& memSys)
    {
    }
};

class CMain : public COsContext
{
    CMemorySys m_memSys;
    CTweaks m_tweaks;
    bool m_run = true;
public:
    CMain()
    : m_memSys(*this, CMemorySys::GetGameAllocator())
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

int main(int argc, const char* argv[])
{
    CMain main;
    return main.RsMain(argc, argv);
}
