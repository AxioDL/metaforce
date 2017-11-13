#ifndef __URDE_CGAMELIGHT_HPP__
#define __URDE_CGAMELIGHT_HPP__

#include "CActor.hpp"

namespace urde
{
class CGameLight : public CActor
{
    TUniqueId xe8_parentId;
    CLight xec_light;
    u32 x13c_loadedIdx;
    u32 x140_priority;
    float x144_lifeTime;

public:
    CGameLight(TUniqueId, TAreaId, bool, std::string_view, const zeus::CTransform&, TUniqueId, const CLight&, u32,
               u32, float);

    void Accept(IVisitor &visitor);
    void Think(float, CStateManager&);
    void SetLightPriorityAndId();
    void SetLight(const CLight&);
    CLight GetLight() const;
    TUniqueId GetParentId() const { return xe8_parentId; }
};
}
#endif // __URDE_CGAMELIGHT_HPP__
