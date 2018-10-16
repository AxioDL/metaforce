#pragma once

#include "CActor.hpp"

namespace urde
{

class CEffect : public CActor
{
public:
    CEffect(TUniqueId uid, const CEntityInfo& info, bool active, std::string_view name,
            const zeus::CTransform& xf);

    virtual void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    virtual void Render(const CStateManager&) const {}
};

}

