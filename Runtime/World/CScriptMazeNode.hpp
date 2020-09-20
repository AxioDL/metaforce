#pragma once

#include <array>
#include <string_view>

#include "Runtime/CRandom16.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
constexpr u32 skMazeRows = 7;
constexpr u32 skMazeColumns = 9;

struct CScriptMazeStateCell {
  bool x0_24_ : 1 = false;
  bool x0_25_ : 1 = false;
  bool x0_26_ : 1 = false;
  bool x0_27_ : 1 = false;
  bool x0_28_ : 1 = false;
  bool x0_29_ : 1 = false;
  bool x0_30_ : 1 = false;
  bool x0_31_ : 1 = false;
  bool x1_24_ : 1 = false;
  bool x1_25_ : 1 = false;
  bool x1_26_ : 1 = false;
};

class CScriptMazeState {
  CRandom16 x0_rand{0};
  std::array<CScriptMazeStateCell, skMazeRows * skMazeColumns> x4_arr{};
  s32 x84_;
  s32 x88_;
  s32 x8c_;
  s32 x90_;
  bool x94_24_initialized : 1 = false;

public:
  CScriptMazeState(s32 w1, s32 w2, s32 w3, s32 w4) : x84_(w1), x88_(w2), x8c_(w3), x90_(w4) {}
  void Reset(s32 seed);
  void Initialize();
  void sub_802899c8();

  [[nodiscard]] CScriptMazeStateCell& GetCell(u32 col, u32 row) { return x4_arr[col + row * skMazeColumns]; }
  [[nodiscard]] const CScriptMazeStateCell& GetCell(u32 col, u32 row) const {
    return x4_arr[col + row * skMazeColumns];
  }
};

class CScriptMazeNode : public CActor {
  static std::array<s32, 300> sMazeSeeds;
  s32 xe8_col;
  s32 xec_row;
  s32 xf0_;
  TUniqueId xf4_ = kInvalidUniqueId;
  float xf8_msgTimer = 0.f;
  TUniqueId xfc_actorId = kInvalidUniqueId;
  zeus::CVector3f x100_actorPos;
  TUniqueId x10c_triggerId = kInvalidUniqueId;
  zeus::CVector3f x110_triggerPos;
  TUniqueId x11c_effectId = kInvalidUniqueId;
  zeus::CVector3f x120_effectPos;
  std::vector<TUniqueId> x12c_;
  bool x13c_24_ : 1 = true;
  bool x13c_25_ : 1 = false;
  bool x13c_26_ : 1 = false;

public:
  CScriptMazeNode(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool, s32, s32, s32,
                  const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  static void LoadMazeSeeds();

private:
  void GenerateObjects(CStateManager& mgr);
  void Reset(CStateManager& mgr);
  void SendScriptMsgs(CStateManager& mgr, EScriptObjectMessage msg);
};
} // namespace urde
