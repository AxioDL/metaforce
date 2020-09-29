#include "Runtime/World/CScriptMazeNode.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

std::array<s32, 300> sMazeSeeds;

#ifndef NDEBUG
std::array<zeus::CVector3f, skMazeRows * skMazeCols> sDebugCellPos;
#endif

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, s32 col, s32 row, s32 side,
                                 const zeus::CVector3f& actorPos, const zeus::CVector3f& triggerPos,
                                 const zeus::CVector3f& effectPos)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_col(col)
, xec_row(row)
, xf0_side(static_cast<ESide>(side))
, x100_actorPos(actorPos)
, x110_triggerPos(triggerPos)
, x120_effectPos(effectPos) {}

void CScriptMazeNode::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptMazeNode::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (GetActive()) {
    if (msg == EScriptObjectMessage::Action) {
      if (auto* maze = mgr.GetCurrentMaze()) {
        bool shouldGenObjs = false;
        auto& cell = maze->GetCell(xe8_col, xec_row);
        if (xf0_side == ESide::Top && cell.x0_24_openTop) {
          if (cell.x0_28_gateTop) {
            shouldGenObjs = true;
            x13c_25_hasGate = true;
          }
        } else if (xf0_side == ESide::Right && cell.x0_25_openRight) {
          if (cell.x0_29_gateRight) {
            shouldGenObjs = true;
            x13c_25_hasGate = true;
          }
        } else {
          shouldGenObjs = true;
        }
        if (shouldGenObjs) {
          GenerateObjects(mgr);
        }
        if (xf0_side == ESide::Right && cell.x1_24_puddle) {
          x13c_24_hasPuddle = true;
        }
        if (x13c_25_hasGate) {
          const auto origin = GetTranslation();
          for (const auto& conn : GetConnectionList()) {
            if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Activate) {
              continue;
            }

            bool wasGeneratingObject = mgr.GetIsGeneratingObject();
            mgr.SetIsGeneratingObject(true);
            const auto genObj = mgr.GenerateObject(conn.x8_objId);
            mgr.SetIsGeneratingObject(wasGeneratingObject);

            xf4_gateEffectId = genObj.second;
            if (TCastToPtr<CActor> actor = mgr.ObjectById(genObj.second)) {
              actor->SetTranslation(origin + x120_effectPos);
              mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::Activate);
            }
            break;
          }
        }
        if (x13c_24_hasPuddle) {
          size_t count = 0;
          for (const auto& conn : GetConnectionList()) {
            if ((conn.x0_state == EScriptObjectState::Closed || conn.x0_state == EScriptObjectState::DeactivateState) &&
                conn.x4_msg == EScriptObjectMessage::Activate) {
              count++;
            }
          }
          x12c_puddleObjectIds.reserve(count);
          for (const auto& conn : GetConnectionList()) {
            if ((conn.x0_state == EScriptObjectState::Closed || conn.x0_state == EScriptObjectState::DeactivateState) &&
                conn.x4_msg == EScriptObjectMessage::Activate) {
              bool wasGeneratingObject = mgr.GetIsGeneratingObject();
              mgr.SetIsGeneratingObject(true);
              const auto genObj = mgr.GenerateObject(conn.x8_objId);
              mgr.SetIsGeneratingObject(wasGeneratingObject);

              x12c_puddleObjectIds.push_back(genObj.second);
              if (TCastToPtr<CActor> actor = mgr.ObjectById(genObj.second)) {
                actor->SetTransform(GetTransform());
                if (conn.x0_state == EScriptObjectState::Closed) {
                  mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::Activate);
                }
              }
            }
          }
        }
      }
    } else if (msg == EScriptObjectMessage::SetToZero) {
      auto* maze = mgr.GetCurrentMaze();
      if (x13c_24_hasPuddle && maze != nullptr &&
          std::any_of(x12c_puddleObjectIds.cbegin(), x12c_puddleObjectIds.cend(), [=](auto v) { return v == uid; })) {
        for (const auto& id : x12c_puddleObjectIds) {
          if (auto* ent = mgr.ObjectById(id)) {
            if (ent->GetActive()) {
              mgr.SendScriptMsg(ent, GetUniqueId(), EScriptObjectMessage::Activate);
            } else {
              mgr.FreeScriptObject(ent->GetUniqueId());
            }
          }
        }
        for (const auto& ent : mgr.GetAllObjectList()) {
          if (TCastToPtr<CScriptMazeNode> node = ent) {
            if (node->xe8_col == xe8_col - 1 && node->xec_row == xec_row && node->xf0_side == ESide::Right) {
              auto& cell = maze->GetCell(xe8_col - 1, xec_row);
              if (!cell.x0_25_openRight) {
                cell.x0_25_openRight = true;
                node->Reset(mgr);
                node->x13c_25_hasGate = false;
              }
            }
            if (node->xe8_col == xe8_col && node->xec_row == xec_row && node->xf0_side == ESide::Right) {
              auto& cell = maze->GetCell(xe8_col, xec_row);
              if (!cell.x0_25_openRight) {
                cell.x0_25_openRight = true;
                node->Reset(mgr);
                node->x13c_25_hasGate = false;
              }
            }
            if (node->xe8_col == xe8_col && node->xec_row == xec_row && node->xf0_side == ESide::Top) {
              auto& cell = maze->GetCell(xe8_col, xec_row);
              if (!cell.x0_24_openTop) {
                cell.x0_24_openTop = true;
                node->Reset(mgr);
                node->x13c_25_hasGate = false;
              }
            }
            if (node->xe8_col == xe8_col && node->xec_row == xec_row + 1 && node->xf0_side == ESide::Top) {
              auto& cell = maze->GetCell(xe8_col, xec_row + 1);
              if (!cell.x0_24_openTop) {
                cell.x0_24_openTop = true;
                node->Reset(mgr);
                node->x13c_25_hasGate = false;
              }
            }
          }
        }
      }
    } else if (msg == EScriptObjectMessage::Deactivate) {
      Reset(mgr);
    } else if (msg == EScriptObjectMessage::InitializedInArea) {
      if (mgr.GetCurrentMaze() == nullptr) {
        auto maze = std::make_unique<CMazeState>(skEnterCol, skEnterRow, skTargetCol, skTargetRow);
        maze->Reset(sMazeSeeds[mgr.GetActiveRandom()->Next() % sMazeSeeds.size()]);
        maze->Initialize();
        maze->GenerateObstacles();
        mgr.SetCurrentMaze(std::move(maze));
      }
#ifndef NDEBUG
      if (xf0_side == ESide::Right) {
        sDebugCellPos[xe8_col + xec_row * skMazeCols] = GetTranslation();
      } else if (xe8_col == skMazeCols - 1) {
        // Last column does not have right nodes, but we can infer the position
        sDebugCellPos[xe8_col + xec_row * skMazeCols] = GetTranslation() - zeus::CVector3f{1.1875f, -0.1215f, 1.2187f};
      }
#endif
    }
  }
  // URDE change: used to be in the above if branch
  if (msg == EScriptObjectMessage::Deleted) {
    mgr.ClearCurrentMaze();
    Reset(mgr);
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptMazeNode::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !x13c_25_hasGate) {
    return;
  }
  xf8_msgTimer -= dt;
  if (xf8_msgTimer <= 0.f) {
    xf8_msgTimer = 1.f;
    if (x13c_26_gateActive) {
      x13c_26_gateActive = false;
      SendScriptMsgs(mgr, EScriptObjectMessage::Deactivate);
    } else {
      x13c_26_gateActive = true;
      SendScriptMsgs(mgr, EScriptObjectMessage::Activate);
    }
  }
}

