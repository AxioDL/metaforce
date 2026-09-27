#ifndef _CANIMSOURCEREADER
#define _CANIMSOURCEREADER

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"

class CAnimSourceInfo : public IAnimSourceInfo {
public:
  ~CAnimSourceInfo() override {}

  bool HasPOIData() const override { return mToken->HasPOIData(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const override {
    return mToken->GetBoolPOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const override {
    return mToken->GetInt32POIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const override {
    return mToken->GetParticlePOIStream();
  }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const override {
    return mToken->GetSoundPOIStream();
  }
  CCharAnimTime GetAnimationDuration() const override { return mToken->GetAnimationDuration(); }

  CAnimSourceInfo(const TSubAnimTypeToken< CAnimSource >& source) : mToken(source) {}

private:
  TSubAnimTypeToken< CAnimSource > mToken;
};
CHECK_SIZEOF(CAnimSourceInfo, 0x14)

class CAnimSourceReader : public CAnimSourceReaderBase {
public:
  ~CAnimSourceReader() override {}
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void VSetPhase(float phase) override;
  CAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
  virtual CVector3f VGetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  virtual bool VSupportsReverseView() const;
  virtual CAdvancementResults VReverseView(const CCharAnimTime& time);

  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time);

private:
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time,
                    const CSteadyStateAnimInfo& steadyStateInfo, int passedBoolCount,
                    int passedIntCount, int passedParticleCount, int passedSoundCount,
                    const rstl::vector< rstl::pair< rstl::string, bool > >& boolStates,
                    const rstl::vector< rstl::pair< rstl::string, int > >& intStates,
                    const rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > >&
                        particleStates)
  : CAnimSourceReaderBase(rs_new CAnimSourceInfo(source), time, passedBoolCount, passedIntCount,
                          passedParticleCount, passedSoundCount, boolStates, intStates,
                          particleStates)
  , mSource(source)
  , mSteadyStateInfo(steadyStateInfo) {}

  TSubAnimTypeToken< CAnimSource > mSource;
  CSteadyStateAnimInfo mSteadyStateInfo;
};
CHECK_SIZEOF(CAnimSourceReader, 0x7c)

#endif // _CANIMSOURCEREADER
