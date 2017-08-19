#ifndef __URDE_CSCRIPTPLAYERHINT_HPP__
#define __URDE_CSCRIPTPLAYERHINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptPlayerHint : public CActor
{
    rstl::reserved_vector<TUniqueId, 8> xe8_objectList;
    bool xfc_deactivated = false;
    u32 x100_priority;
    u32 x104_overrideFlags;
    TUniqueId x108_actor = kInvalidUniqueId;
public:
    CScriptPlayerHint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                      const zeus::CTransform& xf, bool active, u32 priority, u32 overrideFlags);
    void Accept(IVisitor& visit);
    u32 GetPriority() const { return x100_priority; }
    u32 GetOverrideFlags() const { return x104_overrideFlags; }
    TUniqueId GetActorId() const { return x108_actor; }
    void ClearObjectList() { xe8_objectList.clear(); }
    void SetDeactivated() { xfc_deactivated = true; }
};

}

#endif // __URDE_CSCRIPTPLAYERHINT_HPP__