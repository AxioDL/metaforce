#ifndef _CANIMSOURCEREADERBASE
#define _CANIMSOURCEREADERBASE

#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/IAnimSourceInfo.hpp"
#include "rstl/object_owner.hpp"
#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CAnimSourceReaderBase : public IAnimReader {
public:
  ~CAnimSourceReaderBase() override {}
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int unk) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int unk) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;

  CAnimSourceReaderBase(const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo)
  : mSourceInfo(sourceInfo), mCurTime(0.f) {}

  CAnimSourceReaderBase(
      const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo, const CCharAnimTime& time,
      int passedBoolCount, int passedIntCount, int passedParticleCount, int passedSoundCount,
      const rstl::vector< rstl::pair< rstl::string, bool > >& boolStates,
      const rstl::vector< rstl::pair< rstl::string, int > >& intStates,
      const rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > >&
          particleStates)
  : mSourceInfo(sourceInfo)
  , mCurTime(time)
  , mPassedBoolCount(passedBoolCount)
  , mPassedIntCount(passedIntCount)
  , mPassedParticleCount(passedParticleCount)
  , mPassedSoundCount(passedSoundCount)
  , mBoolStates(boolStates)
  , mInt32States(intStates)
  , mParticleStates(particleStates) {}

  void PostConstruct(const CCharAnimTime& time);
  void UpdatePOIStates();
  const IAnimSourceInfo& AnimSource() const { return *mSourceInfo; }
  const CCharAnimTime& GetCurTime() const { return mCurTime; }

protected:
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueBoolPOIs() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueInt32POIs() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueParticlePOIs() const;

  rstl::object_owner< IAnimSourceInfo > mSourceInfo;
  CCharAnimTime mCurTime;
  int mPassedBoolCount;
  int mPassedIntCount;
  int mPassedParticleCount;
  int mPassedSoundCount;
  rstl::vector< rstl::pair< rstl::string, bool > > mBoolStates;
  rstl::vector< rstl::pair< rstl::string, int > > mInt32States;
  rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > > mParticleStates;
};
CHECK_SIZEOF(CAnimSourceReaderBase, 0x54)

#endif // _CANIMSOURCEREADERBASE
