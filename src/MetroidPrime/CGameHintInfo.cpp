#include "MetroidPrime/CGameHintInfo.hpp"

#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "rstl/math.hpp"

const float CGameHintInfo::skHintTextTime = 3.f;

uint CHintOptions::GetBitCount(uint value) {
  uint count = 0;
  for (; value != 0; value >>= 1) {
    ++count;
  }
  return count;
}

CGameHintInfo::SHintLocation::SHintLocation(CInputStream& in)
: mMlvlId(in.ReadLong())
, mMreaId(in.ReadLong())
, mAreaId(in.ReadLong())
, mStringId(in.ReadLong()) {}

inline void CGameHintInfo::CGameHint::ReadLocations(CInputStream& in) {
  const int count = in.Get< int >();
  mLocations.reserve(count);
  for (int i = 0; i < count; ++i) {
    mLocations.push_back(SHintLocation(in));
  }
}

CGameHintInfo::CGameHint::CGameHint(CInputStream& in, int version)
: mName(in)
, mImmediateTime(in.ReadFloat())
, mNormalTime(in.ReadFloat())
, mStringId(in.ReadLong())
, mTextTime(CGameHintInfo::skHintTextTime * static_cast< float >(version > 0 ? in.Get< int >() : 1))
, mLocations() {
  ReadLocations(in);
}

CGameHintInfo::CGameHintInfo(CInputStream& in, int version) {
  mHints.reserve(in.ReadLong());
  for (int i = 0; i < mHints.capacity(); ++i) {
    mHints.push_back(CGameHint(in, version));
  }
}

CHintOptions::SHintState::SHintState() : mState(kHS_Zero), mTime(0.f), mDismissed(false) {}

CHintOptions::SHintState::SHintState(EHintState state, float time)
: mState(state), mTime(time), mDismissed(false) {}

bool CHintOptions::SHintState::CanContinue() { return mTime / CGameHintInfo::skHintTextTime < 1.f; }

CHintOptions::CHintOptions() : mNextHintIdx(-1)
#if VERSION >= VERSION_GM8P_00
, mPalHintFlag(false)
#endif
{}

CHintOptions::CHintOptions(CInputStream& in) : mNextHintIdx(-1)
#if VERSION >= VERSION_GM8P_00
, mPalHintFlag(false)
#endif
{
  mHintStates.reserve(gpMemoryCard->GetHints().size());
  for (int i = 0; i < mHintStates.capacity(); ++i) {
    const EHintState state = static_cast< EHintState >(in.ReadBits(GetBitCount(kHS_Delayed)));
    const uint timeBits = in.ReadBits(32);
    const float hintTime = reinterpret_cast< const float& >(timeBits);
    mHintStates.push_back(SHintState(
        state, state == kHS_Waiting || state == kHS_Displaying ? hintTime : 0.f));
    if (mNextHintIdx == -1 && state == kHS_Displaying) {
      mNextHintIdx = i;
    }
  }
}

void CHintOptions::SetHintNextTime() {
  if (mNextHintIdx == -1) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
  AUTO(it, mHintStates.begin());
  it += mNextHintIdx;
  it->mTime = hint.GetTextTime() + 5.f;
}

#if VERSION >= VERSION_GM8P_00
void CHintOptions::EnsureHintNextTime() {
  if (mNextHintIdx != -1) {
    SHintState& state = mHintStates[mNextHintIdx];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
    state.mTime = rstl::max_val(state.mTime, hint.GetTextTime() + 5.f);
  }
}

#endif

void CHintOptions::PutTo(COutputStream& out) const {
  for (AUTO(it, mHintStates.begin()); it != mHintStates.end(); ++it) {
    out.WriteBits(it->mState, GetBitCount(kHS_Delayed));
    out.WriteBits(reinterpret_cast< const uint& >(it->mTime), 32);
  }
}

void CHintOptions::InitializeMemoryState() {
  const int count = gpMemoryCard->GetHints().size();
  mHintStates.assign(count);
}