void CScriptMazeNode::LoadMazeSeeds() {
  const SObjectTag* tag = g_ResFactory->GetResourceIdByName("DUMB_MazeSeeds");
  const u32 resSize = g_ResFactory->ResourceSize(*tag);
  const std::unique_ptr<u8[]> buf = g_ResFactory->LoadResourceSync(*tag);
  CMemoryInStream in(buf.get(), resSize);
  for (auto& seed : sMazeSeeds) {
    seed = in.readInt32Big();
  }
}

void CScriptMazeNode::GenerateObjects(CStateManager& mgr) {
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state != EScriptObjectState::MaxReached || conn.x4_msg != EScriptObjectMessage::Activate) {
      continue;
    }
    const auto* ent = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId));
    TCastToConstPtr<CScriptEffect> scriptEffect{ent};
    TCastToConstPtr<CScriptActor> scriptActor{ent};
    TCastToConstPtr<CScriptTrigger> scriptTrigger{ent};
    if ((scriptEffect || scriptActor || scriptTrigger) && (!scriptEffect || !x13c_25_hasGate)) {
      bool wasGeneratingObject = mgr.GetIsGeneratingObject();
      mgr.SetIsGeneratingObject(true);
      const auto genObj = mgr.GenerateObject(conn.x8_objId);
      mgr.SetIsGeneratingObject(wasGeneratingObject);
      if (auto* actor = static_cast<CActor*>(mgr.ObjectById(genObj.second))) {
        mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::Activate);
        if (scriptEffect) {
          actor->SetTranslation(GetTranslation() + x120_effectPos);
          x11c_effectId = genObj.second;
        }
        if (scriptActor) {
          actor->SetTranslation(GetTranslation() + x100_actorPos);
          xfc_actorId = genObj.second;
        }
        if (scriptTrigger) {
          actor->SetTranslation(GetTranslation() + x110_triggerPos);
          x10c_triggerId = genObj.second;
        }
      }
    }
  }
}

