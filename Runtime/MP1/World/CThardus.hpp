#pragma once

#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"

namespace urde {
class CCollisionActorManager;
namespace MP1 {
class CThardus : public CPatterned {
  class CThardusSomething {
    TUniqueId x0_ = kInvalidUniqueId;
    zeus::CVector3f x4_;
    bool x10_24_ : 1;

  public:
    CThardusSomething() : x10_24_(false) {}
  };
  u32 x568_;
  TUniqueId x56c_ = kInvalidUniqueId;
  u32 x570_ = 0;
  u32 x574_ = 0;
  u32 x578_ = 0;
  u32 x5c4_ = 1;
  bool x5c8_ = false;
  std::vector<CModelData> x5cc_;
  std::vector<CModelData> x5dc_;
  s32 x5ec_ = -1;
  std::unique_ptr<CCollisionActorManager> x5f4_;
  std::unique_ptr<CCollisionActorManager> x5f8_;
  TUniqueId x5fc_ = kInvalidUniqueId;
  CAssetId x600_;
  CAssetId x604_;
  CAssetId x608_;
  TEditorId x60c_ = kInvalidEditorId;
  std::vector<TUniqueId> x610_;
  u32 x624_;
  u32 x628_;
  u32 x62c_;
  CAssetId x630_;
  u32 x634_;
  u32 x638_ = 0;
  u32 x63c_ = 0;
  u32 x640_ = 0;
  s32 x644_ = -1;
  u32 x648_ = 0;
  TUniqueId x64c_ = kInvalidUniqueId;
  zeus::CVector2f x650_ = zeus::CVector2f(0.f, 1.f);
  s32 x658_ = -1;
  s32 x65c_ = -1;
  u32 x660_ = 0;
  u32 x664_ = 0;
  bool x688_ = false;
  bool x690_ = false;
  float x694_;
  float x698_;
  float x69c_;
  float x6a0_;
  float x6a4_;
  float x6a8_;
  float x6ac_;
  std::vector<char> x6b0_; /* TODO: Determine real value */
  std::vector<TUniqueId> x6c0_;
  CAssetId x6d0_;
  CAssetId x6d4_;
  CAssetId x6d8_;
  CAssetId x6dc_;
  CAssetId x6e0_;
  CAssetId x6e4_;
  CAssetId x6e8_;
  s16 x6ec_;
  CAssetId x6f0_;
  u32 x6f4_ = 0;
  float x6f8_ = 0.3f;
  std::array<CThardusSomething, 4> x6fc_;
  zeus::CVector3f x74c_ = zeus::skForward;
  s32 x758_;
  s32 x75c_;
  s32 x760_;
  zeus::CTransform x764_;
  u32 x794_ = 0;
  std::vector<TUniqueId> x798_;
  std::vector<TUniqueId> x7a8_;
  float x7b8_ = 0.f;
  float x7bc_ = 10.f;
  u32 x7c4_ = 0;
  bool x7c8_ = false;
  zeus::CVector3f x7cc_;
  zeus::CVector3f x7d8_;
  zeus::CVector3f x7e4_;
  CPathFindSearch x7f0_;
  bool x8d4_ = false;
  zeus::CVector3f x8d8_;
  zeus::CVector3f x8e4_;
  bool x8f0_ = false;
  std::vector<TUniqueId> x8f4_;
  u32 x904_ = 0;
  bool x908_ = false;
  std::vector<float> x90c_;
  TLockedToken<CTexture> x91c_;
  TUniqueId x928_;
  zeus::CVector3f x92c_;
  bool x938_ = false;
  bool x939_ = false;
  bool x93a_ = false;
  bool x93b_ = false;
  bool x93c_ = false;
  bool x93d_ = true;
  u32 x940_ = 0;
  float x944_ = 0.3f;
  u32 x948_;
  bool x94c_ = false;
  bool x94d_ = false;
  zeus::CVector3f x950_;
  bool x95c_ = false;
  bool x95d_ = false;
  bool x95e_ = false;

public:
  DEFINE_PATTERNED(Thardus)
  CThardus(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
           CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
           std::vector<CModelData> mData1, std::vector<CModelData> mData2, CAssetId particle1, CAssetId particle2,
           CAssetId particle3, float f1, float f2, float f3, float f4, float f5, float f6, CAssetId stateMachine,
           CAssetId particle4, CAssetId particle5, CAssetId particle6, CAssetId particle7, CAssetId particle8,
           CAssetId particle9, CAssetId texture, u32 sfxId1, CAssetId particle10, u32 sfxId2, u32 sfxId3, u32 sfxId4);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override {
    if (msg == EStateMsg::Activate) {
      x5ec_ = 0;
    } else if (msg == EStateMsg::Update) {
      if (x5ec_ == 0) {
        if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Getup) {
          x5ec_ = 2;
        } else {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
        }
      } else if (x5ec_ == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Getup) {
        x5ec_ = 3;
      }
    } else if (msg == EStateMsg::Deactivate) {
      x93d_ = false;
    }
  }

  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override {
    if (msg != EStateMsg::Activate)
      return;
    RemoveMaterial(EMaterialTypes::RadarObject, mgr);
  }
};
} // namespace MP1
} // namespace urde
