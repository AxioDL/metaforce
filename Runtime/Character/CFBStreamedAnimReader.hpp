#ifndef __URDE_CFSTREAMEDANIMREADER_HPP__
#define __URDE_CFSTREAMEDANIMREADER_HPP__

#include "CAnimSourceReader.hpp"
#include "CFBStreamedCompression.hpp"

namespace urde
{

template <class T>
class TAnimSourceInfo : public IAnimSourceInfo
{
    TSubAnimTypeToken<T> x4_token;
public:
    TAnimSourceInfo(const TSubAnimTypeToken<T>& token);
    bool HasPOIData() const { return x4_token->HasPOIData(); }
    const std::vector<CBoolPOINode>& GetBoolPOIStream() const { return x4_token->GetBoolPOIStream(); }
    const std::vector<CInt32POINode>& GetInt32POIStream() const { return x4_token->GetInt32POIStream(); }
    const std::vector<CParticlePOINode>& GetParticlePOIStream() const { return x4_token->GetParticlePOIStream(); }
    const std::vector<CSoundPOINode>& GetSoundPOIStream() const { return x4_token->GetSoundPOIStream(); }
    CCharAnimTime GetAnimationDuration() const { return x4_token->GetAnimationDuration(); }
};

class CFBStreamedAnimReaderTotals
{
    std::unique_ptr<u8[]> x0_buffer;
    u8* x4_first16;
    u8* x8_second1;
    u8* xc_third2;
    u8* x10_fourth32;
    u32 x14_rotDiv;
    float x18_transMult;
    u32 x1c_ = 0;
    bool x20_ = false;
    u32 x24_boneChanCount;
    void Allocate(u32 chanCount);
public:
    CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
};

class CFBStreamedPairOfTotals
{
    friend class CFBStreamedAnimReader;

    TSubAnimTypeToken<CFBStreamedCompression> x0_source;
    u32* xc_rotsAndOffs;
    bool x10_ = true;
    CFBStreamedAnimReaderTotals x14_;
    CFBStreamedAnimReaderTotals x3c_;
    const u8* x88_;
    const u8** x8c_;
    u32 x90_;
    u32 x94_ = 0;
public:
    CFBStreamedPairOfTotals(const TSubAnimTypeToken<CFBStreamedCompression>& source);
};

class CSegIdToIndexConverter
{
    u32 x0_indices[96] = {-1};
public:
    CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals);
};

class CFBStreamedAnimReader : public CAnimSourceReaderBase
{
    TSubAnimTypeToken<CFBStreamedCompression> x54_source;
    CSteadyStateAnimInfo x64_steadyStateInfo;
    CFBStreamedPairOfTotals x7c_;
    CSegIdToIndexConverter x114_;
public:
    CFBStreamedAnimReader(const TSubAnimTypeToken<CFBStreamedCompression>& source, const CCharAnimTime& time);

    SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
    bool VSupportsReverseView() const {return false;}
    void VSetPhase(float);
    SAdvancementResults VReverseView(const CCharAnimTime& time);
    std::shared_ptr<IAnimReader> VClone() const;
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

}

#endif // __URDE_CFSTREAMEDANIMREADER_HPP__
