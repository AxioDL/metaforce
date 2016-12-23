#ifndef __URDE_CSCRIPTDEBRIS_HPP__
#define __URDE_CSCRIPTDEBRIS_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{
class CScriptDebris : public CPhysicsActor
{
public:
    enum class EScaleType
    {
    };

    enum class EOrientationType
    {
    };

private:
    TReservedAverage<float, 8> x2e0_;
public:
    CScriptDebris(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CActorParameters&, ResId, const zeus::CVector3f&, float, const zeus::CVector3f&,
                  const zeus::CColor&, float, float, float, EScaleType, bool, bool, bool);
#if 0
    CScriptDebris(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CActorParameters&, float, float, float, float, float, float, float, float, float,
                  const zeus::CColor&, const zeus::CColor&, float, const zeus::CVector3f&, const zeus::CVector3f&,
                  float, float, const zeus::CVector3f&, u32, const zeus::CVector3f&, bool, bool, EOrientationType, u32,
                  const zeus::CVector3f&, bool, bool, EOrientationType, u32, const zeus::CVector3f&, EOrientationType,
                  bool, bool, bool);
#endif
};
}

#endif // __URDE_CSCRIPTDEBRIS_HPP__
