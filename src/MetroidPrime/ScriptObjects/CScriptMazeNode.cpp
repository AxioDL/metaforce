#include "MetroidPrime/ScriptObjects/CScriptMazeNode.hpp"

#include "Kyoto/CResFactory.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CObjectList.hpp"

#include "rstl/algorithm.hpp"

uint CScriptMazeNode::sMazeSeeds[300];

CSinglePathMaze::CSinglePathMaze(int enterCol, int enterRow, int targetCol, int targetRow)
: mRand(0)
, mEnterCol(enterCol)
, mEnterRow(enterRow)
, mTargetCol(targetCol)
, mTargetRow(targetRow)
, mInitialized(false) {}

void CSinglePathMaze::CreateMaze(int seed) {
  int numCells = NUM_MAZE_CELLS - 1;
  int cellIdx = 0;

  mRand = CRandom16(seed);
  mInitialized = false;
  for (int i = 0; i < NUM_MAZE_CELLS; i++) {
    mCells[i] = SMazeCell();
  }

  ESide sides[4];
  while (numCells != 0) {
    int acc = 0;
    if (cellIdx - skMazeCols > 0 && MazePoint(cellIdx - skMazeCols).IsClosed()) {
      sides[acc++] = kS_Top;
    }
    if (cellIdx < NUM_MAZE_CELLS - 2 && (cellIdx + 1) % skMazeCols != 0 &&
        MazePoint(cellIdx + 1).IsClosed()) {
      sides[acc++] = kS_Right;
    }
    if (cellIdx + skMazeCols <= NUM_MAZE_CELLS - 1 && MazePoint(cellIdx + skMazeCols).IsClosed()) {
      sides[acc++] = kS_Bottom;
    }
    if (cellIdx > 0 && cellIdx % skMazeCols != 0 && MazePoint(cellIdx - 1).IsClosed()) {
      sides[acc++] = kS_Left;
    }

    if (acc != 0) {
      numCells--;

      ESide side = sides[mRand.Next() % static_cast< uint >(acc)];
      switch (side) {
      case kS_Top:
        MazePoint(cellIdx).mOpenTop = true;
        MazePoint(cellIdx - skMazeCols).mOpenBottom = true;
        cellIdx -= skMazeCols;
        break;
      case kS_Right:
        MazePoint(cellIdx).mOpenRight = true;
        MazePoint(cellIdx + 1).mOpenLeft = true;
        cellIdx++;
        break;
      case kS_Bottom:
        MazePoint(cellIdx).mOpenBottom = true;
        MazePoint(cellIdx + skMazeCols).mOpenTop = true;
        cellIdx += skMazeCols;
        break;
      case kS_Left:
        MazePoint(cellIdx).mOpenLeft = true;
        MazePoint(cellIdx - 1).mOpenRight = true;
        cellIdx--;
        break;
      }
      continue;
    }

    do {
      cellIdx++;
      if (cellIdx > NUM_MAZE_CELLS - 1) {
        cellIdx = 0;
      }
    } while (MazePoint(cellIdx).IsClosed());
  }
}

const SMazeCell& CSinglePathMaze::GetMazePoint(uint col, uint row) const {
  return mCells[col + row * skMazeCols];
}

SMazeCell& CSinglePathMaze::MazePoint(uint col, uint row) {
  return mCells[col + row * skMazeCols];
}

static inline int GetRandom(CRandom16& rand, int offset) {
  int tmp = rand.Next();
  return tmp - ((tmp / 5) * 5) + offset;
}

