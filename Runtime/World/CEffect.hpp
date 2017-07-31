#ifndef __URDE_CEFFECT_HPP__
#define __URDE_CEFFECT_HPP__

#include "CActor.hpp"

namespace urde
{

class CEffect : public CActor
{
public:
    CEffect(TUniqueId uid, const CEntityInfo& info, bool active, const std::string& name,
            const zeus::CTransform& xf);

    virtual void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    virtual void Render(const CStateManager&) const {}
};

}

#endif // __URDE_CEFFECT_HPP__
