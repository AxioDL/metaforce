#pragma once

#include <array>
#include <cassert>
#include <string_view>

#include "Runtime/CRandom16.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
constexpr s32 skMazeCols = 9;
constexpr s32 skMazeRows = 7;
constexpr s32 skEnterCol = 4;
constexpr s32 skEnterRow = 4;
constexpr s32 skTargetCol = 5;
constexpr s32 skTargetRow = 3;

enum class ESide {
  Invalid = -1,
  Top = 0,
  Right = 1,
  Bottom = 2,
  Left = 3,
};

struct SMazeCell {
  bool x0_24_openTop : 1 = false;
  bool x0_25_openRight : 1 = false;
  bool x0_26_openBottom : 1 = false;
  bool x0_27_openLeft : 1 = false;
  bool x0_28_gateTop : 1 = false;
  bool x0_29_gateRight : 1 = false;
  bool x0_30_gateBottom : 1 = false;
  bool x0_31_gateLeft : 1 = false;
  bool x1_24_puddle : 1 = false;
  bool x1_25_onPath : 1 = false;
  bool x1_26_checked : 1 = false;

  [[nodiscard]] constexpr bool IsOpen() const {
    return x0_24_openTop || x0_25_openRight || x0_26_openBottom || x0_27_openLeft;
  }
};

class CMazeState {
  CRandom16 x0_rand{0};
  std::array<SMazeCell, skMazeRows * skMazeCols> x4_cells{};
  s32 x84_enterCol;
  s32 x88_enterRow;
  s32 x8c_targetCol;
  s32 x90_targetRow;
  bool x94_24_initialized : 1 = false;

  std::vector<s32> m_path;
  CLineRenderer m_renderer = {CLineRenderer::EPrimitiveMode::LineStrip, skMazeRows * skMazeCols, {}, true};

public:
  CMazeState(s32 enterCol, s32 enterRow, s32 targetCol, s32 targetRow)
  : x84_enterCol(enterCol), x88_enterRow(enterRow), x8c_targetCol(targetCol), x90_targetRow(targetRow) {}
  void Reset(s32 seed);
  void Initialize();
  void GenerateObstacles();

  void DebugRender();

  [[nodiscard]] SMazeCell& GetCell(u32 col, u32 row) {
#ifndef NDEBUG
    assert(col < skMazeCols);
    assert(row < skMazeRows);
#endif
    return x4_cells[col + row * skMazeCols];
  }
  [[nodiscard]] SMazeCell& GetCell(u32 idx) {
#ifndef NDEBUG
    assert(idx < x4_cells.size());
#endif
    return x4_cells[idx];
  }
};

class CScriptMazeNode : public CActor {
  s32 xe8_col;
  s32 xec_row;
  ESide xf0_side;
  TUniqueId xf4_gateEffectId = kInvalidUniqueId;
  float xf8_msgTimer = 1.f;
  TUniqueId xfc_actorId = kInvalidUniqueId;
  zeus::CVector3f x100_actorPos;
  TUniqueId x10c_triggerId = kInvalidUniqueId;
  zeus::CVector3f x110_triggerPos;
  TUniqueId x11c_effectId = kInvalidUniqueId;
  zeus::CVector3f x120_effectPos;
  std::vector<TUniqueId> x12c_puddleObjectIds;
  bool x13c_24_hasPuddle : 1 = false;
  bool x13c_25_hasGate : 1 = false;
  bool x13c_26_gateActive : 1 = true;

public:
  CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                  bool active, s32 col, s32 row, s32 side, const zeus::CVector3f& actorPos,
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
} // namespace metaforce
