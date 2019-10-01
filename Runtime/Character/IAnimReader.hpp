#pragma once

#include <memory>
#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAllFormatsAnimSource.hpp"
#include "Runtime/Character/CCharAnimTime.hpp"
#include "Runtime/Character/CParticleData.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSegId;
class CSegIdList;
class CSegStatementSet;
class CSoundPOINode;

struct SAdvancementDeltas {
  zeus::CVector3f x0_posDelta;
  zeus::CQuaternion xc_rotDelta;

  static SAdvancementDeltas Interpolate(const SAdvancementDeltas& a, const SAdvancementDeltas& b, float oldWeight,
                                        float newWeight);
  static SAdvancementDeltas Blend(const SAdvancementDeltas& a, const SAdvancementDeltas& b, float w);
};

struct SAdvancementResults {
  CCharAnimTime x0_remTime;
  SAdvancementDeltas x8_deltas;
};

class CSteadyStateAnimInfo {
  CCharAnimTime x0_duration;
  zeus::CVector3f x8_offset;
  bool x14_looping = false;

public:
  CSteadyStateAnimInfo(bool looping, const CCharAnimTime& duration, const zeus::CVector3f& offset)
  : x0_duration(duration), x8_offset(offset), x14_looping(looping) {}

  const CCharAnimTime& GetDuration() const { return x0_duration; }
  const zeus::CVector3f& GetOffset() const { return x8_offset; }
  bool IsLooping() const { return x14_looping; }
};

struct CAnimTreeEffectiveContribution {
  float x0_contributionWeight;
  std::string x4_name;
  CSteadyStateAnimInfo x14_ssInfo;
  CCharAnimTime x2c_remTime;
  u32 x34_dbIdx;

public:
  CAnimTreeEffectiveContribution(float cweight, std::string_view name, const CSteadyStateAnimInfo& ssInfo,
                                 const CCharAnimTime& remTime, u32 dbIdx)
  : x0_contributionWeight(cweight), x4_name(name), x14_ssInfo(ssInfo), x2c_remTime(remTime), x34_dbIdx(dbIdx) {}
  float GetContributionWeight() const { return x0_contributionWeight; }
  std::string_view GetPrimitiveName() const { return x4_name; }
  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return x14_ssInfo; }
  const CCharAnimTime& GetTimeRemaining() const { return x2c_remTime; }
  u32 GetAnimDatabaseIndex() const { return x34_dbIdx; }
};

template <class T>
class TSubAnimTypeToken : public TLockedToken<CAllFormatsAnimSource> {};

template <>
class TSubAnimTypeToken<CAnimSource> : public TLockedToken<CAllFormatsAnimSource> {
public:
  TSubAnimTypeToken<CAnimSource>(const TLockedToken<CAllFormatsAnimSource>& token)
  : TLockedToken<CAllFormatsAnimSource>(token) {}

  const CAnimSource* GetObj() const {
    const CAllFormatsAnimSource* source = TLockedToken<CAllFormatsAnimSource>::GetObj();
    return &source->GetAsCAnimSource();
  }
  const CAnimSource* operator->() const { return GetObj(); }
  const CAnimSource& operator*() const { return *GetObj(); }
};

template <>
class TSubAnimTypeToken<CFBStreamedCompression> : public TLockedToken<CAllFormatsAnimSource> {
public:
  TSubAnimTypeToken<CFBStreamedCompression>(const TLockedToken<CAllFormatsAnimSource>& token)
  : TLockedToken<CAllFormatsAnimSource>(token) {}

  const CFBStreamedCompression* GetObj() const {
    const CAllFormatsAnimSource* source = TLockedToken<CAllFormatsAnimSource>::GetObj();
    return &source->GetAsCFBStreamedCompression();
  }
  const CFBStreamedCompression* operator->() const { return GetObj(); }
  const CFBStreamedCompression& operator*() const { return *GetObj(); }
};

class IAnimReader {
public:
  virtual ~IAnimReader() = default;
  virtual bool IsCAnimTreeNode() const { return false; }
  virtual SAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;
  virtual CCharAnimTime VGetTimeRemaining() const = 0;
  virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const = 0;
  virtual bool VHasOffset(const CSegId& seg) const = 0;
  virtual zeus::CVector3f VGetOffset(const CSegId& seg) const = 0;
  virtual zeus::CQuaternion VGetRotation(const CSegId& seg) const = 0;
  virtual u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator,
                              u32) const = 0;
  virtual u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                               u32) const = 0;
  virtual u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator,
                                  u32) const = 0;
  virtual u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                               u32) const = 0;
  virtual bool VGetBoolPOIState(const char*) const = 0;
  virtual s32 VGetInt32POIState(const char*) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(const char*) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                   const CCharAnimTime& time) const = 0;
  virtual std::unique_ptr<IAnimReader> VClone() const = 0;
  virtual std::optional<std::unique_ptr<IAnimReader>> VSimplified() { return {}; }
  virtual void VSetPhase(float) = 0;
  virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;

  u32 GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;

  std::optional<std::unique_ptr<IAnimReader>> Simplified() { return VSimplified(); }

  std::unique_ptr<IAnimReader> Clone() const { return VClone(); }
};

} // namespace urde
