#ifndef __URDE_CSCRIPTDEBRIS_HPP__
#define __URDE_CSCRIPTDEBRIS_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{
class CElementGen;
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
    zeus::CVector3f x258_;
    zeus::CColor x264_;
    float x26c_ = 0.f;
    float x270_ = 0.f;
    float x274_ = 0.f;
    float x278_;
    float x27c_;
    bool x281_24_ : 1;
    bool x281_25_ : 1;
    bool x281_26_ : 1;
    bool x281_27_ : 1;
    bool x281_28_ : 1;
    bool x281_29_ : 1;
    bool x281_30_ : 1;
    bool x281_31_ : 1;
    bool x282_24_ : 1;
    zeus::CVector3f x2c8_collisionNormal;
    std::unique_ptr<CElementGen> x2d4_;
    std::unique_ptr<CElementGen> x2d8_;
    std::unique_ptr<CElementGen> x2dc_;
    TReservedAverage<float, 8> x2e0_;

public:
    CScriptDebris(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CActorParameters&, CAssetId, const zeus::CVector3f&, float, const zeus::CVector3f&,
                  const zeus::CColor&, float, float, float, EScaleType, bool, bool, bool);

    CScriptDebris(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CActorParameters&, float, float, float, float, float, float, float, float, float,
                  const zeus::CColor&, const zeus::CColor&, float, const zeus::CVector3f&, const zeus::CVector3f&,
                  float, float, const zeus::CVector3f&, CAssetId, const zeus::CVector3f&, bool, bool, EOrientationType,
                  CAssetId, const zeus::CVector3f&, bool, bool, EOrientationType, CAssetId, const zeus::CVector3f&,
                  EOrientationType, bool, bool, bool, bool);

    void Accept(IVisitor& visitor);
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;

    void CollidedWith(TUniqueId uid, const CCollisionInfoList&, CStateManager&);
};
} // namespace urde

#endif // __URDE_CSCRIPTDEBRIS_HPP__
