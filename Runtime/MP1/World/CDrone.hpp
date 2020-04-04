#pragma once

#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CVisorFlare.hpp"

namespace urde::MP1 {
class CDrone : public CPatterned {
  CAssetId x568_;
  TLockedToken<CCollisionResponseData> x56c_;
  CCollisionResponseData* x574_;
  TUniqueId x578_ = kInvalidUniqueId;
  TUniqueId x57a_ = kInvalidUniqueId;
  std::vector<CVisorFlare::CFlareDef> x57c_flares;
  u32 x58c_ = 2;
  CDamageInfo x590_;
  CDamageInfo x5ac_;
  float x5c8_ = 0.f;
  float x5cc_ = 0.f;
  float x5d0_ = 0.f;
  float x5d4_ = 0.f;
  float x5d8_ = 0.f;
  float x5dc_ = 0.f;
  float x5e0_ = 0.f;
  float x5e4_;
  float x5e8_ = 0.f;
  float x5ec_;
  float x5f0_;
  float x5f4_;
  float x5f8_;
  float x5fc_;
  float x600_;
  float x604_ = 0.f;
  float x608_;
  float x60c_;
  float x610_;
  float x614_;
  float x618_;
  float x61c_;
  float x620_;
  float x624_ = 0.f;
  float x628_ = 0.f;
  float x62c_ = 0.f;
  float x630_ = 0.f;
  float x634_ = 0.f;
  float x638_ = 0.f;
  float x63c_;
  float x640_;
  float x644_ = 0.f;
  float x648_;
  float x64c_;
  float x650_;
  float x654_;
  float x658_;
  float x65c_;
  float x660_;
  float x664_;
  float x668_ = 0.f;
  float x66c_ = 0.f;
  zeus::CVector3f x670_;
  zeus::CVector3f x67c_;
  TUniqueId x688_ = kInvalidUniqueId;
  CCollidableSphere x690_;
  CPathFindSearch x6b0_pathFind;
  zeus::CAxisAngle x794_;
  zeus::CVector3f x7a0_;
  float x7ac_ = 0.f;
  float x7b0_ = 0.f;
  float x7b4_ = 0.f;
  float x7b8_ = 0.f;
  float x7bc_ = 0.f;
  s32 x7c8_ = 0;
  s16 x7cc_;
  s32 x7d0_ = 0;
  rstl::reserved_vector<TUniqueId, 2> x7d4_;
  rstl::reserved_vector<zeus::CVector3f, 2> x7e0_;
  rstl::reserved_vector<zeus::CVector3f, 2> x7fc_;
  rstl::reserved_vector<float, 2> x818_;
  rstl::reserved_vector<bool, 2> x824_;
  std::unique_ptr<CModelData> x82c_;
  u8 x830_13_ : 2;
  u8 x830_10_ : 2;
  bool x834_24_ : 1;
  bool x834_25_ : 1;
  bool x834_26_ : 1;
  bool x834_27_ : 1;
  bool x834_28_ : 1;
  bool x834_29_ : 1;
  bool x834_30_ : 1;
  bool x834_31_ : 1;
  bool x835_24_ : 1;
  bool x835_25_ : 1;
  bool x835_26_ : 1;
  void UpdateTouchBounds(float radius);
public:
  DEFINE_PATTERNED(Drone);
  CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info, const zeus::CTransform& xf,
         float f1, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
         EMovementType movement, EColliderType colliderType, EBodyType bodyType, const CDamageInfo& dInfo1,
         CAssetId aId1, const CDamageInfo& dInfo2, CAssetId aId2, std::vector<CVisorFlare::CFlareDef> flares, float f2,
         float f3, float f4, float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12,
         float f13, float f14, float f15, float f16, float f17, float f18, float f19, float f20, CAssetId crscId,
         float f21, float f22, float f23, float f24, s32 w3, bool b1);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
};
} // namespace urde::MP1
