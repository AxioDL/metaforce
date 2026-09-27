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
  const bool GetFusionLinked() const { return mFusionLinked; }
  bool GetNormalModeBeat() const { return mNormalModeBeat; }
  bool GetHardModeBeat() const { return mHardModeBeat; }
  const bool GetFusionBeat() const { return mFusionBeat; }
  bool GetHasFusion() const { return mFusionSuitActive; }

  bool GetCinematicState(rstl::pair< CAssetId, TEditorId > cineId) const;
  void SetCinematicState(rstl::pair< CAssetId, TEditorId > cineId, bool state);

#if VERSION >= VERSION_GM8P_00
  void SetLanguage(int language);
#endif

  int GetAutoMapperKeyState() const { return mAutoMapperKeyState; }
  void SetAutoMapperKeyState(int state);// { xbc_autoMapperKeyState = state; }

  bool GetShowPowerBombAmmoMessage() const;
  void IncrementPowerBombAmmoCount();
  void IncrementFrozenBallCount();
  bool GetShowFrozenBallMessage() const;
  bool GetShowFrozenFpsMessage() const;

  uchar* GetNESState() { return mNesState.data(); }

  bool GetAllItemsCollected() const { return mAllItemsCollected; }
#if VERSION >= VERSION_GM8P_00
  int GetLanguage() const { return mLanguage; }
#endif
  void SetAllItemsCollected(bool);

  // MP1R
  bool AreFreezeInstructionsStillEnabledFirstPerson() const;
  bool AreFreezeInstructionsStillEnabledMorphBall() const;
  void IncNumFreezeInstructionsPrintedFirstPerson();

private:
  static uint GetBitCount(uint value);

#if VERSION == VERSION_GM8J_00
  enum { kNESStateSize = 1526 };
#elif VERSION >= VERSION_GM8P_00
  enum { kNESStateSize = 18 };
#else
  enum { kNESStateSize = 98 };
#endif
  rstl::reserved_vector< uchar, kNESStateSize > mNesState;
  rstl::reserved_vector< uchar, 64 > x68_;
  rstl::vector< rstl::pair< CAssetId, TEditorId > > mCinematicStates;
#if VERSION >= VERSION_GM8P_00
  int mLanguage;
#endif
  int mAutoMapperKeyState;
  int mFrozenFpsCount;
  int mFrozenBallCount;
  int mPowerBombAmmoCount;
  int mLogScanPercent;
  bool mFusionLinked : 1;
  bool mNormalModeBeat : 1;
  bool mHardModeBeat : 1;
  bool mFusionBeat : 1;
  bool mFusionSuitActive : 1;
  bool mAllItemsCollected : 1;
};
#if VERSION == VERSION_GM8J_00
CHECK_SIZEOF(CSystemState, 0x66c)
#elif VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CSystemState, 0x88)
#else
CHECK_SIZEOF(CSystemState, 0xd4)
#endif

#endif // _CSYSTEMSTATE