void CScriptMazeNode::Reset(CStateManager& mgr) {
  mgr.FreeScriptObject(x11c_effectId);
  mgr.FreeScriptObject(xfc_actorId);
  mgr.FreeScriptObject(x10c_triggerId);
  mgr.FreeScriptObject(xf4_gateEffectId);
  xf4_gateEffectId = kInvalidUniqueId;
  xfc_actorId = kInvalidUniqueId;
  x10c_triggerId = kInvalidUniqueId;
  x11c_effectId = kInvalidUniqueId;
}

void CScriptMazeNode::SendScriptMsgs(CStateManager& mgr, EScriptObjectMessage msg) {
  mgr.SendScriptMsg(x11c_effectId, GetUniqueId(), msg);
  mgr.SendScriptMsg(xfc_actorId, GetUniqueId(), msg);
  mgr.SendScriptMsg(x10c_triggerId, GetUniqueId(), msg);
  mgr.SendScriptMsg(xf4_gateEffectId, GetUniqueId(), msg);
}

void CMazeState::Reset(s32 seed) {
  x0_rand.SetSeed(seed);
  x94_24_initialized = false;
  x4_cells.fill({});

  std::array<ESide, 4> sides{};
  s32 cellIdx = 0;
  for (u32 i = skMazeCols * skMazeRows - 1; i != 0;) {
    u32 acc = 0;
    if (cellIdx - skMazeCols > 0 && !GetCell(cellIdx - skMazeCols).IsOpen()) {
      sides[acc++] = ESide::Top;
    }
    if (cellIdx < x4_cells.size() - 2 && (cellIdx + 1) % skMazeCols != 0 && !GetCell(cellIdx + 1).IsOpen()) {
      sides[acc++] = ESide::Right;
    }
    if (cellIdx + skMazeCols < x4_cells.size() && !GetCell(cellIdx + skMazeCols).IsOpen()) {
      sides[acc++] = ESide::Bottom;
    }
    if (cellIdx > 0 && cellIdx % skMazeCols != 0 && !GetCell(cellIdx - 1).IsOpen()) {
      sides[acc++] = ESide::Left;
    }

    if (acc == 0) {
      do {
        cellIdx++;
        if (cellIdx > x4_cells.size() - 1) {
          cellIdx = 0;
        }
      } while (!GetCell(cellIdx).IsOpen());
      continue;
    }

    i--;
    ESide side = sides[x0_rand.Next() % acc];
    if (side == ESide::Bottom) {
      GetCell(cellIdx).x0_26_openBottom = true;
      GetCell(cellIdx + skMazeCols).x0_24_openTop = true;
      cellIdx += skMazeCols;
    } else if (side == ESide::Top) {
      GetCell(cellIdx).x0_24_openTop = true;
      GetCell(cellIdx - skMazeCols).x0_26_openBottom = true;
      cellIdx -= skMazeCols;
    } else if (side == ESide::Right) {
      GetCell(cellIdx).x0_25_openRight = true;
      GetCell(cellIdx + 1).x0_27_openLeft = true;
      cellIdx++;
    } else if (side == ESide::Left) {
      GetCell(cellIdx).x0_27_openLeft = true;
      GetCell(cellIdx - 1).x0_25_openRight = true;
      cellIdx--;
    }
  }
}

