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
: x0_mlvlId(in.ReadLong())
, x4_mreaId(in.ReadLong())
, x8_areaId(in.ReadLong())
, xc_stringId(in.ReadLong()) {}

inline void CGameHintInfo::CGameHint::ReadLocations(CInputStream& in) {
  const int count = in.Get< int >();
  x20_locations.reserve(count);
  for (int i = 0; i < count; ++i) {
    x20_locations.push_back(SHintLocation(in));
  }
}

CGameHintInfo::CGameHint::CGameHint(CInputStream& in, int version)
: x0_name(in)
, x10_immediateTime(in.ReadFloat())
, x14_normalTime(in.ReadFloat())
, x18_stringId(in.ReadLong())
, x1c_textTime(CGameHintInfo::skHintTextTime * static_cast< float >(version > 0 ? in.Get< int >() : 1))
, x20_locations() {
  ReadLocations(in);
}

CGameHintInfo::CGameHintInfo(CInputStream& in, int version) {
  x0_hints.reserve(in.ReadLong());
  for (int i = 0; i < x0_hints.capacity(); ++i) {
    x0_hints.push_back(CGameHint(in, version));
  }
}

CHintOptions::SHintState::SHintState() : x0_state(kHS_Zero), x4_time(0.f), x8_dismissed(false) {}

CHintOptions::SHintState::SHintState(EHintState state, float time)
: x0_state(state), x4_time(time), x8_dismissed(false) {}

bool CHintOptions::SHintState::CanContinue() { return x4_time / CGameHintInfo::skHintTextTime < 1.f; }

CHintOptions::CHintOptions() : x10_nextHintIdx(-1)
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
, x14_palHintFlag(false)
#endif
{}

CHintOptions::CHintOptions(CInputStream& in) : x10_nextHintIdx(-1)
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
, x14_palHintFlag(false)
#endif
{
  x0_hintStates.reserve(gpMemoryCard->GetHints().size());
  for (int i = 0; i < x0_hintStates.capacity(); ++i) {
    const EHintState state = static_cast< EHintState >(in.ReadBits(GetBitCount(kHS_Delayed)));
    const uint timeBits = in.ReadBits(32);
    const float hintTime = reinterpret_cast< const float& >(timeBits);
    x0_hintStates.push_back(SHintState(
        state, state == kHS_Waiting || state == kHS_Displaying ? hintTime : 0.f));
    if (x10_nextHintIdx == -1 && state == kHS_Displaying) {
      x10_nextHintIdx = i;
    }
  }
}

void CHintOptions::SetHintNextTime() {
  if (x10_nextHintIdx == -1) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[x10_nextHintIdx];
  AUTO(it, x0_hintStates.begin());
  it += x10_nextHintIdx;
  it->x4_time = hint.GetTextTime() + 5.f;
}

void CHintOptions::PutTo(COutputStream& out) const {
  for (AUTO(it, x0_hintStates.begin()); it != x0_hintStates.end(); ++it) {
    out.WriteBits(it->x0_state, GetBitCount(kHS_Delayed));
    out.WriteBits(reinterpret_cast< const uint& >(it->x4_time), 32);
  }
}

void CHintOptions::InitializeMemoryState() {
  const int count = gpMemoryCard->GetHints().size();
  x0_hintStates.assign(count);
}

void CHintOptions::Update(float dt, const CStateManager& mgr) {
  x10_nextHintIdx = -1;
  for (int i = 0; i < x0_hintStates.size(); ++i) {
    SHintState& state = x0_hintStates[i];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[i];
    switch (state.x0_state) {
    case kHS_Zero:
    case kHS_Delayed:
      break;
    case kHS_Waiting:
      state.x4_time -= dt;
      if (state.x4_time <= 0.f) {
        state.x0_state = kHS_Displaying;
        state.x4_time = hint.GetTextTime();
      }
      break;
    case kHS_Displaying:
      if (x10_nextHintIdx == -1) {
        x10_nextHintIdx = i;
      }
      break;
    }
  }

  if (x10_nextHintIdx == -1) {
    return;
  }
  SHintState& state = x0_hintStates[x10_nextHintIdx];
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[x10_nextHintIdx];
  state.x4_time = rstl::max_val(0.f, state.x4_time - dt);
  if (state.x4_time < hint.GetTextTime()) {
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = hint.GetLocations();
    for (int i = 0; i < locations.size(); ++i) {
      const CGameHintInfo::SHintLocation& loc = locations[i];
      if (loc.x0_mlvlId == mgr.GetWorld()->GetWorldAssetId() &&
          loc.x8_areaId == mgr.GetNextAreaId()) {
        state.x4_time = hint.GetNormalTime();
        state.x8_dismissed = true;
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
  SHintState& state = x0_hintStates[idx];
  if (state.x0_state == kHS_Zero) {
    state.x0_state = kHS_Waiting;
    state.x4_time = hint.GetImmediateTime();
  }
}

void CHintOptions::DelayHint(const rstl::string& name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }
  SHintState& state = x0_hintStates[idx];
  if (idx == x10_nextHintIdx) {
    for (AUTO(it, x0_hintStates.begin()); it != x0_hintStates.end(); ++it) {
      it->x4_time += 60.f;
    }
  }
  state.x0_state = kHS_Delayed;
}

void CHintOptions::ActivateContinueDelayHintTimer(const rstl::string& name) {
  int idx = x10_nextHintIdx;
  if (static_cast< int >(name.size()) != 0) {
    idx = CGameHintInfo::FindHintIndex(name);
  }
  if (idx == -1) {
    return;
  }
  SHintState& state = x0_hintStates[idx];
  if (state.x0_state != kHS_Displaying) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[idx];
  state.x4_time = hint.GetTextTime();
}

const CHintOptions::SHintState* CHintOptions::GetCurrentDisplayedHint() const {
  if (gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    if (x10_nextHintIdx == -1) {
      return nullptr;
    }
    const SHintState& state = x0_hintStates[x10_nextHintIdx];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[x10_nextHintIdx];
    if (state.x4_time >= hint.GetTextTime()) {
      return nullptr;
    }
    if (state.x4_time >= CGameHintInfo::skHintTextTime) {
      return state.x8_dismissed ? nullptr : &state;
    } else {
      return &state;
    }
  }
  return nullptr;
}

int CHintOptions::GetNextHintIdx() {
  if (gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    return x10_nextHintIdx;
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
  if (x10_nextHintIdx == -1) {
    return;
  }
  SHintState& state = x0_hintStates[x10_nextHintIdx];
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[x10_nextHintIdx];
  if (state.x4_time < hint.GetTextTime()) {
    state.x4_time = hint.GetNormalTime();
    state.x8_dismissed = true;
  }
}

const CFactoryFnReturn FHintFactory(const SObjectTag& tag, CInputStream& in,
                              const CVParamTransfer& params) {
  in.ReadLong();
  const int version = in.Get< int >();
  return rs_new CGameHintInfo(in, version);
}
