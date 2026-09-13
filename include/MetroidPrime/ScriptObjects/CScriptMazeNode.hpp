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
  bool x0_24_openTop : 1;
  bool x0_25_openRight : 1;
  bool x0_26_openBottom : 1;
  bool x0_27_openLeft : 1;
  bool x0_28_gateTop : 1;
  bool x0_29_gateRight : 1;
  bool x0_30_gateBottom : 1;
  bool x0_31_gateLeft : 1;
  bool x1_24_puddle : 1;
  bool x1_25_onPath : 1;
  bool x1_26_checked : 1;

  SMazeCell() {
    x0_24_openTop = x0_25_openRight = x0_26_openBottom = x0_27_openLeft = x0_28_gateTop =
        x0_29_gateRight = x0_30_gateBottom = x0_31_gateLeft = x1_24_puddle = x1_25_onPath =
            x1_26_checked = false;
  }

  inline bool IsClosed() const {
    return !x0_24_openTop && !x0_25_openRight && !x0_26_openBottom && !x0_27_openLeft;
  }
};

class CSinglePathMaze {
  CRandom16 x0_rand;
  SMazeCell x4_cells[skMazeRows * skMazeCols];
  int x84_enterCol;
  int x88_enterRow;
  int x8c_targetCol;
  int x90_targetRow;
  bool x94_24_initialized : 1;

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
  SMazeCell& MazePointInline(uint col, uint row) { return x4_cells[col + row * skMazeCols]; } // ????
  inline SMazeCell& MazePoint(uint idx) { return x4_cells[idx]; }
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

  int xe8_col;
  int xec_row;
  ESide xf0_side;
  TUniqueId xf4_gateEffectId;
  float xf8_msgTimer;
  TUniqueId xfc_actorId;
  CVector3f x100_actorPos;
  TUniqueId x10c_triggerId;
  CVector3f x110_triggerPos;
  TUniqueId x11c_effectId;
  CVector3f x120_effectPos;
  rstl::vector< TUniqueId > x12c_puddleObjectIds;
  bool x13c_24_hasPuddle : 1;
  bool x13c_25_hasGate : 1;
  bool x13c_26_gateActive : 1;

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
