#ifndef __URDE_CSCRIPTIMER_HPP__
#define __URDE_CSCRIPTIMER_HPP__

#include "CEntity.hpp"

namespace urde
{

class CScriptTimer : public CEntity
{
    float x34_time;
    float x38_startTime;
    float x3c_maxRandDelay;
    bool x40_loop;
    bool x41_autoStart;
    bool x42_isTiming;
public:
    CScriptTimer(TUniqueId, std::string_view name, const CEntityInfo& info,
                 float, float, bool, bool, bool);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    bool IsTiming() const;
    void StartTiming(bool isTiming);
    void Reset(CStateManager&);
    void ApplyTime(float, CStateManager&);
};

}

#endif // __URDE_CSCRIPTIMER_HPP__
