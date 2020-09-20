#include "Runtime/World/CScriptMazeNode.hpp"

#include "Runtime/Character/CModelData.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

std::array<s32, 300> CScriptMazeNode::sMazeSeeds{};

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, s32 w1, s32 w2, s32 w3,
                                 const zeus::CVector3f& actorPos, const zeus::CVector3f& triggerPos,
                                 const zeus::CVector3f& effectPos)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_col(w1)
, xec_row(w1)
, xf0_(w2)
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
  x4_arr.fill({});

  std::array<size_t, 4> local_20{};
  size_t i = skMazeColumns * skMazeRows - 1;
  size_t iVar7 = 0;
  while (i != 0) {
    size_t acc = 0;
    if (iVar7 - 9 > 0) {
      auto& cell = x4_arr[iVar7 - skMazeColumns];
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        acc = 1;
        local_20[0] = 0;
      }
    }
    size_t uVar6 = acc;
    size_t iVar8 = iVar7 + 1;
    if (iVar7 < 61 && iVar8 % skMazeColumns != 0) {
      auto& cell = x4_arr[iVar8];
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        uVar6 = acc + 1;
        local_20[acc] = 1;
      }
    }
    acc = uVar6;
    if (iVar7 + skMazeColumns < 63) {
      auto& cell = x4_arr[iVar7 + skMazeColumns];
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        acc = uVar6 + 1;
        local_20[uVar6] = 2;
      }
    }
    uVar6 = acc;
    if (iVar7 > 0 && iVar7 % skMazeColumns != 0) {
      auto& cell = x4_arr[iVar7 - 1];
      if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
        uVar6 = acc + 1;
        local_20[acc] = 3;
      }
    }
    if (uVar6 == 0) {
      bool cont;
      do {
        iVar7++;
        if (iVar7 > 62) {
          iVar7 = 0;
        }
        cont = false;
        auto& cell = x4_arr[iVar7];
        if (!cell.x0_24_ && !cell.x0_25_ && !cell.x0_26_ && !cell.x0_27_) {
          cont = true;
        }
      } while (cont);
    } else {
      i--;
      s32 rand = x0_rand.Next();
      s32 iVar5 = local_20[rand - (rand / uVar6) * uVar6];
      if (iVar5 == 2) {
        x4_arr[iVar7].x0_26_ = true;
        x4_arr[iVar7 + skMazeColumns].x0_24_ = true;
        iVar7 += skMazeColumns;
      } else if (iVar5 == 0) {
        x4_arr[iVar7].x0_24_ = true;
        x4_arr[iVar7 - skMazeColumns].x0_26_ = true;
        iVar7 -= skMazeColumns;
      } else if (iVar5 == 1) {
        x4_arr[iVar7].x0_25_ = true;
        x4_arr[iVar7 + 1].x0_27_ = true;
        iVar7++;
      } else if (iVar5 == 3) {
        x4_arr[iVar7].x0_27_ = true;
        x4_arr[iVar7 - 1].x0_25_ = true;
        iVar7--;
      }
    }
  }
}

void CScriptMazeState::Initialize() {
  std::array<size_t, 66> arr{};
  arr[0] = x84_ + x88_ * skMazeColumns;
  x4_arr[arr[0]].x1_26_ = true;
  size_t i = 1;
  while (arr[0] != x8c_ + x90_ * skMazeColumns) {
    if (x4_arr[arr[0]].x0_24_) {
      if (!x4_arr[arr[0] - skMazeColumns].x1_26_) {
        arr[i] = arr[0] - skMazeColumns;
        i++;
      }
    }
    if (x4_arr[arr[0]].x0_25_) {
      if (!x4_arr[arr[0] + 1].x1_26_) {
        arr[i] = arr[0] + 1;
        i++;
      }
    }
    if (x4_arr[arr[0]].x0_26_) {
      if (!x4_arr[arr[0] + skMazeColumns].x1_26_) {
        arr[i] = arr[0] + skMazeColumns;
        i++;
      }
    }
    if (x4_arr[arr[0]].x0_27_) {
      if (!x4_arr[arr[0] - 1].x1_26_) {
        arr[i] = arr[0] - 1;
        i++;
      }
    }
    if (arr[0] == arr[i - 1]) {
      i--;
    }
    arr[0] = arr[i - 1];
    x4_arr[arr[0]].x1_26_ = true;
  }
  size_t* v = &arr[i];
  while (i != 0) {
    i--;
    v--;
    if (x4_arr[*v].x1_26_) {
      x4_arr[*v].x1_25_ = true;
    }
  }
  x94_24_initialized = true;
}