void CSinglePathMaze::AddGimmicks() {
  if (!mInitialized) {
    SolveMaze();
  }

  int gate1Idx = GetRandom(mRand, 9);
  int gate2Idx = GetRandom(mRand, 21);
  int gate3Idx = GetRandom(mRand, 33);
  int puddle1Idx = GetRandom(mRand, 13);
  int puddle2Idx = GetRandom(mRand, 29);

  int prevCol = mEnterCol;
  int prevRow = mEnterRow;
  ESide side = kS_Invalid;
  int idx = 0;
  int col = prevCol;
  int row = prevRow;

  while (col != mTargetCol || row != mTargetRow) {
    if (idx == gate1Idx || idx == gate2Idx || idx == gate3Idx) {
      switch (side) {
      case kS_Top:
        MazePointInline(col, row).mGateBottom = true;
        MazePointInline(prevCol, prevRow).mGateTop = true;
        break;
      case kS_Right:
        MazePointInline(col, row).mGateLeft = true;
        MazePointInline(prevCol, prevRow).mGateRight = true;
        break;
      case kS_Bottom:
        MazePointInline(col, row).mGateTop = true;
        MazePointInline(prevCol, prevRow).mGateBottom = true;
        break;
      case kS_Left:
        MazePointInline(col, row).mGateRight = true;
        MazePointInline(prevCol, prevRow).mGateLeft = true;
        break;
      }
    }

    int curCol = col;
    int curRow = row;
    if (row > 0 && side != kS_Bottom && MazePointInline(col, row).mOpenTop &&
        MazePointInline(col, row - 1).mOnPath) {
      side = kS_Top;
      row--;
    } else if (row < skMazeRows - 1 && side != kS_Top &&
               MazePointInline(col, row).mOpenBottom &&
               MazePointInline(col, row + 1).mOnPath) {
      side = kS_Bottom;
      row++;
    } else if (col > 0 && side != kS_Right && MazePointInline(col, row).mOpenLeft &&
               MazePointInline(col - 1, row).mOnPath) {
      side = kS_Left;
      col--;
    } else if (col < skMazeCols - 1 && side != kS_Left &&
               MazePointInline(col, row).mOpenRight &&
               MazePointInline(col + 1, row).mOnPath) {
      side = kS_Right;
      col++;
    } else {
      return;
    }

#define MazePointInline(c, r) mCells[c + r * skMazeCols]
    if (idx == puddle1Idx || idx == puddle2Idx) {
      if (curCol == 0 || curRow == 0 || curCol == skMazeCols - 1 || curRow == skMazeRows - 1) {
        if (idx == puddle1Idx) {
          puddle1Idx++;
        } else {
          puddle2Idx++;
        }
      } else {
        MazePointInline(curCol, curRow).mPuddle = true;
        switch (side) {
        case kS_Top:
          MazePointInline(col, row).mOpenBottom = false;
          MazePointInline(curCol, curRow).mOpenTop = false;
          break;
        case kS_Right:
          MazePointInline(col, row).mOpenLeft = false;
          MazePointInline(curCol, curRow).mOpenRight = false;
          break;
        case kS_Bottom:
          MazePointInline(col, row).mOpenTop = false;
          MazePointInline(curCol, curRow).mOpenBottom = false;
          break;
        case kS_Left:
          MazePointInline(col, row).mOpenRight = false;
          MazePointInline(curCol, curRow).mOpenLeft = false;
          break;
        }
      }
    }
#undef MazePointInline

    idx++;
    prevCol = curCol;
    prevRow = curRow;
  }
}

