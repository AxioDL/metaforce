#include "Collision/CJointCollisionDescription.hpp"

namespace urde
{

CJointCollisionDescription::CJointCollisionDescription(ECollisionType colType, CSegId seg1, CSegId seg2,
                                                       const zeus::CVector3f& v1, const zeus::CVector3f& v2, float f1,
                                                       float f2, EOrientationType orientType, const std::string& name,
                                                       float f3)
: x0_colType(colType)
, x4_orientType(orientType)
, x8_(seg1)
, x9_(seg2)
, xc_(v1)
, x18_(v2)
, x24_(f1)
, x28_(f2)
, x2c_name(name)
, x40_(f3)
{
}

CJointCollisionDescription CJointCollisionDescription::SphereSubdivideCollision(CSegId seg1, CSegId seg2, float f1,
                                                                                float f2, EOrientationType orientType,
                                                                                const std::string& name, float f3)
{
    return CJointCollisionDescription(ECollisionType::SphereSubdivide, seg1, seg2, zeus::CVector3f::skZero,
                                      zeus::CVector3f::skZero, f1, f2, orientType, name, f3);
}

CJointCollisionDescription CJointCollisionDescription::SphereCollision(CSegId segId, float f1, const std::string& name, float f2)
{
    return CJointCollisionDescription(ECollisionType::Sphere, segId, -1, zeus::CVector3f::skZero,
                                      zeus::CVector3f::skZero, f1, 0.f, EOrientationType::Zero, name, f2);
}

CJointCollisionDescription CJointCollisionDescription::AABoxCollision(CSegId segId, const zeus::CVector3f& v1,
                                                                      const std::string& name, float f1)
{
    return CJointCollisionDescription(ECollisionType::AABox, segId, -1, v1, zeus::CVector3f::skZero, 0.f, 0.f,
                                      EOrientationType::Zero, name, f1);
}

CJointCollisionDescription CJointCollisionDescription::OBBCollision(CSegId segId, const zeus::CVector3f& v1,
                                                                    const zeus::CVector3f& v2, const std::string& name,
                                                                    float f1)
{
    return CJointCollisionDescription(ECollisionType::OBB, segId, -1, v1, v2, 0.f, 0.f, EOrientationType::Zero, name,
                                      f1);
}
}
