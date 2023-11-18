#include "Runtime/Character/CAnimSourceReader.hpp"

#include <algorithm>

#include "Runtime/Character/CBoolPOINode.hpp"
#include "Runtime/Character/CFBStreamedAnimReader.hpp"
#include "Runtime/Character/CInt32POINode.hpp"
#include "Runtime/Character/CParticlePOINode.hpp"
#include "Runtime/Character/CSoundPOINode.hpp"

namespace metaforce {

CAnimSourceInfo::CAnimSourceInfo(TSubAnimTypeToken<CAnimSource> token) : x4_token(std::move(token)) {}

bool CAnimSourceInfo::HasPOIData() const { return x4_token->x58_evntData.HasReference(); }

const std::vector<CBoolPOINode>& CAnimSourceInfo::GetBoolPOIStream() const { return x4_token->GetBoolPOIStream(); }

const std::vector<CInt32POINode>& CAnimSourceInfo::GetInt32POIStream() const { return x4_token->GetInt32POIStream(); }

const std::vector<CParticlePOINode>& CAnimSourceInfo::GetParticlePOIStream() const {
  return x4_token->GetParticlePOIStream();
}

const std::vector<CSoundPOINode>& CAnimSourceInfo::GetSoundPOIStream() const { return x4_token->GetSoundPOIStream(); }

CCharAnimTime CAnimSourceInfo::GetAnimationDuration() const { return x4_token->GetDuration(); }

std::set<std::pair<std::string, s32>> CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const std::vector<CParticlePOINode>& particleNodes = x4_sourceInfo->GetParticlePOIStream();
  std::set<std::pair<std::string, s32>> ret;
  for (const CParticlePOINode& node : particleNodes) {
    if (node.GetUnique()) {
      ret.emplace(node.GetString(), node.GetIndex());
    }
  }
  return ret;
}

std::set<std::pair<std::string, s32>> CAnimSourceReaderBase::GetUniqueInt32POIs() const {
  const std::vector<CInt32POINode>& int32Nodes = x4_sourceInfo->GetInt32POIStream();
  std::set<std::pair<std::string, s32>> ret;
  for (const CInt32POINode& node : int32Nodes) {
    if (node.GetUnique()) {
      ret.emplace(node.GetString(), node.GetIndex());
    }
  }
  return ret;
}

std::set<std::pair<std::string, s32>> CAnimSourceReaderBase::GetUniqueBoolPOIs() const {
  const std::vector<CBoolPOINode>& boolNodes = x4_sourceInfo->GetBoolPOIStream();
  std::set<std::pair<std::string, s32>> ret;
  for (const CBoolPOINode& node : boolNodes) {
    if (node.GetUnique()) {
      ret.emplace(node.GetString(), node.GetIndex());
    }
  }
  return ret;
}

void CAnimSourceReaderBase::PostConstruct(const CCharAnimTime& time) {
  x14_passedBoolCount = 0;
  x18_passedIntCount = 0;
  x1c_passedParticleCount = 0;
  x20_passedSoundCount = 0;

  if (x4_sourceInfo->HasPOIData()) {
    std::set<std::pair<std::string, s32>> boolPOIs = GetUniqueBoolPOIs();
    std::set<std::pair<std::string, s32>> int32POIs = GetUniqueInt32POIs();
    std::set<std::pair<std::string, s32>> particlePOIs = GetUniqueParticlePOIs();

    x24_boolStates.resize(boolPOIs.size());
    x34_int32States.resize(int32POIs.size());
    x44_particleStates.resize(particlePOIs.size());

    for (const auto& poi : boolPOIs)
      x24_boolStates[poi.second] = std::make_pair(poi.first, false);
    for (const auto& poi : int32POIs)
      x34_int32States[poi.second] = std::make_pair(poi.first, 0);
    for (const auto& poi : particlePOIs)
      x44_particleStates[poi.second] = std::make_pair(poi.first, CParticleData::EParentedMode::Initial);
  }

  CCharAnimTime tmpTime = time;
  if (tmpTime.GreaterThanZero()) {
    while (tmpTime.GreaterThanZero()) {
      SAdvancementResults res = VAdvanceView(tmpTime);
      tmpTime = res.x0_remTime;
    }
  } else if (x4_sourceInfo->HasPOIData()) {
    UpdatePOIStates();
    if (!time.GreaterThanZero()) {
      x14_passedBoolCount = 0;
      x18_passedIntCount = 0;
      x1c_passedParticleCount = 0;
      x20_passedSoundCount = 0;
    }
  }
}