void CScriptMazeState::sub_802899c8() {
  if (!x94_24_initialized) {
    Initialize();
  }

  s32 iVar5 = x0_rand.Next();
  s32 iVar1 = ((iVar5 / 5) * -5) + 9;
  s32 iVar6 = x0_rand.Next();
  s32 iVar2 = ((iVar6 / 5) * -5) + 21;
  s32 iVar7 = x0_rand.Next();
  s32 iVar3 = ((iVar7 / 5) * -5) + 33;
  s32 iVar16 = ((x0_rand.Next() / 5) * -5) + 13;
  s32 iVar9 = ((x0_rand.Next() / 5) * -5) + 29;

  u32 uVar10 = -1;
  s32 idx = 0;

  s32 prevCol = x84_;
  s32 prevRow = x88_;
  s32 col = x84_;
  s32 row = x88_;

  s32 nextCol;
  while (col != x8c_ || row != x90_) {
    if (idx == iVar5 + iVar1 || idx == iVar6 + iVar2 || idx == iVar7 + iVar3) {
      if (uVar10 == 2) {
        GetCell(col, row).x0_28_ = true;
        GetCell(prevCol, prevRow).x0_30_ = true;
      } else if (uVar10 == 0) {
        GetCell(col, row).x0_30_ = true;
        GetCell(prevCol, prevRow).x0_28_ = true;
      } else if (uVar10 == 1) {
        GetCell(col, row).x0_31_ = true;
        GetCell(prevCol, prevRow).x0_29_ = true;
      } else if (uVar10 == 3) {
        GetCell(col, row).x0_29_ = true;
        GetCell(prevCol, prevRow).x0_31_ = true;
      }
    }
    nextCol = col;

    s32 nextRow;
    if (row < 1 || uVar10 == 2 || !GetCell(col, row).x0_24_ || !GetCell(col, row - 1).x1_25_) {
      if (row < 6 && uVar10 != 0 && GetCell(col, row).x0_26_ && GetCell(col, row + 1).x1_25_) {
        uVar10 = 2;
        nextRow = row + 1;
      } else {
        nextRow = row;
        if (col < 1 || uVar10 == 1 || !GetCell(col, row).x0_27_ || !x4_arr[(col + row * skMazeColumns) - 1].x1_25_) {
          if (col > skMazeRows) {
            return;
          }
          if (uVar10 == 3) {
            return;
          }
          s32 iVar4_ = col + row * skMazeColumns;
          if (!x4_arr[iVar4_].x0_25_) {
            return;
          }
          if (!x4_arr[iVar4_ + 1].x1_25_) {
            return;
          }
          uVar10 = 1;
          nextCol = col + 1;
        } else {
          uVar10 = 3;
          nextCol = col - 1;
        }
      }
    } else {
      uVar10 = 0;
      nextRow = row - 1;
    }
    if (idx == iVar16 || idx == iVar9) {
      if (col == 0 || row == 0 || col == 8 || row == 6) {
        if (idx == iVar16) {
          iVar16++;
        } else {
          iVar9++;
        }
      } else {
        auto& cell = GetCell(col, row);
        cell.x1_24_ = true;
        if (uVar10 == 2) {
          GetCell(nextCol, nextRow).x0_24_ = false;
          cell.x0_26_ = false;
        } else if (uVar10 == 0) {
          GetCell(nextCol, nextRow).x0_26_ = false;
          cell.x0_24_ = false;
        } else if (uVar10 == 1) {
          GetCell(nextCol, nextRow).x0_27_ = false;
          cell.x0_25_ = false;
        } else if (uVar10 == 3) {
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
