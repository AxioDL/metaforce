#ifndef __URDE_CGUISYS_HPP__
#define __URDE_CGUISYS_HPP__

#include "hecl/hecl.hpp"
#include <unordered_map>
#include <vector>
#include <stack>
#include <chrono>
#include "CGuiAutoRepeatData.hpp"
#include "CSaveableState.hpp"
#include "IOStreams.hpp"

namespace urde
{
class CGuiObject;
class CVParamTransfer;
struct SObjectTag;
class IFactory;
class CSimplePool;
class CGuiWidget;
class CGuiFrame;
class CTextParser;
class CTextExecuteBuffer;

typedef CGuiObject*(*FGuiFactoryFunc)(const SObjectTag&, const CVParamTransfer&);

class CGuiFactoryMgr
{
    friend class CGuiSys;
    std::unordered_map<FourCC, FGuiFactoryFunc> m_factories;
public:
    void AddFactory(FourCC key, FGuiFactoryFunc func)
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
        Zero,
        One,
        Two
    };
private:
    IFactory& x0_resFactory;
    CSimplePool& x4_resStore;
    CGuiFactoryMgr x8_factoryMgr;
    std::unordered_map<EPhysicalControllerID, CGuiAutoRepeatData> x18_repeatMap;
    EUsageMode x2c_mode;
    std::unique_ptr<CTextExecuteBuffer> x30_textExecuteBuf;
    std::unique_ptr<CTextParser> x34_textParser;
    CVParamTransfer x38_frameFactoryParams;
    std::chrono::time_point<std::chrono::steady_clock> x40_constructTime;

    void AddFactories(EUsageMode mode);
    void LoadWidgetFunctions();
    static CGuiWidget* CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* frame);
public:
    CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode);

    CSimplePool& GetResStore() {return x4_resStore;}
    std::unordered_map<EPhysicalControllerID, CGuiAutoRepeatData>& GetRepeatMap() {return x18_repeatMap;}
    EUsageMode GetUsageMode() const {return x2c_mode;}
};

/** Global GuiSys instance */
extern CGuiSys* g_GuiSys;

/** Global CTextExecuteBuffer instance */
extern CTextExecuteBuffer* g_TextExecuteBuf;

/** Global CTextParser instance */
extern CTextParser* g_TextParser;

/** Parameter pack for FRME factory */
struct CGuiResFrameData
{
    CGuiSys& x0_guiSys;
    CGuiResFrameData(CGuiSys& guiSys) : x0_guiSys(guiSys) {}
};

}

#endif // __URDE_CGUISYS_HPP__
