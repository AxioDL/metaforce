#include "Runtime/World/CScriptMazeNode.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

std::array<s32, 300> sMazeSeeds;

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, s32 w1, s32 w2, s32 w3,
                                 const zeus::CVector3f& actorPos, const zeus::CVector3f& triggerPos,
                                 const zeus::CVector3f& effectPos)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_col(w1)
, xec_row(w2)
, xf0_(w3)
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
        if (xf0_ == 0 && cell.x0_24_) {
          if (cell.x0_28_) {
            shouldGenObjs = true;
            x13c_25_ = true;
          }
        } else if (xf0_ == 1 && cell.x0_25_) {
          if (cell.x0_29_) {
            shouldGenObjs = true;
            x13c_25_ = true;
          }
        } else {
          shouldGenObjs = true;
        }
        if (shouldGenObjs) {
          GenerateObjects(mgr);
        }
        if (xf0_ == 1 && cell.x1_24_) {
          x13c_24_ = true;
        }
        if (x13c_25_) {
          const auto origin = GetTranslation();
          for (const auto& conn : GetConnectionList()) {
            if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Activate) {
              continue;
            }

            bool wasGeneratingObject = mgr.GetIsGeneratingObject();
            mgr.SetIsGeneratingObject(true);
            const auto genObj = mgr.GenerateObject(conn.x8_objId);
            mgr.SetIsGeneratingObject(wasGeneratingObject);

            xf4_ = genObj.second;
            if (TCastToPtr<CActor> actor = mgr.ObjectById(genObj.second)) {
              actor->SetTranslation(origin + x120_effectPos);
              mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::Activate);
            }
            break;
          }
        }
        if (x13c_24_) {
          size_t count = 0;
          for (const auto& conn : GetConnectionList()) {
            if ((conn.x0_state == EScriptObjectState::Closed || conn.x0_state == EScriptObjectState::DeactivateState) &&
                conn.x4_msg == EScriptObjectMessage::Activate) {
              count++;
            }
          }
          x12c_.reserve(count);
          for (const auto& conn : GetConnectionList()) {
            if ((conn.x0_state == EScriptObjectState::Closed || conn.x0_state == EScriptObjectState::DeactivateState) &&
                conn.x4_msg == EScriptObjectMessage::Activate) {
              bool wasGeneratingObject = mgr.GetIsGeneratingObject();
              mgr.SetIsGeneratingObject(true);
              const auto genObj = mgr.GenerateObject(conn.x8_objId);
              mgr.SetIsGeneratingObject(wasGeneratingObject);

              x12c_.push_back(genObj.second);
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
      if (x13c_24_ && maze != nullptr && std::any_of(x12c_.cbegin(), x12c_.cend(), [=](auto v) { return v == uid; })) {
        for (const auto& id : x12c_) {
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
            s32 col = xe8_col - 1;
            if (node->xe8_col == col && node->xec_row == xec_row && node->xf0_ == 1) {
              auto& cell = maze->GetCell(col, xec_row);
              if (!cell.x0_25_) {
                cell.x0_25_ = true;
                node->Reset(mgr);
                node->x13c_25_ = false;
              }
            }
            if (node->xe8_col == xe8_col && node->xec_row == xec_row && node->xf0_ == 1) {
              auto& cell = maze->GetCell(xe8_col, xec_row);
              if (!cell.x0_25_) {
                cell.x0_25_ = true;
                node->Reset(mgr);
                node->x13c_25_ = false;
              }
            }
            if (node->xe8_col == xe8_col && node->xec_row == xec_row && node->xf0_ == 0) {
              auto& cell = maze->GetCell(xe8_col, xec_row);
              if (!cell.x0_24_) {
                cell.x0_24_ = true;
                node->Reset(mgr);
                node->x13c_25_ = false;
              }
            }
            if (node->xe8_col == xe8_col && node->xec_row == xec_row + 1 && node->xf0_ == 0) {
              auto& cell = maze->GetCell(xe8_col, xec_row + 1);
              if (!cell.x0_24_) {
                cell.x0_24_ = true;
                node->Reset(mgr);
                node->x13c_25_ = false;
              }
            }
          }
        }
      }
    } else if (msg == EScriptObjectMessage::Deactivate) {
      Reset(mgr);
    } else if (msg == EScriptObjectMessage::InitializedInArea) {
      if (mgr.GetCurrentMaze() == nullptr) {
        auto maze = std::make_unique<CScriptMazeState>(4, 4, 5, 3);
        maze->Reset(sMazeSeeds[mgr.GetActiveRandom()->Next() % sMazeSeeds.size()]);
        maze->Initialize();
        maze->sub_802899c8();
        mgr.SetCurrentMaze(std::move(maze));
      }
    } else if (msg == EScriptObjectMessage::Deleted) {
      mgr.ClearCurrentMaze();
      Reset(mgr);
    }
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptMazeNode::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !x13c_25_) {
    return;
  }
  xf8_msgTimer -= dt;
  if (xf8_msgTimer <= 0.f) {
    xf8_msgTimer = 1.f;
    if (x13c_26_) {
      x13c_26_ = false;
      SendScriptMsgs(mgr, EScriptObjectMessage::Deactivate);
    } else {
      x13c_26_ = true;
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
    if ((scriptEffect || scriptActor || scriptTrigger) && (!scriptEffect || !x13c_25_)) {
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
  mgr.FreeScriptObject(xf4_);
  xf4_ = kInvalidUniqueId;
  xfc_actorId = kInvalidUniqueId;
  x10c_triggerId = kInvalidUniqueId;
  x11c_effectId = kInvalidUniqueId;
}

void CScriptMazeNode::SendScriptMsgs(CStateManager& mgr, EScriptObjectMessage msg) {
  mgr.SendScriptMsg(x11c_effectId, GetUniqueId(), msg);
  mgr.SendScriptMsg(xfc_actorId, GetUniqueId(), msg);
  mgr.SendScriptMsg(x10c_triggerId, GetUniqueId(), msg);
  mgr.SendScriptMsg(xf4_, GetUniqueId(), msg);
}

void CScriptMazeState::Reset(s32 seed) {
  x0_rand.SetSeed(seed);
  x94_24_initialized = false;
  x4_cells.fill({});

  std::array<s32, 4> states{};
  s32 cellIdx = 0;
  for (u32 i = skMazeColumns * skMazeRows - 1; i != 0;) {
    u32 acc = 0;
    if (cellIdx - skMazeColumns > 0) {
      auto& cell = GetCell(cellIdx - skMazeColumns);
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        states[acc++] = 0;
      }
    }
    if (cellIdx < x4_cells.size() - 2 && (cellIdx + 1) % skMazeColumns != 0) {
      auto& cell = GetCell(cellIdx + 1);
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        states[acc++] = 1;
      }
    }
    if (cellIdx + skMazeColumns < x4_cells.size()) {
      auto& cell = GetCell(cellIdx + skMazeColumns);
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        states[acc++] = 2;
      }
    }
    if (cellIdx > 0 && cellIdx % skMazeColumns != 0) {
      auto& cell = GetCell(cellIdx - 1);
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        states[acc++] = 3;
      }
    }
    if (acc == 0) {
      while (true) {
        cellIdx++;
        if (cellIdx > x4_cells.size() - 1) {
          cellIdx = 0;
        }
        auto& cell = GetCell(cellIdx);
        if (cell.x0_24_ || cell.x0_25_ || cell.x0_26_ || cell.x0_27_) {
          break;
        }
      };
    } else {
      i--;
      s32 state = states[x0_rand.Next() % acc];
      if (state == 2) {
        GetCell(cellIdx).x0_26_ = true;
        GetCell(cellIdx + skMazeColumns).x0_24_ = true;
        cellIdx += skMazeColumns;
      } else if (state == 0) {
        GetCell(cellIdx).x0_24_ = true;
        GetCell(cellIdx - skMazeColumns).x0_26_ = true;
        cellIdx -= skMazeColumns;
      } else if (state == 1) {
        GetCell(cellIdx).x0_25_ = true;
        GetCell(cellIdx + 1).x0_27_ = true;
        cellIdx++;
      } else if (state == 3) {
        GetCell(cellIdx).x0_27_ = true;
        GetCell(cellIdx - 1).x0_25_ = true;
        cellIdx--;
      }
    }
  }
}

