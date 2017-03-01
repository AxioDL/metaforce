#ifndef __URDE_CSCRIPTSTEAM_HPP__
#define __URDE_CSCRIPTSTEAM_HPP__

#include "CScriptTrigger.hpp"

namespace urde
{

class CScriptSteam : public CScriptTrigger
{
public:
    CScriptSteam(TUniqueId, const std::string& name, const CEntityInfo& info, const zeus::CVector3f& pos,
                 const zeus::CAABox&, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                 ETriggerFlags flags, bool active, ResId, float, float, float, float, bool);
};

}

#endif // __URDE_CSCRIPTSTEAM_HPP__
