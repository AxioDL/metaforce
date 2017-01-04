#ifndef __URDE_CCOLLISIONINFO_HPP__
#define __URDE_CCOLLISIONINFO_HPP__

#include "RetroTypes.hpp"
#include "CMaterialList.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CCollisionInfo
{
    zeus::CAABox x0_aabox = zeus::CAABox::skNullBox;
    zeus::CVector3f x18_;
    zeus::CVector3f x24_;
    bool x30_ = false;
    bool x31_ = false;
    /* u32 x34_; */
    CMaterialList x38_;
    CMaterialList x40_;
    zeus::CVector3f x48_;
    zeus::CVector3f x54_;
public:
    CCollisionInfo() = default;
    CCollisionInfo(const zeus::CVector3f& v1, const CMaterialList& list1, const CMaterialList& list2,
                   const zeus::CVector3f& v2, const zeus::CVector3f& v3);
    CCollisionInfo(const zeus::CVector3f& v1, const CMaterialList& list1, const CMaterialList& list2,
                   zeus::CVector3f const& v2);
    CCollisionInfo(const zeus::CAABox& aabox, const CMaterialList& list1, const CMaterialList& list2,
                   const zeus::CVector3f& v1, const zeus::CVector3f& v2)
        : x0_aabox(aabox)
    {}

    bool IsValid() const;
    void GetMaterialLeft() const;
    void GetMaterialRight() const;
    zeus::CVector3f GetExtreme() const;
    void Swap();
    void Transform(const zeus::CTransform&);
    zeus::CVector3f GetNormalLeft() const;
    zeus::CVector3f GetNormalRight() const;
    zeus::CVector3f GetOrigin() const;
    zeus::CVector3f GetPoint() const;
};

}

#endif // __URDE_CCOLLISIONINFO_HPP__
