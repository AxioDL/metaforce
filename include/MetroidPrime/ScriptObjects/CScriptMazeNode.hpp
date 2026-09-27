#ifndef _CSCRIPTMAZENODE
#define _CSCRIPTMAZENODE

#include "types.h"

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"

#define skMazeCols 9
#define skMazeRows 7
#define skEnterCol 4
#define skEnterRow 4
#define skTargetCol 5
#define skTargetRow 3

#define NUM_MAZE_CELLS (skMazeRows * skMazeCols)

struct SMazeCell {
  bool mOpenTop : 1;
  bool mOpenRight : 1;
  bool mOpenBottom : 1;
  bool mOpenLeft : 1;
  bool mGateTop : 1;
  bool mGateRight : 1;
  bool mGateBottom : 1;
  bool mGateLeft : 1;
  bool mPuddle : 1;
  bool mOnPath : 1;
  bool mChecked : 1;

  SMazeCell() {
    mOpenTop = mOpenRight = mOpenBottom = mOpenLeft = mGateTop =
        mGateRight = mGateBottom = mGateLeft = mPuddle = mOnPath =
            mChecked = false;
  }

  inline bool IsClosed() const {
    return !mOpenTop && !mOpenRight && !mOpenBottom && !mOpenLeft;
  }
};

class CSinglePathMaze {
  CRandom16 mRand;
  SMazeCell mCells[skMazeRows * skMazeCols];
  int mEnterCol;
  int mEnterRow;
  int mTargetCol;
  int mTargetRow;
  bool mInitialized : 1;

public:
  enum ESide {
    kS_Invalid = -1,
    kS_Top = 0,
    kS_Right = 1,
    kS_Bottom = 2,
    kS_Left = 3,
  };

  CSinglePathMaze(int enterCol, int enterRow, int targetCol, int targetRow);
  void CreateMaze(int seed);
  void SolveMaze();
  void AddGimmicks();

  SMazeCell& MazePoint(uint col, uint row);
  const SMazeCell& GetMazePoint(uint col, uint row) const;
  SMazeCell& MazePointInline(uint col, uint row) { return mCells[col + row * skMazeCols]; } // ????
  inline SMazeCell& MazePoint(uint idx) { return mCells[idx]; }
};

class CScriptMazeNode : public CActor {
public:
  CScriptMazeNode(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, bool active, int col, int row, int side,
                  const CVector3f& actorPos, const CVector3f& triggerPos,
                  const CVector3f& effectPos);
  ~CScriptMazeNode() override;

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  static void LoadMazeSeeds();

private:
  enum ESide {
    Invalid = -1,
    Top = 0,
    Right = 1,
    Bottom = 2,
    Left = 3,
  };

  int mCol;
  int mRow;
  ESide mSide;
  TUniqueId mGateEffectId;
  float mMsgTimer;
  TUniqueId mActorId;
  CVector3f mActorPos;
  TUniqueId mTriggerId;
  CVector3f mTriggerPos;
  TUniqueId mEffectId;
  CVector3f mEffectPos;
  rstl::vector< TUniqueId > mPuddleObjectIds;
  bool mHasPuddle : 1;
  bool mHasGate : 1;
  bool mGateActive : 1;

  void GenerateBarrier(CStateManager& mgr);
  void DeleteBarrier(CStateManager& mgr);
  void SendBarrierMsg(CStateManager& mgr, EScriptObjectMessage msg);

  static uint sMazeSeeds[300];

  // hack for SendBarrierMsg
  static inline void DeliverScriptMsg(CStateManager& mgr, CEntity* to, TUniqueId sender,
                                   EScriptObjectMessage msg) {
    mgr.DeliverScriptMsg(to, sender, msg);
  }
};

#endif // _CSCRIPTMAZENODE