void CScriptMazeState::Initialize() {
  std::array<s32, skMazeRows * skMazeColumns> arr{};
  arr[0] = x84_startCol + x88_startRow * skMazeColumns;
  GetCell(arr[0]).x1_26_ = true;
  s32 i = 1;
  while (arr[0] != x8c_endCol + x90_endRow * skMazeColumns) {
    if (GetCell(arr[0]).x0_24_) {
      if (!GetCell(arr[0] - skMazeColumns).x1_26_) {
        arr[i] = arr[0] - skMazeColumns;
        i++;
      }
    }
    if (GetCell(arr[0]).x0_25_) {
      if (!GetCell(arr[0] + 1).x1_26_) {
        arr[i] = arr[0] + 1;
        i++;
      }
    }
    if (GetCell(arr[0]).x0_26_) {
      if (!GetCell(arr[0] + skMazeColumns).x1_26_) {
        arr[i] = arr[0] + skMazeColumns;
        i++;
      }
    }
    if (GetCell(arr[0]).x0_27_) {
      if (!GetCell(arr[0] - 1).x1_26_) {
        arr[i] = arr[0] - 1;
        i++;
      }
    }
    if (arr[0] == arr[i - 1]) {
      i--;
    }
    arr[0] = arr[i - 1];
    GetCell(arr[0]).x1_26_ = true;
  }
  s32* v = &arr[i];
  while (i != 0) {
    i--;
    v--;
    if (GetCell(*v).x1_26_) {
      GetCell(*v).x1_25_ = true;
    }
  }
  x94_24_initialized = true;
}

