#ifndef __PSHAG_CANIMSOURCEREADER_HPP__
#define __PSHAG_CANIMSOURCEREADER_HPP__

#include "IAnimReader.hpp"
#include "CToken.hpp"
#include "CAnimSource.hpp"
#include "CParticleData.hpp"

namespace urde
{

template <class T>
class TSubAnimTypeToken : public TCachedToken<T>
{
};

class IAnimSourceInfo
{
public:
    virtual ~IAnimSourceInfo() = default;
    virtual bool HasPOIData() const=0;
    virtual void GetBoolPOIStream() const=0;
    virtual void GetInt32POIStream() const=0;
    virtual void GetParticlePOIStream() const=0;
    virtual void GetSoundPOIStream() const=0;
    virtual void GetAnimationDuration() const=0;
};

class CAnimSourceInfo : public IAnimSourceInfo
{
    TSubAnimTypeToken<CAnimSource> x4_token;
public:
    CAnimSourceInfo(const TSubAnimTypeToken<CAnimSource>& token);
    bool HasPOIData() const;
    void GetBoolPOIStream() const;
    void GetInt32POIStream() const;
    void GetParticlePOIStream() const;
    void GetSoundPOIStream() const;
    void GetAnimationDuration() const;
};

class CAnimSourceReaderBase : public IAnimReader
{
    std::unique_ptr<IAnimSourceInfo> x4_sourceInfo;
    CCharAnimTime xc_curTime;
    u32 x14_passedBoolCount;
    u32 x18_passedIntCount;
    u32 x1c_passedParticleCount;
    u32 x20_passedSoundCount;
    std::vector<std::pair<std::string, bool>> x24_boolPOIs;
    std::vector<std::pair<std::string, s32>> x34_int32POIs;
    std::vector<std::pair<std::string, CParticleData::EParentedMode>> x44_particlePOIs;

    std::map<std::string, CParticleData::EParentedMode> GetUniqueParticlePOIs() const;
    std::map<std::string, s32> GetUniqueInt32POIs() const;
    std::map<std::string, bool> GetUniqueBoolPOIs() const;
    void PostConstruct(const CCharAnimTime& time);

protected:
    void UpdatePOIStates();

public:
    CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo,
                          const CCharAnimTime& time);

    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    void VGetBoolPOIState(const char*) const;
    void VGetInt32POIState(const char*) const;
    void VGetParticlePOIState(const char*) const;
};

class CAnimSourceReader : public CAnimSourceReaderBase
{
    TSubAnimTypeToken<CAnimSource> x54_source;
public:
    CAnimSourceReader(const TSubAnimTypeToken<CAnimSource>& source, const CCharAnimTime& time);
};

}

#endif // __PSHAG_CANIMSOURCEREADER_HPP__
