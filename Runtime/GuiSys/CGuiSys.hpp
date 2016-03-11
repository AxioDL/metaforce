#ifndef __URDE_CGUISYS_HPP__
#define __URDE_CGUISYS_HPP__

#include "hecl/hecl.hpp"
#include <unordered_map>
#include <vector>
#include <stack>
#include <chrono>
#include "CGuiAutoRepeatData.hpp"
#include "CSaveableState.hpp"

namespace urde
{
class CGuiObject;
class CVParamTransfer;
class SObjectTag;
class IFactory;
class CSimplePool;

typedef CGuiObject*(*FGuiFactoryFunc)(const SObjectTag&, const CVParamTransfer&);

class CGuiFactoryMgr
{
    friend class CGuiSys;
    std::unordered_map<hecl::FourCC, FGuiFactoryFunc> m_factories;
public:
    void AddFactory(hecl::FourCC key, FGuiFactoryFunc func)
    {
        m_factories[key] = func;
    }
};

class CGuiSys
{
    friend class CGuiFrame;
public:
    enum class EUsageMode
    {
        Zero
    };
private:
    IFactory& x0_resFactory;
    CSimplePool& x4_resStore;
    CGuiFactoryMgr x8_factoryMgr;
    std::unordered_map<EPhysicalControllerID, CGuiAutoRepeatData> x18_repeatMap;
    EUsageMode x2c_mode;
    std::stack<CSaveableState, std::vector<CSaveableState>> x28_saveStack;
    CVParamTransfer x38_frameFactoryParams;
    std::chrono::time_point<std::chrono::steady_clock> x40_constructTime;

    void AddFactories(EUsageMode mode);
    void LoadWidgetFunctions();
public:
    CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode);
};

/** Parameter pack for FRME factory */
struct CGuiResFrameData
{
    CGuiSys& x0_guiSys;
    CGuiResFrameData(CGuiSys& guiSys) : x0_guiSys(guiSys) {}
};

}

#endif // __URDE_CGUISYS_HPP__
