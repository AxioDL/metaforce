#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/string.hpp"

#include <string.h>

static inline CAssetId get_asset_id_from_name(const char* name) {
  CAssetId id = gpResourceFactory->GetResourceIdByName(name)->GetId();
  return id;
}

static inline rstl::reserved_vector< CAssetId, 9 > read_map_icons(CInputStream& in) {
  rstl::reserved_vector< CAssetId, 9 > ids;
  for (int i = 0; i < 9; ++i) {
    ids.push_back(get_asset_id_from_name(rstl::string(in).c_str()));
  }
  return ids;
}

template < int N >
inline rstl::reserved_vector< CAssetId, N > read_asset_ids(CInputStream& in) {
  rstl::reserved_vector< CAssetId, N > ids;
  for (int i = 0; i < N; ++i) {
    ids.push_back(get_asset_id_from_name(rstl::string(in).c_str()));
  }
  return ids;
}

CTweakPlayerRes::CTweakPlayerRes(CInputStream& in)
#if VERSION >= VERSION_GM8P_00
: mMapIcons(read_map_icons(in))
#else
: mSaveStationIcon(get_asset_id_from_name(rstl::string(in).c_str()))
, mMissileStationIcon(get_asset_id_from_name(rstl::string(in).c_str()))
, mElevatorIcon(get_asset_id_from_name(rstl::string(in).c_str()))
, mMinesBreakFirstTopIcon(get_asset_id_from_name(rstl::string(in).c_str()))
, mMinesBreakFirstBottomIcon(get_asset_id_from_name(rstl::string(in).c_str()))
, mMinesBreakSecondTopIcon(get_asset_id_from_name(rstl::string(in).c_str()))
, mMinesBreakSecondBottomIcon(get_asset_id_from_name(rstl::string(in).c_str()))
#endif
, mLStick(read_asset_ids< 9 >(in))
, mCStick(read_asset_ids< 9 >(in))
, mLTrigger(read_asset_ids< 2 >(in))
, mRTrigger(read_asset_ids< 2 >(in))
, mStartButton(read_asset_ids< 2 >(in))
, mAButton(read_asset_ids< 2 >(in))
, mBButton(read_asset_ids< 2 >(in))
, mXButton(read_asset_ids< 2 >(in))
, mYButton(read_asset_ids< 2 >(in))
, mBallTransitionsANCS(0)
, mCinematicMoveOutofIntoPlayerDistance(5.f) {
  memset(mBallTransitions, 0, sizeof(mBallTransitions));
  memset(mCineGun, 0, sizeof(mCineGun));

  const rstl::string ballTransitions(in);
  mBallTransitionsANCS = get_asset_id_from_name(ballTransitions.c_str());
  for (int i = 0; i < 5; ++i) {
    const rstl::string name(in);
    mBallTransitions[i] = get_asset_id_from_name(name.c_str());
  }
  for (int i = 0; i < 5; ++i) {
    const rstl::string name(in);
    mCineGun[i] = get_asset_id_from_name(name.c_str());
  }
  mCinematicMoveOutofIntoPlayerDistance = in.ReadFloat();
}

CAssetId CTweakPlayerRes::GetBallTransitionBeamResId(CPlayerState::EBeamId id) const {
  if (id < CPlayerState::kBI_Power || id > CPlayerState::kBI_Phazon) {
    return mBallTransitions[0];
  }
  return mBallTransitions[id];
}

CAssetId CTweakPlayerRes::GetCinematicBeamResId(CPlayerState::EBeamId id) const {
  if (id < CPlayerState::kBI_Power || id > CPlayerState::kBI_Phazon) {
    return mCineGun[0];
  }
  return mCineGun[id];
}
