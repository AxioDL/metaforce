#ifndef __URDE_CJOINTCOLLISIONDESCRIPTION_HPP__
#define __URDE_CJOINTCOLLISIONDESCRIPTION_HPP__

#include "Character/CSegId.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CJointCollisionDescription
{
public:
    enum class ECollisionType
    {
    };

    enum class EOrientationType
    {

    };

private:
    u32 x0_;
    u32 x4_;
    CSegId x8_;
    CSegId x9_;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
    float x24_;
    float x28_;
    std::string x2c_;
    TUniqueId x3c_;
    float x40_;
public:
    CJointCollisionDescription(ECollisionType, CSegId, CSegId, const zeus::CVector3f&, const zeus::CVector3f&, float,
                               float, EOrientationType, const std::string&, float);
    const std::string& GetName() const;
    void GetCollisionActorId() const;
    void SphereSubdivideCollision(ECollisionType, CSegId, CSegId, float, float, EOrientationType, const std::string&,
                                  float);
    void SphereCollision(CSegId, float, const std::string&, float);
    void AABoxCollision(CSegId, zeus::CVector3f const &, const std::string&, float);
    void OBBAutoSizeCollision(CSegId, CSegId, const zeus::CVector3f&, EOrientationType, const std::string&, float);
    void OBBCollision(CSegId, const zeus::CVector3f&, const zeus::CVector3f&, const std::string&, float);
    zeus::CVector3f GetPivotPoint() const;
    float GetRadius() const;
    void SetCollisionActorId(TUniqueId);
    EOrientationType GetOrientationType() const;
    float GetMass() const;
    zeus::CAABox GetBounds() const;
    void GetType() const;
    void GetNextId() const;
    void GetPivotId() const;
    void ScaleAllBounds(const zeus::CVector3f&);
    void GetTween() const;
};
}

#endif // __URDE_CJOINTCOLLISIONDESCRIPTION_HPP__
