#include "CExplosion.hpp"
#include "TCastTo.hpp"

namespace urde
{

CExplosion::CExplosion(const TLockedToken<CGenDescription>& particle, TUniqueId uid, bool active,
                       const CEntityInfo& info, std::string_view name, const zeus::CTransform& xf,
                       u32 flags, const zeus::CVector3f& scale, const zeus::CColor& color)
: CEffect(uid, info, active, name, xf)
{
    xe8_particleGen = std::make_unique<CElementGen>(particle, CElementGen::EModelOrientationType::Normal,
                                                    flags & 0x2 ? CElementGen::EOptionalSystemFlags::Two :
                                                                  CElementGen::EOptionalSystemFlags::One);
    xf0_particleDesc = particle.GetObj();
    xf4_24_ = flags & 0x4;
    xf4_25_ = true;
    xf4_26_ = flags & 0x8;
    xe6_27_renderVisorFlags = flags & 0x1 ? 1 : 2;
    xe8_particleGen->SetGlobalTranslation(xf.origin);
    xe8_particleGen->SetOrientation(xf.getRotation());
    xe8_particleGen->SetGlobalScale(scale);
    xe8_particleGen->SetModulationColor(color);
}

void CExplosion::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}
}
