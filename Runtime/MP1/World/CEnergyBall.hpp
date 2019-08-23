#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CEnergyBall : public CPatterned {
public:
  DEFINE_PATTERNED(EnergyBall)
  CEnergyBall(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
              CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, s32 w1, float f1,
              const CDamageInfo& dInfo1, float f2, const CAssetId& a1, s16 sfxId1, const CAssetId& a2,
              const CAssetId& a3, s16 sfxId2, float f3, float f4, const CAssetId& a4, const CDamageInfo& dInfo2,
              float f5);
};
}
