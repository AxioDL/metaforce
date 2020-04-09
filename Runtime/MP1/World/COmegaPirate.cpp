#include "Runtime/MP1/World/COmegaPirate.hpp"

namespace urde::MP1 {
COmegaPirate::CFlash::CFlash(TUniqueId uid, const CEntityInfo& info, const zeus::CVector3f& pos, CToken& p4, float p5)
: CActor(uid, true, "Omega Pirate Flash", info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(), {},
         CActorParameters::None(), kInvalidUniqueId) {}

COmegaPirate::COmegaPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           CElitePirateData data, CAssetId w1, CAssetId w2, CAssetId w3)
: CElitePirate(uid, name, info, xf, std::move(mData), pInfo, actParms, data) {}
} // namespace urde::MP1