void CSinglePathMaze::SolveMaze() {
  int path[NUM_MAZE_CELLS];
  int pathIdx = 0;
  int targetRow = mTargetRow;
  int targetCol = mTargetCol;
  int cellIdx = mEnterCol + mEnterRow * skMazeCols;
  int max = targetCol + targetRow * skMazeCols;

  path[pathIdx++] = cellIdx;
  mCells[cellIdx].mChecked = true;

  while (cellIdx != max) {
    if (mCells[cellIdx].mOpenTop && !mCells[cellIdx - skMazeCols].mChecked) {
      path[pathIdx++] = cellIdx - skMazeCols;
    }
    if (mCells[cellIdx].mOpenRight && !mCells[cellIdx + 1].mChecked) {
      path[pathIdx++] = cellIdx + 1;
    }
    if (mCells[cellIdx].mOpenBottom && !mCells[cellIdx + skMazeCols].mChecked) {
      path[pathIdx++] = cellIdx + skMazeCols;
    }
    if (mCells[cellIdx].mOpenLeft && !mCells[cellIdx - 1].mChecked) {
      path[pathIdx++] = cellIdx - 1;
    }
    if (cellIdx == path[pathIdx - 1]) {
      pathIdx--;
    }
    cellIdx = path[pathIdx - 1];
    mCells[cellIdx].mChecked = true;
  }

  while (pathIdx--) {
    cellIdx = path[pathIdx];
    if (mCells[cellIdx].mChecked) {
      mCells[cellIdx].mOnPath = true;
    }
  }

  mInitialized = true;
}

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& xf, const bool active, int col, int row,
                                 int side, const CVector3f& actorPos, const CVector3f& triggerPos,
                                 const CVector3f& effectPos)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mCol(col)
, mRow(row)
, mSide(static_cast< ESide >(side))
, mGateEffectId(kInvalidUniqueId)
, mMsgTimer(1.f)
, mActorId(kInvalidUniqueId)
, mActorPos(actorPos)
, mTriggerId(kInvalidUniqueId)
, mTriggerPos(triggerPos)
, mEffectId(kInvalidUniqueId)
, mEffectPos(effectPos)
, mHasPuddle(false)
, mHasGate(false)
, mGateActive(true) {}

ENTITY_ACCEPT_IMPL(CScriptMazeNode)

void CScriptMazeNode::GenerateBarrier(CStateManager& mgr) {
  rstl::vector< SConnection >::iterator conn = ConnectionList().begin();
  for (; conn != ConnectionList().end(); ++conn) {
    if (conn->mState != kSS_MaxReached || conn->mMsg != kSM_Activate) {
      continue;
    }

    CEntity* ent = mgr.ObjectById(mgr.GetIdForScript(conn->mObjId));
    CScriptEffect* scriptEffect = TCastToPtr< CScriptEffect >(ent);
    CScriptActor* scriptActor = TCastToPtr< CScriptActor >(ent);
    CScriptTrigger* scriptTrigger = TCastToPtr< CScriptTrigger >(ent);
    if (!scriptEffect && !scriptActor && !scriptTrigger) {
      continue;
    }
    if (scriptEffect && mHasGate) {
      continue;
    }

    bool wasGeneratingObject = mgr.IsGeneratingObject();
    mgr.SetIsGeneratingObject(true);
    TUniqueId objUid = mgr.GenerateObject(conn->mObjId).second;
    mgr.SetIsGeneratingObject(wasGeneratingObject);

    if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(objUid))) {
      mgr.DeliverScriptMsg(actor, GetUniqueId(), kSM_Activate);
      if (scriptEffect) {
        actor->SetTranslation(GetTranslation() + mEffectPos);
        mEffectId = objUid;
      }
      if (scriptActor) {
        actor->SetTranslation(GetTranslation() + mActorPos);
        mActorId = objUid;
      }
      if (scriptTrigger) {
        actor->SetTranslation(GetTranslation() + mTriggerPos);
        mTriggerId = objUid;
      }
    }
  }
}

void CScriptMazeNode::DeleteBarrier(CStateManager& mgr) {
  mgr.DeleteObjectRequest(mEffectId);
  mgr.DeleteObjectRequest(mActorId);
  mgr.DeleteObjectRequest(mTriggerId);
  mgr.DeleteObjectRequest(mGateEffectId);
  mEffectId = mActorId = mTriggerId = mGateEffectId = kInvalidUniqueId;
}

