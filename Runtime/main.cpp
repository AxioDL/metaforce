#include <memory>
#include "COsContext.hpp"
#include "CBasics.hpp"
#include "CTweaks.hpp"
#include "CMemoryCardSys.hpp"

class CGameGlobalObjects : public TOneStatic<CGameGlobalObjects>
{
    CMemoryCardSys m_memoryCardSys;
public:

};

class CMain : public COsContext
{
    std::unique_ptr<CGameGlobalObjects> m_gameGlobalObjects;
    CTweaks m_tweaks;
public:
    CMain()
    {
        OpenWindow("", 0, 0, 640, 480);
    }
    void InitializeSubsystems()
    {
    }
    int RsMain(int argc, const char* argv[])
    {
        m_gameGlobalObjects.reset(new CGameGlobalObjects);
        return 0;
    }
};

int main(int argc, const char* argv[])
{
    CMain main;
    return main.RsMain(argc, argv);
}
