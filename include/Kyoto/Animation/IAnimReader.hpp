#ifndef _IANIMREADER
#define _IANIMREADER

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"
#include "Kyoto/Particles/CParticleData.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/ownership_transfer.hpp"
#include "rstl/string.hpp"

struct CAdvancementResults {
  CCharAnimTime x0_remTime;
  CAdvancementDeltas x8_deltas;
  const CCharAnimTime& GetRemainder() const { return x0_remTime; }
  const CAdvancementDeltas& GetAdvancementDeltas() const { return x8_deltas; }
  CAdvancementResults() {}
  CAdvancementResults(const CCharAnimTime& time) : x0_remTime(time) {}
  CAdvancementResults(const CCharAnimTime& time, const CAdvancementDeltas& deltas)
  : x0_remTime(time), x8_deltas(deltas) {}

  static CAdvancementResults RemainderOnly(const CCharAnimTime& time) {
    return CAdvancementResults(time);
  }
};

CHECK_SIZEOF(CAdvancementResults, 0x24)

struct CAnimTreeEffectiveContribution {
  float x0_contributionWeight;
  rstl::string x4_name;
  CSteadyStateAnimInfo x14_ssInfo;
  CCharAnimTime x2c_remTime;
  u32 x34_dbIdx;

public:
  CAnimTreeEffectiveContribution(float cweight, const rstl::string& name,
                                 const CSteadyStateAnimInfo& ssInfo, const CCharAnimTime& remTime,
                                 u32 dbIdx)
  : x0_contributionWeight(cweight)
  , x4_name(name)
  , x14_ssInfo(ssInfo)
  , x2c_remTime(remTime)
  , x34_dbIdx(dbIdx) {}
  float GetContributionWeight() const { return x0_contributionWeight; }
  const rstl::string& GetPrimitiveName() const { return x4_name; }
  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return x14_ssInfo; }
  const CCharAnimTime& GetTimeRemaining() const { return x2c_remTime; }
  u32 GetAnimDatabaseIndex() const { return x34_dbIdx; }
  float GetPhase() const {
    return rstl::min_val(rstl::max_val(1.f - x2c_remTime / x14_ssInfo.GetDuration(), 0.f), 1.f);
  }
};

class CSegId;
class CSegIdList;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class CSegStatementSet;

class IAnimReader {
public:
  virtual ~IAnimReader();
  virtual bool IsCAnimTreeNode() const;
  virtual CAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;
  virtual CCharAnimTime VGetTimeRemaining() const = 0;
  virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const = 0;
  virtual bool VHasOffset(const CSegId& seg) const = 0;
  virtual CVector3f VGetOffset(const CSegId& seg) const = 0;
  virtual CQuaternion VGetRotation(const CSegId& seg) const = 0;
  virtual uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                               uint iterator, int) const = 0;
  virtual uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                uint iterator, int) const = 0;
  virtual uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                   uint capacity, uint iterator, int) const = 0;
  virtual uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                                uint iterator, int) const = 0;
  virtual bool VGetBoolPOIState(const char* name) const = 0;
  virtual s32 VGetInt32POIState(const char* name) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(const char* name) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                   const CCharAnimTime& time) const = 0;
  virtual rstl::ownership_transfer< IAnimReader > VClone() const = 0;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified();
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > Simplified();
  virtual void VSetPhase(float) = 0;
  virtual CAdvancementResults VGetAdvancementResults(const CCharAnimTime& aTime,
                                                     const CCharAnimTime& bTime) const;

  uint GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                      uint iterator, int unk) const;

  uint GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                       uint iterator, int unk) const;

  uint GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                          uint iterator, int unk) const;

  uint GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                       uint iterator, int unk) const;

  rstl::ownership_transfer< IAnimReader > Clone() const { return VClone(); }
  CCharAnimTime GetTimeRemaining() const { return VGetTimeRemaining(); }
  bool HasOffset(const CSegId& seg) const { return VHasOffset(seg); }
  CVector3f GetOffset(const CSegId& seg) const { return VGetOffset(seg); }
  CQuaternion GetRotation(const CSegId& seg) const { return VGetRotation(seg); }
  CAdvancementResults GetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const {
    return VGetAdvancementResults(a, b);
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo() const { return VGetSteadyStateAnimInfo(); }
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
    VGetSegStatementSet(list, setOut);
  }
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                          const CCharAnimTime& time) const {
    VGetSegStatementSet(list, setOut, time);
  }
  void SetPhase(float phase) { VSetPhase(phase); }
  CAdvancementResults AdvanceView(const CCharAnimTime& time) { return VAdvanceView(time); }
  bool GetBoolPOIState(const char* name) const { return VGetBoolPOIState(name); }
  s32 GetInt32POIState(const char* name) const { return VGetInt32POIState(name); }
  CParticleData::EParentedMode GetParticlePOIState(const char* name) const {
    return VGetParticlePOIState(name);
  }
};

#endif // _IANIMREADER
