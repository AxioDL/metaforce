#ifndef __URDE_CCOLLISIONINFO_HPP__
#define __URDE_CCOLLISIONINFO_HPP__

#include "RetroTypes.hpp"
#include "CMaterialList.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CMatrix3f.hpp"

namespace urde
{
class CCollisionInfo
{
    zeus::CVector3f x0_point;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
    zeus::CVector3f x24_;
    bool x30_valid = false;
    bool x31_ = false;
    CMaterialList x38_materialLeft;
    CMaterialList x40_materialRight;
    zeus::CVector3f x48_normalLeft;
    zeus::CVector3f x54_normalRight;
public:
    CCollisionInfo() = default;
    CCollisionInfo(const zeus::CVector3f& point, const CMaterialList& list1, const CMaterialList& list2,
                   const zeus::CVector3f& normalLeft, const zeus::CVector3f& normalRight)
    : x0_point(point), x30_valid(true), x31_(false), x38_materialLeft(list2), x40_materialRight(list1),
      x48_normalLeft(normalLeft), x54_normalRight(normalRight) {}
    CCollisionInfo(const zeus::CVector3f& point, const CMaterialList& list1, const CMaterialList& list2,
                   const zeus::CVector3f& normal)
    : x0_point(point), x30_valid(true), x31_(false), x38_materialLeft(list2), x40_materialRight(list1),
      x48_normalLeft(normal), x54_normalRight(-normal) {}
    CCollisionInfo(const zeus::CAABox& aabox, const CMaterialList& list1, const CMaterialList& list2,
                   const zeus::CVector3f& normalLeft, const zeus::CVector3f& normalRight)
    : x0_point(aabox.min),
      xc_(aabox.max.x - aabox.min.x, 0.f, 0.f),
      x18_(0.f, aabox.max.y - aabox.min.y, 0.f),
      x24_(0.f, 0.f, aabox.max.z - aabox.min.z),
      x30_valid(true), x31_(true), x38_materialLeft(list2),
      x40_materialRight(list1), x48_normalLeft(normalLeft),
      x54_normalRight(normalRight)
    {}

    CCollisionInfo GetSwapped() const;
    bool IsValid() const { return x30_valid; }
    const CMaterialList& GetMaterialLeft() const { return x38_materialLeft; }
    const CMaterialList& GetMaterialRight() const { return x40_materialRight; }
    zeus::CVector3f GetExtreme() const;
    void Swap();
    void Transform(const zeus::CTransform&);
    zeus::CVector3f GetNormalLeft() const { return x48_normalLeft; }
    zeus::CVector3f GetNormalRight() const { return x54_normalRight; }
    zeus::CVector3f GetOrigin() const;
    zeus::CVector3f GetPoint() const { return x0_point; }
};

}

#endif // __URDE_CCOLLISIONINFO_HPP__
