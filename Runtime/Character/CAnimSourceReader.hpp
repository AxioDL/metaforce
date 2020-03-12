#pragma once

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CAnimSource.hpp"
#include "Runtime/Character/CParticleData.hpp"
#include "Runtime/Character/IAnimReader.hpp"

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
  explicit CAnimSourceInfo(TSubAnimTypeToken<CAnimSource> token);
  bool HasPOIData() const override;
  const std::vector<CBoolPOINode>& GetBoolPOIStream() const override;
  const std::vector<CInt32POINode>& GetInt32POIStream() const override;
  const std::vector<CParticlePOINode>& GetParticlePOIStream() const override;
  const std::vector<CSoundPOINode>& GetSoundPOIStream() const override;
  CCharAnimTime GetAnimationDuration() const override;
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

  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const override;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator,
                          u32) const override;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;

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

  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const override;
  bool VSupportsReverseView() const override { return true; }
  void VSetPhase(float) override;
  SAdvancementResults VReverseView(const CCharAnimTime& time) override;
  std::unique_ptr<IAnimReader> VClone() const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& a) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  zeus::CVector3f VGetOffset(const CSegId& seg) const override;
  zeus::CVector3f VGetOffset(const CSegId& seg, const CCharAnimTime& time) const override;
  zeus::CQuaternion VGetRotation(const CSegId& seg) const override;
};

} // namespace urde
