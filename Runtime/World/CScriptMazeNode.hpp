#pragma once

#include <array>
#include <cassert>
#include <string_view>

#include "Runtime/CRandom16.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
constexpr s32 skMazeRows = 7;
constexpr s32 skMazeColumns = 9;

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
  std::array<CScriptMazeStateCell, skMazeRows * skMazeColumns> x4_cells{};
  s32 x84_startCol;
  s32 x88_startRow;
  s32 x8c_endCol;
  s32 x90_endRow;
  bool x94_24_initialized : 1 = false;

public:
  CScriptMazeState(s32 w1, s32 w2, s32 w3, s32 w4) : x84_startCol(w1), x88_startRow(w2), x8c_endCol(w3), x90_endRow(w4) {}
  void Reset(s32 seed);
  void Initialize();
  void sub_802899c8();

  [[nodiscard]] CScriptMazeStateCell& GetCell(u32 col, u32 row) {
#ifndef NDEBUG
    assert(col < skMazeColumns);
    assert(row < skMazeRows);
#endif
    return x4_cells[col + row * skMazeColumns];
  }
  [[nodiscard]] CScriptMazeStateCell& GetCell(u32 idx) {
#ifndef NDEBUG
    assert(idx < x4_cells.size());
#endif
    return x4_cells[idx];
  }
};

class CScriptMazeNode : public CActor {
  s32 xe8_col;
  s32 xec_row;
  s32 xf0_;
  TUniqueId xf4_ = kInvalidUniqueId;
  float xf8_msgTimer = 1.f;
  TUniqueId xfc_actorId = kInvalidUniqueId;
  zeus::CVector3f x100_actorPos;
  TUniqueId x10c_triggerId = kInvalidUniqueId;
  zeus::CVector3f x110_triggerPos;
  TUniqueId x11c_effectId = kInvalidUniqueId;
  zeus::CVector3f x120_effectPos;
  std::vector<TUniqueId> x12c_;
  bool x13c_24_ : 1 = false;
  bool x13c_25_ : 1 = false;
  bool x13c_26_ : 1 = true;

public:
  CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                  bool active, s32 w1, s32 w2, s32 w3, const zeus::CVector3f& actorPos,
                  const zeus::CVector3f& triggerPos, const zeus::CVector3f& effectPos);

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