void CMazeState::Initialize() {
  std::array<s32, skMazeRows * skMazeCols> path{};
  path[0] = x84_enterCol + x88_enterRow * skMazeCols;
  GetCell(path[0]).x1_26_checked = true;
  s32 pathLength = 1;
  while (path[0] != x8c_targetCol + x90_targetRow * skMazeCols) {
    if (GetCell(path[0]).x0_24_openTop) {
      if (!GetCell(path[0] - skMazeCols).x1_26_checked) {
        path[pathLength] = path[0] - skMazeCols;
        pathLength++;
      }
    }
    if (GetCell(path[0]).x0_25_openRight) {
      if (!GetCell(path[0] + 1).x1_26_checked) {
        path[pathLength] = path[0] + 1;
        pathLength++;
      }
    }
    if (GetCell(path[0]).x0_26_openBottom) {
      if (!GetCell(path[0] + skMazeCols).x1_26_checked) {
        path[pathLength] = path[0] + skMazeCols;
        pathLength++;
      }
    }
    if (GetCell(path[0]).x0_27_openLeft) {
      if (!GetCell(path[0] - 1).x1_26_checked) {
        path[pathLength] = path[0] - 1;
        pathLength++;
      }
    }
    if (path[0] == path[pathLength - 1]) {
      pathLength--;
    }
    path[0] = path[pathLength - 1];
    GetCell(path[0]).x1_26_checked = true;
  }
  s32* idx = &path[pathLength];
  while (pathLength != 0) {
    pathLength--;
    idx--;
    auto& cell = GetCell(*idx);
    if (cell.x1_26_checked) {
      cell.x1_25_onPath = true;
#ifndef NDEBUG
      if (pathLength > 0) {
        m_path.push_back(*idx);
      }
#endif
    }
  }
  x94_24_initialized = true;
}

