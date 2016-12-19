#ifndef __URDE_CSCRIPTIMER_HPP__
#define __URDE_CSCRIPTIMER_HPP__

#include "CEntity.hpp"

namespace urde
{

class CScriptTimer : public CEntity
{
    float x34_;
    float x38_;
    float x3c_;
    bool x40_;
    bool x41_;
    bool x42_;
public:
    CScriptTimer(TUniqueId, const std::string& name, const CEntityInfo& info,
                 float, float, bool, bool, bool);

    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    bool IsTiming();
    void StartTiming(bool);
    void Reset(CStateManager&);
    void ApplyTime(float, CStateManager&);
};

}

#endif // __URDE_CSCRIPTIMER_HPP__
