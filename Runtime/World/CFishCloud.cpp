#include "CFishCloud.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CFishCloud::CFishCloud(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                       const zeus::CVector3f& scale, const zeus::CTransform& xf, CModelData&& mData,
                       const CAnimRes& aRes, u32 w1, float f1, float f2, float f3, float f4, float f5, float f6,
                       float f7, float f8, float f9, float f10, float f11, float f12, float f13, u32 w2,
                       const zeus::CColor& color, bool b1, float f14, CAssetId part1, u32 w3, CAssetId part2, u32 w4,
                       CAssetId part3, u32 w5, CAssetId part4, u32 w6, u32 w7, bool b2, bool b3)
: CActor(uid, active, name, info, xf, std::move(mData), {EMaterialTypes::Unknown},
         CActorParameters::None(), kInvalidUniqueId)
{

}

}

