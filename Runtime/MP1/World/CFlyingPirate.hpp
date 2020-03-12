#pragma once

#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Weapon/CBurstFire.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CFlyingPirate : public CPatterned {
public:
  DEFINE_PATTERNED(FlyingPirate)

private:
  class CFlyingPirateData {
    friend class CFlyingPirate;
    float x0_;
    float x4_;
    int x8_;
    CProjectileInfo xc_projInfo1;
    u16 x34_sfx1;
    CProjectileInfo x38_projInfo2;
    CProjectileInfo x60_projInfo3;
    float x88_;
    float x8c_;
    TCachedToken<CGenDescription> x90_particleGenDesc;
    CDamageInfo x9c_dInfo;
    float xb8_;
    float xbc_;
    float xc0_;
    float xc4_;
    u16 xc8_sfx2;
    u16 xca_sfx3;
    float xcc_;
    float xd0_;
    float xd4_;
    CAssetId xd8_;
    CAssetId xdc_;
    CAssetId xe0_;
    u16 xe4_sfx4;
    u16 xe6_sfx5;
    float xe8_;
    float xec_;
    float xf0_;

  public:
    CFlyingPirateData(CInputStream& in, u32 propCount);
  };

public:
  CFlyingPirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

private:
  CFlyingPirateData x568_data;
  rstl::reserved_vector<TCachedToken<CGenDescription>, 4> x65c_particleGenDescs;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 16> x684_particleGens;
  char x6a0_flags1; // TODO
  char x6a1_flags2; // TODO
  char x6a2_flags3; // TODO
  TUniqueId x6a4_id1 = kInvalidUniqueId;
  TUniqueId x6a6_id2 = kInvalidUniqueId;
  CPathFindSearch x6a8_pathFindSearch;
  int x790_ = 0;
  int x794_health;
  CSegId x798_;
  int x79c_ = -1;
  CBoneTracking x7a0_boneTracking;
  float x7d8_ = 0.f;
  int x7dc_ = 0;
  CSegId x7e0_;
  float x7e4_ = 1.f;
  TUniqueId x7e8_id3 = kInvalidUniqueId;
  CBurstFire x7ec_burstFire;
  int x84c_ = -1;
  float x850_ = 3.f;
  float x854_ = FLT_MAX;
  float x858_ = FLT_MAX;
  TUniqueId x85c_ = kInvalidUniqueId;
  float x860_ = 15.f;
  rstl::reserved_vector<CSegId, 4> x864_missileSegments;
  float x86c_ = 0.f;
  zeus::CVector3f x870_ = zeus::skZero3f;
  zeus::CVector3f x87c_ = zeus::skZero3f;
  float x888_ = 10.f;
  float x88c_ = 3.f;
  TUniqueId x890_ = kInvalidUniqueId;
  float x894_ = 1.f;
  float x898_ = 1.f;
//  CFlyingPirateRagDoll x89c_ragDoll; TODO
  TUniqueId x8a0_ = kInvalidUniqueId;
  float x8a4_ = 0.f;
};
} // namespace urde::MP1
