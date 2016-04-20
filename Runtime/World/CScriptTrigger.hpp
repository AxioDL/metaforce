#ifndef __URDE_CSCRIPTTRIGGER_HPP__
#define __URDE_CSCRIPTTRIGGER_HPP__

#include "CActor.hpp"

namespace urde
{
class CDamageInfo;

class CScriptTrigger : public CActor
{
public:
    CScriptTrigger(TUniqueId, const std::string& name, const CEntityInfo& info,
                   const zeus::CVector3f& pos, const zeus::CAABox&,
                   const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                   u32 triggerFlags, bool, bool, bool);
};

}

#endif // __URDE_CSCRIPTTRIGGER_HPP__
