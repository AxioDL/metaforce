#ifndef _CSYSTEMSTATE
#define _CSYSTEMSTATE

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CSystemState {
  friend class CGameState;
public:
  CSystemState();
  explicit CSystemState(CInputStream&);
  ~CSystemState() {}
  void PutTo(COutputStream&);

  void SetFusionLinked(bool v);
  void SetHasHardMode(bool v);
  void SetHardModeBeat(bool v);
  void SetLogScanCount(int count);
  int GetLogScanCount() const;
  void SetFusionBeat(bool v);
  void SetHasFusion(bool v);
  const bool GetFusionLinked() const { return xd0_24_fusionLinked; }
  bool GetNormalModeBeat() const { return xd0_25_normalModeBeat; }
  bool GetHardModeBeat() const { return xd0_26_hardModeBeat; }
  const bool GetFusionBeat() const { return xd0_27_fusionBeat; }
  bool GetHasFusion() const { return xd0_28_fusionSuitActive; }

  bool GetCinematicState(rstl::pair< CAssetId, TEditorId > cineId) const;
  void SetCinematicState(rstl::pair< CAssetId, TEditorId > cineId, bool state);

  int GetAutoMapperKeyState() const { return xbc_autoMapperKeyState; }
  void SetAutoMapperKeyState(int state);// { xbc_autoMapperKeyState = state; }

  bool GetShowPowerBombAmmoMessage() const;
  void IncrementPowerBombAmmoCount();
  void IncrementFrozenBallCount();
  bool GetShowFrozenBallMessage() const;
  bool GetShowFrozenFpsMessage() const;

  uchar* GetNESState() { return x0_nesState.data(); }

  bool GetAllItemsCollected() const { return xd0_29_allItemsCollected; }
  void SetAllItemsCollected(bool);

  // MP1R
  bool AreFreezeInstructionsStillEnabledFirstPerson() const;
  bool AreFreezeInstructionsStillEnabledMorphBall() const;
  void IncNumFreezeInstructionsPrintedFirstPerson();

private:
  static uint GetBitCount(uint value);

#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
  enum { kNESStateSize = 18 };
#else
  enum { kNESStateSize = 98 };
#endif
  rstl::reserved_vector< uchar, kNESStateSize > x0_nesState;
  rstl::reserved_vector< uchar, 64 > x68_;
  rstl::vector< rstl::pair< CAssetId, TEditorId > > xac_cinematicStates;
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
  int x6c_language;
#endif
  int xbc_autoMapperKeyState;
  int xc0_frozenFpsCount;
  int xc4_frozenBallCount;
  int xc8_powerBombAmmoCount;
  int xcc_logScanPercent;
  bool xd0_24_fusionLinked : 1;
  bool xd0_25_normalModeBeat : 1;
  bool xd0_26_hardModeBeat : 1;
  bool xd0_27_fusionBeat : 1;
  bool xd0_28_fusionSuitActive : 1;
  bool xd0_29_allItemsCollected : 1;
};
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
CHECK_SIZEOF(CSystemState, 0x88)
#else
CHECK_SIZEOF(CSystemState, 0xd4)
#endif

#endif // _CSYSTEMSTATE