void CAnimSourceReaderBase::UpdatePOIStates() {
  const std::vector<CBoolPOINode>& boolNodes = x4_sourceInfo->GetBoolPOIStream();
  const std::vector<CInt32POINode>& int32Nodes = x4_sourceInfo->GetInt32POIStream();
  const std::vector<CParticlePOINode>& particleNodes = x4_sourceInfo->GetParticlePOIStream();
  const std::vector<CSoundPOINode>& soundNodes = x4_sourceInfo->GetSoundPOIStream();

  while (x14_passedBoolCount < boolNodes.size() && boolNodes[x14_passedBoolCount].GetTime() <= xc_curTime) {
    const auto& node = boolNodes[x14_passedBoolCount];
    if (node.GetIndex() >= 0) {
      x24_boolStates[node.GetIndex()].second = node.GetValue();
    }
    ++x14_passedBoolCount;
  }

  while (x18_passedIntCount < int32Nodes.size() && int32Nodes[x18_passedIntCount].GetTime() <= xc_curTime) {
    const auto& node = int32Nodes[x18_passedIntCount];
    if (node.GetIndex() >= 0) {
      x34_int32States[node.GetIndex()].second = node.GetValue();
    }
    ++x18_passedIntCount;
  }

  while (x1c_passedParticleCount < particleNodes.size() &&
         particleNodes[x1c_passedParticleCount].GetTime() <= xc_curTime) {
    const auto& node = particleNodes[x1c_passedParticleCount];
    if (node.GetIndex() >= 0) {
      x44_particleStates[node.GetIndex()].second = node.GetParticleData().GetParentedMode();
    }
    ++x1c_passedParticleCount;
  }

  while (x20_passedSoundCount < soundNodes.size() && soundNodes[x20_passedSoundCount].GetTime() <= xc_curTime) {
    ++x20_passedSoundCount;
  }
}

size_t CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity,
                                              size_t iterator, u32 unk) const {
  if (x4_sourceInfo->HasPOIData()) {
    const std::vector<CBoolPOINode>& boolNodes = x4_sourceInfo->GetBoolPOIStream();
    return _getPOIList(time, listOut, capacity, iterator, unk, boolNodes, xc_curTime, *x4_sourceInfo,
                       x14_passedBoolCount);
  }
  return 0;
}

size_t CAnimSourceReaderBase::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity,
                                               size_t iterator, u32 unk) const {
  if (x4_sourceInfo->HasPOIData()) {
    const std::vector<CInt32POINode>& int32Nodes = x4_sourceInfo->GetInt32POIStream();
    return _getPOIList(time, listOut, capacity, iterator, unk, int32Nodes, xc_curTime, *x4_sourceInfo,
                       x18_passedIntCount);
  }
  return 0;
}

size_t CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity,
                                                  size_t iterator, u32 unk) const {
  if (x4_sourceInfo->HasPOIData()) {
    const std::vector<CParticlePOINode>& particleNodes = x4_sourceInfo->GetParticlePOIStream();
    return _getPOIList(time, listOut, capacity, iterator, unk, particleNodes, xc_curTime, *x4_sourceInfo,
                       x1c_passedParticleCount);
  }
  return 0;
}

