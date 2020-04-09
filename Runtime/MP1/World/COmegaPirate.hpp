#pragma once

#include "Runtime/MP1/World/CElitePirate.hpp"

namespace urde::MP1 {
class COmegaPirate : public CElitePirate {
private:
  class CFlash : public CActor {
  private:
    CToken xe8_;
    int xf0_;
    float xf4_;
    float xf8_;
    float xfc_;

    CFlash(TUniqueId uid, const CEntityInfo& info, const zeus::CVector3f& pos, CToken& p4, float p5);
  };

public:
  COmegaPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, CElitePirateData data,
               CAssetId w1, CAssetId w2, CAssetId w3);
};
} // namespace urde::MP1
