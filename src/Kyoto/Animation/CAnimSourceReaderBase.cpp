#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "rstl/math.hpp"

template < class T >
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
                 const rstl::vector< T >& stream, const CCharAnimTime& curTime,
                 const IAnimSourceInfo& sourceInfo, int passedCount) {
  uint ret = 0;
  if (sourceInfo.HasPOIData()) {
    int count = stream.size();
    if (count > 0) {
      const CCharAnimTime& duration = sourceInfo.GetAnimationDuration();
      CCharAnimTime totalTime = curTime + time;
      CCharAnimTime endTime = rstl::min_val(duration, totalTime);
      if (passedCount < count) {
        int index = passedCount;
        const int initialIndex = index;
        CCharAnimTime nodeTime(stream[initialIndex].GetTime());
        while (index < count && nodeTime <= endTime) {
          const T& node = stream[index];
          if (ret + iterator < capacity) {
            listOut[iterator + ret] = T::CopyNodeMinusStartTime(node, curTime);
            ++ret;
          }
          ++index;
          if (index < count) {
            nodeTime = stream[index].GetTime();
          }
        }
      }
    }
  }
  return ret;
}

uint CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CBoolPOINode >& stream = sourceInfo.GetBoolPOIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedBoolCount);
  }
  return 0;
}

uint CAnimSourceReaderBase::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CInt32POINode >& stream = sourceInfo.GetInt32POIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedIntCount);
  }
  return 0;
}

uint CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime& time,
                                                CParticlePOINode* listOut, uint capacity,
                                                uint iterator, int additive) const {
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CParticlePOINode >& stream = sourceInfo.GetParticlePOIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedParticleCount);
  }
  return 0;
}

uint CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CSoundPOINode >& stream = sourceInfo.GetSoundPOIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedSoundCount);
  }
  return 0;
}

bool CAnimSourceReaderBase::VGetBoolPOIState(const char* name) const {
  int count = mBoolStates.size();
  for (int i = 0; i < count; ++i) {
    if (mBoolStates[i].first == name) {
      return mBoolStates[i].second;
    }
  }
  return false;
}

s32 CAnimSourceReaderBase::VGetInt32POIState(const char* name) const {
  int count = mInt32States.size();
  for (int i = 0; i < count; ++i) {
    if (mInt32States[i].first == name) {
      return mInt32States[i].second;
    }
  }
  return 0;
}

bool rstl::operator==(const rstl::string& lhs, const char* rhs) { return lhs.compare(rhs) == 0; }

CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(const char* name) const {
  int count = mParticleStates.size();
  for (int i = 0; i < count; ++i) {
    if (mParticleStates[i].first == name) {
      return mParticleStates[i].second;
    }
  }
  return CParticleData::kPM_Initial;
}

void CAnimSourceReaderBase::UpdatePOIStates() {
  const IAnimSourceInfo& sourceInfo = AnimSource();
  const rstl::vector< CBoolPOINode >& boolNodes = sourceInfo.GetBoolPOIStream();
  const rstl::vector< CInt32POINode >& int32Nodes = sourceInfo.GetInt32POIStream();
  const rstl::vector< CParticlePOINode >& particleNodes = sourceInfo.GetParticlePOIStream();
  const rstl::vector< CSoundPOINode >& soundNodes = sourceInfo.GetSoundPOIStream();
  int boolCount = boolNodes.size();
  int int32Count = int32Nodes.size();
  int particleCount = particleNodes.size();
  int soundCount = soundNodes.size();
  while (mPassedBoolCount < boolCount &&
         boolNodes[mPassedBoolCount].GetTime() <= mCurTime) {
    const CBoolPOINode& node = boolNodes[mPassedBoolCount];
    int index = node.GetIndex();
    if (index >= 0) {
      mBoolStates[index] =
          rstl::pair< rstl::string, bool >(mBoolStates[index].first, node.GetValue());
    }
    ++mPassedBoolCount;
  }
  while (mPassedIntCount < int32Count &&
         int32Nodes[mPassedIntCount].GetTime() <= mCurTime) {
    const CInt32POINode& node = int32Nodes[mPassedIntCount];
    int index = node.GetIndex();
    if (index >= 0) {
      mInt32States[index] =
          rstl::pair< rstl::string, int >(mInt32States[index].first, node.GetValue());
    }
    ++mPassedIntCount;
  }
  while (mPassedParticleCount < particleCount &&
         particleNodes[mPassedParticleCount].GetTime() <= mCurTime) {
    const CParticlePOINode& node = particleNodes[mPassedParticleCount];
    int index = node.GetIndex();
    if (index >= 0) {
      mParticleStates[index] = rstl::pair< rstl::string, CParticleData::EParentedMode >(
          mParticleStates[index].first, node.GetParticleData().GetParentedMode());
    }
    ++mPassedParticleCount;
  }
  while (mPassedSoundCount < soundCount &&
         soundNodes[mPassedSoundCount].GetTime() <= mCurTime) {
    ++mPassedSoundCount;
  }
}

rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueBoolPOIs() const {
  const rstl::vector< CBoolPOINode >& nodes = mSourceInfo->GetBoolPOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
  for (int i = 0; i < count; ++i) {
    const CBoolPOINode& node = nodes[i];
    if (node.GetSaveState()) {
      ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
    }
  }
  return ret;
}

rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueInt32POIs() const {
  const rstl::vector< CInt32POINode >& nodes = mSourceInfo->GetInt32POIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
  for (int i = 0; i < count; ++i) {
    const CInt32POINode& node = nodes[i];
    if (node.GetSaveState()) {
      ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
    }
  }
  return ret;
}

rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const rstl::vector< CParticlePOINode >& nodes = mSourceInfo->GetParticlePOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
  for (int i = 0; i < count; ++i) {
    const CParticlePOINode& node = nodes[i];
    if (node.GetSaveState()) {
      ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
    }
  }
  return ret;
}

void CAnimSourceReaderBase::PostConstruct(const CCharAnimTime& time) {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
  const IAnimSourceInfo& sourceInfo = AnimSource();
  if (sourceInfo.HasPOIData()) {
    const rstl::set< rstl::pair< rstl::string, int > > boolPOIs = GetUniqueBoolPOIs();
    const rstl::set< rstl::pair< rstl::string, int > > int32POIs = GetUniqueInt32POIs();
    const rstl::set< rstl::pair< rstl::string, int > > particlePOIs = GetUniqueParticlePOIs();
    int boolCount = boolPOIs.size();
    int int32Count = int32POIs.size();
    int particleCount = particlePOIs.size();
    mBoolStates.resize(boolCount, rstl::pair< rstl::string, bool >(rstl::string_l(""), false));
    mInt32States.resize(int32Count, rstl::pair< rstl::string, int >(rstl::string_l(""), 0));
    mParticleStates.resize(particleCount,
                              rstl::pair< rstl::string, CParticleData::EParentedMode >(
                                  rstl::string_l(""), CParticleData::kPM_Initial));
    for (AUTO(it, boolPOIs.begin()); it != boolPOIs.end();) {
      rstl::string name = it->first;
      int index = it->second;
      mBoolStates[index] = rstl::pair< rstl::string, bool >(name, false);
      ++it;
    }
    for (AUTO(it, int32POIs.begin()); it != int32POIs.end();) {
      rstl::string name = it->first;
      int index = it->second;
      mInt32States[index] = rstl::pair< rstl::string, int >(name, 0);
      ++it;
    }
    for (AUTO(it, particlePOIs.begin()); it != particlePOIs.end();) {
      rstl::string name = it->first;
      int index = it->second;
      mParticleStates[index] = rstl::pair< rstl::string, CParticleData::EParentedMode >(
          name, CParticleData::kPM_Initial);
      ++it;
    }
  }
  CCharAnimTime remaining = time;
  if (remaining.GreaterThanZero()) {
    while (remaining.GreaterThanZero()) {
      remaining = VAdvanceView(remaining).GetRemainder();
    }
  } else if (sourceInfo.HasPOIData()) {
    UpdatePOIStates();
    if (!time.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
}