size_t CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity,
                                               size_t iterator, u32 unk) const {
  if (x4_sourceInfo->HasPOIData()) {
    const std::vector<CSoundPOINode>& soundNodes = x4_sourceInfo->GetSoundPOIStream();
    return _getPOIList(time, listOut, capacity, iterator, unk, soundNodes, xc_curTime, *x4_sourceInfo,
                       x20_passedSoundCount);
  }
  return 0;
}

bool CAnimSourceReaderBase::VGetBoolPOIState(std::string_view name) const {
  const auto iter = std::find_if(x24_boolStates.cbegin(), x24_boolStates.cend(),
                                 [name](const auto& entry) { return entry.first == name; });

  if (iter == x24_boolStates.cend()) {
    return false;
  }

  return iter->second;
}

s32 CAnimSourceReaderBase::VGetInt32POIState(std::string_view name) const {
  const auto iter = std::find_if(x34_int32States.cbegin(), x34_int32States.cend(),
                                 [name](const auto& entry) { return entry.first == name; });

  if (iter == x34_int32States.cend()) {
    return 0;
  }

  return iter->second;
}

CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(std::string_view name) const {
  const auto iter = std::find_if(x44_particleStates.cbegin(), x44_particleStates.cend(),
                                 [name](const auto& entry) { return entry.first == name; });

  if (iter == x44_particleStates.cend()) {
    return CParticleData::EParentedMode::Initial;
  }

  return iter->second;
}

CAnimSourceReaderBase::CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo, const CCharAnimTime& time)
: x4_sourceInfo(std::move(sourceInfo)), xc_curTime(time) {}

CAnimSourceReaderBase::CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo,
                                             const CAnimSourceReaderBase& other)
: x4_sourceInfo(std::move(sourceInfo))
, xc_curTime(other.xc_curTime)
, x14_passedBoolCount(other.x14_passedBoolCount)
, x18_passedIntCount(other.x18_passedIntCount)
, x1c_passedParticleCount(other.x1c_passedParticleCount)
, x20_passedSoundCount(other.x20_passedSoundCount)
, x24_boolStates(other.x24_boolStates)
, x34_int32States(other.x34_int32States)
, x44_particleStates(other.x44_particleStates) {}

SAdvancementResults CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                                              const CCharAnimTime& startOff) const {
  SAdvancementResults ret;
  CCharAnimTime accum = xc_curTime + startOff;

  if (xc_curTime + startOff >= x54_source->GetDuration()) {
    ret.x0_remTime = dt;
    return ret;
  } else if (dt.EqualsZero()) {
    return ret;
  } else {
    CCharAnimTime prevTime = accum;
    accum += dt;
    CCharAnimTime remTime;
    if (accum > x54_source->GetDuration()) {
      remTime = accum - x54_source->GetDuration();
      accum = x54_source->GetDuration();
    }

    zeus::CQuaternion ra = x54_source->GetRotation(3, prevTime).inverse();
    zeus::CQuaternion rb = x54_source->GetRotation(3, accum);
    ret.x0_remTime = remTime;
    ret.x8_deltas.xc_rotDelta = rb * ra;

    if (x54_source->HasOffset(3)) {
      zeus::CVector3f ta = x54_source->GetOffset(3, prevTime);
      zeus::CVector3f tb = x54_source->GetOffset(3, accum);
      ret.x8_deltas.x0_posDelta = zeus::CMatrix3f(rb.inverse()) * (tb - ta);
    }

    return ret;
  }
}

void CAnimSourceReader::VSetPhase(float phase) {
  xc_curTime = phase * x54_source->GetDuration().GetSeconds();
  if (x54_source->GetPOIData()) {
    UpdatePOIStates();
    if (!xc_curTime.GreaterThanZero()) {
      x14_passedBoolCount = 0;
      x18_passedIntCount = 0;
      x1c_passedParticleCount = 0;
      x20_passedSoundCount = 0;
    }
  }
}

SAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& dt) {
  SAdvancementResults ret;

  if (xc_curTime.EqualsZero()) {
    xc_curTime = x54_source->GetDuration();
    ret.x0_remTime = dt;
    return ret;
  } else if (dt.EqualsZero()) {
    return ret;
  } else {
    CCharAnimTime prevTime = xc_curTime;
    xc_curTime -= dt;
    CCharAnimTime remTime;
    if (xc_curTime < CCharAnimTime()) {
      remTime = CCharAnimTime() - xc_curTime;
      xc_curTime = CCharAnimTime();
    }

    if (x54_source->GetPOIData())
      UpdatePOIStates();

    zeus::CQuaternion ra = x54_source->GetRotation(3, prevTime).inverse();
    zeus::CQuaternion rb = x54_source->GetRotation(3, xc_curTime);
    ret.x0_remTime = remTime;
    ret.x8_deltas.xc_rotDelta = rb * ra;

    if (x54_source->HasOffset(3)) {
      zeus::CVector3f ta = x54_source->GetOffset(3, prevTime);
      zeus::CVector3f tb = x54_source->GetOffset(3, xc_curTime);
      ret.x8_deltas.x0_posDelta = zeus::CMatrix3f(rb.inverse()) * (tb - ta);
    }

    return ret;
  }
}

std::unique_ptr<IAnimReader> CAnimSourceReader::VClone() const { return std::make_unique<CAnimSourceReader>(*this); }

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
  x54_source->GetSegStatementSet(list, setOut, xc_curTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                            const CCharAnimTime& time) const {
  x54_source->GetSegStatementSet(list, setOut, time);
}

SAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& dt) {
  SAdvancementResults ret;

  if (xc_curTime == x54_source->GetDuration()) {
    xc_curTime = {};
    x14_passedBoolCount = 0;
    x18_passedIntCount = 0;
    x1c_passedParticleCount = 0;
    x20_passedSoundCount = 0;
    ret.x0_remTime = dt;
    return ret;
  } else if (dt.EqualsZero()) {
    return ret;
  } else {
    CCharAnimTime prevTime = xc_curTime;
    xc_curTime += dt;
    CCharAnimTime remTime;
    if (xc_curTime > x54_source->GetDuration()) {
      remTime = xc_curTime - x54_source->GetDuration();
      xc_curTime = x54_source->GetDuration();
    }

    if (x54_source->GetPOIData())
      UpdatePOIStates();

    zeus::CQuaternion ra = x54_source->GetRotation(3, prevTime).inverse();
    zeus::CQuaternion rb = x54_source->GetRotation(3, xc_curTime);
    ret.x0_remTime = remTime;
    ret.x8_deltas.xc_rotDelta = rb * ra;

    if (x54_source->HasOffset(3)) {
      zeus::CVector3f ta = x54_source->GetOffset(3, prevTime);
      zeus::CVector3f tb = x54_source->GetOffset(3, xc_curTime);
      ret.x8_deltas.x0_posDelta = zeus::CMatrix3f(rb.inverse()) * (tb - ta);
    }

    return ret;
  }
}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const { return x54_source->GetDuration() - xc_curTime; }

CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const { return x64_steadyStateInfo; }

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const { return x54_source->HasOffset(seg); }

zeus::CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return x54_source->GetOffset(seg, xc_curTime);
}

zeus::CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  return x54_source->GetOffset(seg, time);
}

zeus::CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return x54_source->GetRotation(seg, xc_curTime);
}

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken<CAnimSource>& source, const CCharAnimTime& time)
: CAnimSourceReaderBase(std::make_unique<CAnimSourceInfo>(source), {})
, x54_source(source)
, x64_steadyStateInfo(false, source->GetDuration(), source->GetOffset(source->GetRootBoneId(), time)) {
  PostConstruct(time);
}

CAnimSourceReader::CAnimSourceReader(const CAnimSourceReader& other)
: CAnimSourceReaderBase(std::make_unique<CAnimSourceInfo>(other.x54_source), other)
, x54_source(other.x54_source)
, x64_steadyStateInfo(other.x64_steadyStateInfo) {}

} // namespace metaforce
