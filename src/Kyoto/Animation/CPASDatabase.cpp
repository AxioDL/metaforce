#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/algorithm.hpp"

CPASDatabase::CPASDatabase(CInputStream& in) : mDefaultState(-1) {
  in.Get< int >();

  const uint stateCount = in.Get< uint >();
  mStates.reserve(stateCount);
  const int defaultState = in.Get< int >();
  for (int i = 0; i < stateCount; i++) {
    AddAnimState(CPASAnimState(in));
  }

  if (stateCount != 0) {
    SetDefaultState(defaultState);
  }
}

rstl::pair< float, int > CPASDatabase::FindBestAnimation(const CPASAnimParmData& data,
                                                         int ignoreAnim) const {
  AUTO(it, rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId())));
  if (it != mStates.end()) {
    CRandom16 random(0x1234);
    return it->FindBestAnimation(data.GetAnimParmData(), random, ignoreAnim);
  }
  return rstl::pair< float, int >(0.f, -1);
}

rstl::pair< float, int > CPASDatabase::FindBestAnimation(const CPASAnimParmData& data,
                                                         CRandom16& random, int ignoreAnim) const {
  AUTO(it, rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId())));
  if (it != mStates.end()) {
    return it->FindBestAnimation(data.GetAnimParmData(), random, ignoreAnim);
  }
  return rstl::pair< float, int >(0.f, -1);
}

size_t CPASDatabase::GetNumAnimStates() const { return mStates.size(); }

bool CPASDatabase::HasState(int id) const {
  AUTO(it, rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(id)));
  return it != mStates.end();
}

const CPASAnimState* CPASDatabase::GetAnimState(int id) const {
  return rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(id)).get_pointer();
}

const CPASAnimState* CPASDatabase::GetAnimStateByIndex(int index) const {
  return &mStates[index];
}

void CPASDatabase::SetDefaultState(int state) { mDefaultState = state; }

void CPASDatabase::AddAnimState(const CPASAnimState& state) {
  const rstl::vector< CPASAnimState >::iterator it =
      rstl::lower_bound(mStates.begin(), mStates.end(), state);
  mStates.insert(it, state);
}