void CMazeState::GenerateObstacles() {
  if (!x94_24_initialized) {
    Initialize();
  }

  auto GetRandom = [this](s32 offset) constexpr {
    s32 tmp = x0_rand.Next();
    return tmp + ((tmp / 5) * -5) + offset;
  };
  s32 gate1Idx = GetRandom(9);
  s32 gate2Idx = GetRandom(21);
  s32 gate3Idx = GetRandom(33);
  s32 puddle1Idx = GetRandom(13);
  s32 puddle2Idx = GetRandom(29);

  ESide side = ESide::Invalid;
  s32 idx = 0;

  s32 prevCol = x84_enterCol;
  s32 prevRow = x88_enterRow;
  s32 col = x84_enterCol;
  s32 row = x88_enterRow;

  while (col != x8c_targetCol || row != x90_targetRow) {
    if (idx == gate1Idx || idx == gate2Idx || idx == gate3Idx) {
      if (side == ESide::Bottom) {
        GetCell(col, row).x0_28_gateTop = true;
        GetCell(prevCol, prevRow).x0_30_gateBottom = true;
      } else if (side == ESide::Top) {
        GetCell(col, row).x0_30_gateBottom = true;
        GetCell(prevCol, prevRow).x0_28_gateTop = true;
      } else if (side == ESide::Right) {
        GetCell(col, row).x0_31_gateLeft = true;
        GetCell(prevCol, prevRow).x0_29_gateRight = true;
      } else if (side == ESide::Left) {
        GetCell(col, row).x0_29_gateRight = true;
        GetCell(prevCol, prevRow).x0_31_gateLeft = true;
      }
    }

    s32 nextCol = col;
    s32 nextRow = -1;
    if (row < 1 || side == ESide::Bottom || !GetCell(col, row).x0_24_openTop || !GetCell(col, row - 1).x1_25_onPath) {
      if (row < skMazeRows - 1 && side != ESide::Top && GetCell(col, row).x0_26_openBottom &&
          GetCell(col, row + 1).x1_25_onPath) {
        side = ESide::Bottom;
        nextRow = row + 1;
      } else {
        nextRow = row;
        if (col < 1 || side == ESide::Right || !GetCell(col, row).x0_27_openLeft ||
            !GetCell(col - 1, row).x1_25_onPath) {
          if (col > skMazeRows || side == ESide::Left || !GetCell(col, row).x0_25_openRight ||
              !GetCell(col + 1, row).x1_25_onPath) {
            return;
          }
          side = ESide::Right;
          nextCol = col + 1;
        } else {
          side = ESide::Left;
          nextCol = col - 1;
        }
      }
    } else {
      side = ESide::Top;
      nextRow = row - 1;
    }

    if (idx == puddle1Idx || idx == puddle2Idx) {
      if (col == 0 || row == 0 || col == skMazeCols - 1 || row == skMazeRows - 1) {
        if (idx == puddle1Idx) {
          puddle1Idx++;
        } else {
          puddle2Idx++;
        }
      } else {
        auto& cell = GetCell(col, row);
        cell.x1_24_puddle = true;
        if (side == ESide::Bottom) {
          GetCell(nextCol, nextRow).x0_24_openTop = false;
          cell.x0_26_openBottom = false;
        } else if (side == ESide::Top) {
          GetCell(nextCol, nextRow).x0_26_openBottom = false;
          cell.x0_24_openTop = false;
        } else if (side == ESide::Right) {
          GetCell(nextCol, nextRow).x0_27_openLeft = false;
          cell.x0_25_openRight = false;
        } else if (side == ESide::Left) {
          GetCell(nextCol, nextRow).x0_25_openRight = false;
          cell.x0_27_openLeft = false;
        }
      }
    }

    idx++;
    prevCol = col;
    prevRow = row;
    col = nextCol;
    row = nextRow;
  };
}

#ifndef NDEBUG
void CMazeState::DebugRender() {
  m_renderer.Reset();
  m_renderer.AddVertex(sDebugCellPos[skEnterCol + skEnterRow * skMazeCols], zeus::skBlue, 2.f);
  for (s32 i = m_path.size() - 1; i >= 0; --i) {
    s32 idx = m_path[i];
    zeus::CVector3f pos;
    if (idx == skMazeCols - 1) {
      // 8,0 has no node, infer from 8,1
      pos = sDebugCellPos[idx + skMazeCols] + zeus::CVector3f{4.f, 0.f, 0.f};
    } else {
      pos = sDebugCellPos[idx];
    }
    m_renderer.AddVertex(pos, zeus::skBlue, 2.f);
  }
  m_renderer.Render();
}
#endif
} // namespace urde
