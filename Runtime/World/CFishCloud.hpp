#ifndef __URDE_CFISHCLOUD_HPP__
#define __URDE_CFISHCLOUD_HPP__

#include "CActor.hpp"

namespace urde
{

class CFishCloud : public CActor
{
    class CModifierSource
    {
        TUniqueId x0_source;
        float x4_;
        float x8_;
        bool xc_;
        bool xd_;
    public:
        CModifierSource(TUniqueId, bool, float, float);
        void SetAffectPriority(float);
        void SetAffectRadius(float);
        float GetAffectPriority() const;
        float GetAffectRadius() const;
        bool IsRepulsor();
        TUniqueId GetSource() const;

        bool operator<(const CModifierSource& other) const { return x0_source < other.x0_source; }
    };

public:
    CFishCloud(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
               const zeus::CVector3f& scale, const zeus::CTransform& xf, CModelData&& mData,
               const CAnimRes& aRes, u32 w1, float f1, float f2, float f3, float f4, float f5, float f6,
               float f7, float f8, float f9, float f10, float f11, float f12, float f13, u32 w2,
               const zeus::CColor& color, bool b1, float f14, CAssetId part1, u32 w3, CAssetId part2, u32 w4,
               CAssetId part3, u32 w5, CAssetId part4, u32 w6, u32 w7, bool b2, bool b3);

    void Accept(IVisitor& visitor);
    void RemoveRepulsor(TUniqueId);
    void RemoveAttractor(TUniqueId);
    void AddRepulsor(TUniqueId, float, float);
    void AddAttractor(TUniqueId, float, float);
};

}

#endif // __URDE_CFISHCLOUD_HPP__
