#ifndef __URDE_CEXPLOSION_HPP__
#define __URDE_CEXPLOSION_HPP__

#include "CEffect.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{

class CExplosion : public CEffect
{
    std::unique_ptr<CElementGen> xe8_particleGen;
    TUniqueId xec_ = kInvalidUniqueId;
    const CGenDescription* xf0_particleDesc;
    bool xf4_24_:1;
    bool xf4_25_:1;
    bool xf4_26_:1;
    float xf8_ = 0.f;

public:
    CExplosion(const TLockedToken<CGenDescription>& particle, TUniqueId uid, bool active,
               const CEntityInfo& info, std::string_view name, const zeus::CTransform& xf,
               u32, const zeus::CVector3f& scale, const zeus::CColor& color);

     void Accept(IVisitor&);
};

}

#endif // __URDE_CEXPLOSION_HPP__