void CHintOptions::Update(float dt, const CStateManager& mgr) {
  mNextHintIdx = -1;
  for (int i = 0; i < mHintStates.size(); ++i) {
    SHintState& state = mHintStates[i];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[i];
    switch (state.mState) {
    case kHS_Zero:
    case kHS_Delayed:
      break;
    case kHS_Waiting:
      state.mTime -= dt;
      if (state.mTime <= 0.f) {
        state.mState = kHS_Displaying;
        state.mTime = hint.GetTextTime();
      }
      break;
    case kHS_Displaying:
      if (mNextHintIdx == -1) {
        mNextHintIdx = i;
      }
      break;
    }
  }

  if (mNextHintIdx == -1) {
    return;
  }
  SHintState& state = mHintStates[mNextHintIdx];
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
  state.mTime = rstl::max_val(0.f, state.mTime - dt);
  if (state.mTime < hint.GetTextTime()) {
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = hint.GetLocations();
    for (int i = 0; i < locations.size(); ++i) {
      const CGameHintInfo::SHintLocation& loc = locations[i];
      if (loc.mMlvlId == mgr.GetWorld()->GetWorldAssetId() &&
          loc.mAreaId == mgr.GetNextAreaId()) {
        state.mTime = hint.GetNormalTime();
        state.mDismissed = true;
        return;
      }
    }
  }
}

void CHintOptions::ActivateImmediateHintTimer(const rstl::string& name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[idx];
  SHintState& state = mHintStates[idx];
  if (state.mState == kHS_Zero) {
    state.mState = kHS_Waiting;
    state.mTime = hint.GetImmediateTime();
  }
}

void CHintOptions::DelayHint(const rstl::string& name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }
  SHintState& state = mHintStates[idx];
  if (idx == mNextHintIdx) {
    for (AUTO(it, mHintStates.begin()); it != mHintStates.end(); ++it) {
      it->mTime += 60.f;
    }
  }
  state.mState = kHS_Delayed;
}

void CHintOptions::ActivateContinueDelayHintTimer(const rstl::string& name) {
  int idx = mNextHintIdx;
  if (static_cast< int >(name.size()) != 0) {
    idx = CGameHintInfo::FindHintIndex(name);
  }
  if (idx == -1) {
    return;
  }
  SHintState& state = mHintStates[idx];
  if (state.mState != kHS_Displaying) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[idx];
  state.mTime = hint.GetTextTime();
}

const CHintOptions::SHintState* CHintOptions::GetCurrentDisplayedHint() const {
  if (gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    if (mNextHintIdx == -1) {
      return nullptr;
    }
    const SHintState& state = mHintStates[mNextHintIdx];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
    if (state.mTime >= hint.GetTextTime()) {
      return nullptr;
    }
    if (state.mTime >= CGameHintInfo::skHintTextTime) {
      return state.mDismissed ? nullptr : &state;
    } else {
      return &state;
    }
  }
  return nullptr;
}

int CHintOptions::GetNextHintIdx() {
  if (gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    return mNextHintIdx;
  }
  return -1;
}

int CGameHintInfo::FindHintIndex(const rstl::string& name) {
  const rstl::vector< CGameHint >& hints = gpMemoryCard->GetHints();
  for (int i = 0; i < hints.size(); ++i) {
    if (hints[i].GetName() == name) {
      return i;
    }
  }
  return -1;
}

void CHintOptions::DismissDisplayedHint() {
  if (mNextHintIdx == -1) {
    return;
  }
  SHintState& state = mHintStates[mNextHintIdx];
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
  if (state.mTime < hint.GetTextTime()) {
    state.mTime = hint.GetNormalTime();
    state.mDismissed = true;
  }
}

const CFactoryFnReturn FHintFactory(const SObjectTag& tag, CInputStream& in,
                              const CVParamTransfer& params) {
  in.ReadLong();
  const int version = in.Get< int >();
  return rs_new CGameHintInfo(in, version);
}
