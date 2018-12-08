#pragma once

#include "IAnimReader.hpp"
#include "CToken.hpp"
#include "CAnimSource.hpp"
#include "CParticleData.hpp"
#include <set>

namespace urde {

class IAnimSourceInfo {
public:
  virtual ~IAnimSourceInfo() = default;
  virtual bool HasPOIData() const = 0;
  virtual const std::vector<CBoolPOINode>& GetBoolPOIStream() const = 0;
  virtual const std::vector<CInt32POINode>& GetInt32POIStream() const = 0;
  virtual const std::vector<CParticlePOINode>& GetParticlePOIStream() const = 0;
  virtual const std::vector<CSoundPOINode>& GetSoundPOIStream() const = 0;
  virtual CCharAnimTime GetAnimationDuration() const = 0;
};

class CAnimSourceInfo : public IAnimSourceInfo {
  TSubAnimTypeToken<CAnimSource> x4_token;

public:
  CAnimSourceInfo(const TSubAnimTypeToken<CAnimSource>& token);
  bool HasPOIData() const;
  const std::vector<CBoolPOINode>& GetBoolPOIStream() const;
  const std::vector<CInt32POINode>& GetInt32POIStream() const;
  const std::vector<CParticlePOINode>& GetParticlePOIStream() const;
  const std::vector<CSoundPOINode>& GetSoundPOIStream() const;
  CCharAnimTime GetAnimationDuration() const;
};

class CAnimSourceReaderBase : public IAnimReader {
protected:
  std::unique_ptr<IAnimSourceInfo> x4_sourceInfo;
  CCharAnimTime xc_curTime;
  u32 x14_passedBoolCount = 0;
  u32 x18_passedIntCount = 0;
  u32 x1c_passedParticleCount = 0;
  u32 x20_passedSoundCount = 0;
  std::vector<std::pair<std::string, bool>> x24_boolStates;
  std::vector<std::pair<std::string, s32>> x34_int32States;
  std::vector<std::pair<std::string, CParticleData::EParentedMode>> x44_particleStates;

  std::set<std::pair<std::string, s32>> GetUniqueParticlePOIs() const;
  std::set<std::pair<std::string, s32>> GetUniqueInt32POIs() const;
  std::set<std::pair<std::string, s32>> GetUniqueBoolPOIs() const;

protected:
  void PostConstruct(const CCharAnimTime& time);
  void UpdatePOIStates();
  CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo, const CAnimSourceReaderBase& other);

public:
  CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo, const CCharAnimTime& time);

  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  bool VGetBoolPOIState(const char* name) const;
  s32 VGetInt32POIState(const char* name) const;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const;

  using IAnimReader::VGetOffset;
  virtual zeus::CVector3f VGetOffset(const CSegId& seg, const CCharAnimTime& b) const = 0;
  virtual bool VSupportsReverseView() const = 0;
  virtual SAdvancementResults VReverseView(const CCharAnimTime& time) = 0;

  const CCharAnimTime& GetCurTime() const { return xc_curTime; }
};

class CAnimSourceReader : public CAnimSourceReaderBase {
  TSubAnimTypeToken<CAnimSource> x54_source;
  CSteadyStateAnimInfo x64_steadyStateInfo;

public:
  CAnimSourceReader(const TSubAnimTypeToken<CAnimSource>& source, const CCharAnimTime& time);
  CAnimSourceReader(const CAnimSourceReader& other);

  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
  bool VSupportsReverseView() const { return true; }
  void VSetPhase(float);
  SAdvancementResults VReverseView(const CCharAnimTime& time);
  std::unique_ptr<IAnimReader> VClone() const;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const;
  SAdvancementResults VAdvanceView(const CCharAnimTime& a);
  CCharAnimTime VGetTimeRemaining() const;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
  bool VHasOffset(const CSegId& seg) const;
  zeus::CVector3f VGetOffset(const CSegId& seg) const;
  zeus::CVector3f VGetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  zeus::CQuaternion VGetRotation(const CSegId& seg) const;
};

} // namespace urde