void CScriptMazeNode::SendBarrierMsg(CStateManager& mgr, EScriptObjectMessage msg) {
  DeliverScriptMsg(mgr, mgr.ObjectById(mEffectId), GetUniqueId(), msg);
  DeliverScriptMsg(mgr, mgr.ObjectById(mActorId), GetUniqueId(), msg);
  DeliverScriptMsg(mgr, mgr.ObjectById(mTriggerId), GetUniqueId(), msg);
  DeliverScriptMsg(mgr, mgr.ObjectById(mGateEffectId), GetUniqueId(), msg);
}

void CScriptMazeNode::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (GetActive()) {
    switch (msg) {
    case kSM_InitializedInArea: {
      if (mgr.SinglePathMaze() == nullptr) {
        rstl::single_ptr< CSinglePathMaze > maze =
            rs_new CSinglePathMaze(skEnterCol, skEnterRow, skTargetCol, skTargetRow);
        maze->CreateMaze(sMazeSeeds[mgr.Random()->Next() % 300]);
        maze->SolveMaze();
        maze->AddGimmicks();
        mgr.SetSinglePathMaze(maze);
      }
      break;
    }
    case kSM_Action: {
      bool shouldGenObjs = false;
      if (const CSinglePathMaze* maze = mgr.SinglePathMaze()) {
        const SMazeCell& cell = maze->GetMazePoint(mCol, mRow);
        if (mSide == CSinglePathMaze::kS_Top && cell.mOpenTop) {
          if (cell.mGateTop) {
            shouldGenObjs = true;
            mHasGate = true;
          }
        } else if (mSide == CSinglePathMaze::kS_Right && cell.mOpenRight) {
          if (cell.mGateRight) {
            shouldGenObjs = true;
            mHasGate = true;
          }
        } else {
          shouldGenObjs = true;
        }
        if (shouldGenObjs) {
          GenerateBarrier(mgr);
        }
        if (mSide == CSinglePathMaze::kS_Right && cell.mPuddle) {
          mHasPuddle = true;
        }
        if (mHasGate) {
#if !NONMATCHING
          // Unused
          CTransform4f xf = GetTransform();
#endif
          rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
          for (; conn != GetConnectionList().end(); ++conn) {
            if (conn->mState != kSS_Modify || conn->mMsg != kSM_Activate) {
              continue;
            }

            bool wasGeneratingObject = mgr.IsGeneratingObject();
            mgr.SetIsGeneratingObject(true);
            TUniqueId genObj = mgr.GenerateObject(conn->mObjId).second;
            mgr.SetIsGeneratingObject(wasGeneratingObject);

            mGateEffectId = genObj;
            if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(genObj))) {
              actor->SetTranslation(GetTranslation() + mEffectPos);
              mgr.DeliverScriptMsg(actor, GetUniqueId(), kSM_Activate);
            }
            break;
          }
        }
        if (mHasPuddle) {
          int count = 0;
          rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
          for (; conn != GetConnectionList().end(); ++conn) {
            if ((conn->mState == kSS_Closed || conn->mState == kSS_DeactivateState) &&
                conn->mMsg == kSM_Activate) {
              count++;
            }
          }
          mPuddleObjectIds.reserve(count);
          conn = GetConnectionList().begin();
          for (; conn != GetConnectionList().end(); ++conn) {
            if ((conn->mState == kSS_Closed || conn->mState == kSS_DeactivateState) &&
                conn->mMsg == kSM_Activate) {
              bool wasGeneratingObject = mgr.IsGeneratingObject();
              mgr.SetIsGeneratingObject(true);
              TUniqueId genObj = mgr.GenerateObject(conn->mObjId).second;
              mgr.SetIsGeneratingObject(wasGeneratingObject);

              mPuddleObjectIds.push_back(genObj);
              if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(genObj))) {
                actor->SetTransform(GetTransform());
                if (conn->mState == kSS_Closed) {
                  mgr.DeliverScriptMsg(actor, GetUniqueId(), kSM_Activate);
                }
              }
            }
          }
        }
      }
      break;
    }
    case kSM_SetToZero: {
      CSinglePathMaze* maze = mgr.SinglePathMaze();
      if (mHasPuddle && maze != nullptr) {
        if (rstl::find(mPuddleObjectIds.begin(), mPuddleObjectIds.end(), uid) !=
            mPuddleObjectIds.end()) {
          rstl::vector< TUniqueId >::const_iterator it = mPuddleObjectIds.begin();
          for (; it != mPuddleObjectIds.end(); ++it) {
            if (CEntity* ent = mgr.ObjectById(*it)) {
              if (!ent->GetActive()) {
                mgr.DeliverScriptMsg(ent, GetUniqueId(), kSM_Activate);
              } else {
                mgr.DeleteObjectRequest(ent->GetUniqueId());
              }
            }
          }
          CObjectList& list = mgr.ObjectListById(kOL_All);
          for (int objIdx = list.GetFirstObjectIndex(); objIdx != -1;
               objIdx = list.GetNextObjectIndex(objIdx)) {
            if (CScriptMazeNode* node = TCastToPtr< CScriptMazeNode >(list[objIdx])) {
              if (node->mCol == mCol - 1 && node->mRow == mRow &&
                  node->mSide == CSinglePathMaze::kS_Right) {
                SMazeCell& cell = maze->MazePoint(mCol - 1, mRow);
                if (!cell.mOpenRight) {
                  cell.mOpenRight = true;
                  node->DeleteBarrier(mgr);
                  node->mHasGate = false;
                }
              }
              if (node->mCol == mCol && node->mRow == mRow &&
                  node->mSide == CSinglePathMaze::kS_Right) {
                SMazeCell& cell = maze->MazePoint(mCol, mRow);
                if (!cell.mOpenRight) {
                  cell.mOpenRight = true;
                  node->DeleteBarrier(mgr);
                  node->mHasGate = false;
                }
              }
              if (node->mCol == mCol && node->mRow == mRow &&
                  node->mSide == CSinglePathMaze::kS_Top) {
                SMazeCell& cell = maze->MazePoint(mCol, mRow);
                if (!cell.mOpenTop) {
                  cell.mOpenTop = true;
                  node->DeleteBarrier(mgr);
                  node->mHasGate = false;
                }
              }
              if (node->mCol == mCol && node->mRow == mRow + 1 &&
                  node->mSide == CSinglePathMaze::kS_Top) {
                SMazeCell& cell = maze->MazePoint(mCol, mRow + 1);
                if (!cell.mOpenTop) {
                  cell.mOpenTop = true;
                  node->DeleteBarrier(mgr);
                  node->mHasGate = false;
                }
              }
            }
          }
        }
      }
      break;
    }
    case kSM_Deleted: {
      if (mgr.GetSinglePathMaze()) {
        mgr.SetSinglePathMaze(nullptr);
      }
      DeleteBarrier(mgr);
      break;
    }
    case kSM_Deactivate: {
      DeleteBarrier(mgr);
      break;
    }
    }
  }
  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptMazeNode::Think(float dt, CStateManager& mgr) {
  if (GetActive() && mHasGate) {
    mMsgTimer -= dt;
    if (mMsgTimer <= 0.f) {
      mMsgTimer = 1.f;
      if (mGateActive) {
        mGateActive = false;
        SendBarrierMsg(mgr, kSM_Deactivate);
      } else {
        mGateActive = true;
        SendBarrierMsg(mgr, kSM_Activate);
      }
    }
  }
}

void CScriptMazeNode::LoadMazeSeeds() {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName("DUMB_MazeSeeds");
  rstl::auto_ptr< CInputStream > stream =
      gpResourceFactory->GetResLoader().LoadNewResourceSync(*tag, nullptr);
  for (int i = 0; i < 300; i++) {
    sMazeSeeds[i] = stream->ReadLong();
  }
}

CScriptMazeNode::~CScriptMazeNode() {}
