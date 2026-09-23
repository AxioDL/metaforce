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
  : x4_sourceInfo(sourceInfo), xc_curTime(0.f) {}

  CAnimSourceReaderBase(
      const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo, const CCharAnimTime& time,
      int passedBoolCount, int passedIntCount, int passedParticleCount, int passedSoundCount,
      const rstl::vector< rstl::pair< rstl::string, bool > >& boolStates,
      const rstl::vector< rstl::pair< rstl::string, int > >& intStates,
      const rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > >&
          particleStates)
  : x4_sourceInfo(sourceInfo)
  , xc_curTime(time)
  , x14_passedBoolCount(passedBoolCount)
  , x18_passedIntCount(passedIntCount)
  , x1c_passedParticleCount(passedParticleCount)
  , x20_passedSoundCount(passedSoundCount)
  , x24_boolStates(boolStates)
  , x34_int32States(intStates)
  , x44_particleStates(particleStates) {}

  void PostConstruct(const CCharAnimTime& time);
  void UpdatePOIStates();
  const IAnimSourceInfo& AnimSource() const { return *x4_sourceInfo; }
  const CCharAnimTime& GetCurTime() const { return xc_curTime; }

protected:
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueBoolPOIs() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueInt32POIs() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueParticlePOIs() const;

  rstl::object_owner< IAnimSourceInfo > x4_sourceInfo;
  CCharAnimTime xc_curTime;
  int x14_passedBoolCount;
  int x18_passedIntCount;
  int x1c_passedParticleCount;
  int x20_passedSoundCount;
  rstl::vector< rstl::pair< rstl::string, bool > > x24_boolStates;
  rstl::vector< rstl::pair< rstl::string, int > > x34_int32States;
  rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > > x44_particleStates;
};
CHECK_SIZEOF(CAnimSourceReaderBase, 0x54)

#endif // _CANIMSOURCEREADERBASE