void CScriptMazeState::sub_802899c8() {
  if (!x94_24_initialized) {
    Initialize();
  }

  auto GetRandom = [this](s32 offset) constexpr {
    s32 tmp = x0_rand.Next();
    return tmp + ((tmp / 5) * -5) + offset;
  };
  s32 rand1 = GetRandom(9);
  s32 rand2 = GetRandom(21);
  s32 rand3 = GetRandom(33);
  s32 rand4 = GetRandom(13);
  s32 rand5 = GetRandom(29);

  u32 state = -1;
  s32 idx = 0;

  s32 prevCol = x84_startCol;
  s32 prevRow = x88_startRow;
  s32 col = x84_startCol;
  s32 row = x88_startRow;

  while (col != x8c_endCol || row != x90_endRow) {
    if (idx == rand1 || idx == rand2 || idx == rand3) {
      if (state == 2) {
        GetCell(col, row).x0_28_ = true;
        GetCell(prevCol, prevRow).x0_30_ = true;
      } else if (state == 0) {
        GetCell(col, row).x0_30_ = true;
        GetCell(prevCol, prevRow).x0_28_ = true;
      } else if (state == 1) {
        GetCell(col, row).x0_31_ = true;
        GetCell(prevCol, prevRow).x0_29_ = true;
      } else if (state == 3) {
        GetCell(col, row).x0_29_ = true;
        GetCell(prevCol, prevRow).x0_31_ = true;
      }
    }
    s32 nextCol = col;

    s32 nextRow = -1;
    if (row < 1 || state == 2 || !GetCell(col, row).x0_24_ || !GetCell(col, row - 1).x1_25_) {
      if (row < 6 && state != 0 && GetCell(col, row).x0_26_ && GetCell(col, row + 1).x1_25_) {
        state = 2;
        nextRow = row + 1;
      } else {
        nextRow = row;
        if (col < 1 || state == 1 || !GetCell(col, row).x0_27_ || !GetCell((col + row * skMazeColumns) - 1).x1_25_) {
          if (col > skMazeRows) {
            return;
          }
          if (state == 3) {
            return;
          }
          s32 iVar4_ = col + row * skMazeColumns;
          if (!GetCell(iVar4_).x0_25_) {
            return;
          }
          if (!GetCell(iVar4_ + 1).x1_25_) {
            return;
          }
          state = 1;
          nextCol = col + 1;
        } else {
          state = 3;
          nextCol = col - 1;
        }
      }
    } else {
      state = 0;
      nextRow = row - 1;
    }
    if (idx == rand4 || idx == rand5) {
      if (col == 0 || row == 0 || col == 8 || row == 6) {
        if (idx == rand4) {
          rand4++;
        } else {
          rand5++;
        }
      } else {
        auto& cell = GetCell(col, row);
        cell.x1_24_ = true;
        if (state == 2) {
          GetCell(nextCol, nextRow).x0_24_ = false;
          cell.x0_26_ = false;
        } else if (state == 0) {
          GetCell(nextCol, nextRow).x0_26_ = false;
          cell.x0_24_ = false;
        } else if (state == 1) {
          GetCell(nextCol, nextRow).x0_27_ = false;
          cell.x0_25_ = false;
        } else if (state == 3) {
          GetCell(nextCol, nextRow).x0_25_ = false;
          cell.x0_27_ = false;
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
} // namespace urde
