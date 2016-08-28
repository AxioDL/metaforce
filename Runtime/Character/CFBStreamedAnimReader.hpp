#ifndef __URDE_CFSTREAMEDANIMREADER_HPP__
#define __URDE_CFSTREAMEDANIMREADER_HPP__

#include "CAnimSourceReader.hpp"
#include "CFBStreamedCompression.hpp"

namespace urde
{
class CBitLevelLoader;

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
    friend class CSegIdToIndexConverter;
    std::unique_ptr<u8[]> x0_buffer;
    s32* x4_cumulativeInts32; /* Used to be 16 per channel */
    u8* x8_hasTrans1;
    u16* xc_segIds2;
    float* x10_computedFloats32;
    u32 x14_rotDiv;
    float x18_transMult;
    u32 x1c_curKey = 0;
    bool x20_ = false;
    u32 x24_boneChanCount;
    void Allocate(u32 chanCount);
    void Initialize(const CFBStreamedCompression& source);
public:
    CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
    void IncrementInto(CBitLevelLoader& loader, const CFBStreamedCompression& source,
                       CFBStreamedAnimReaderTotals& dest);
    void CalculateDown();
};

class CFBStreamedPairOfTotals
{
    friend class CFBStreamedAnimReader;

    TSubAnimTypeToken<CFBStreamedCompression> x0_source;
    u32* xc_rotsAndOffs;
    bool x10_ = true;
    CFBStreamedAnimReaderTotals x14_;
    CFBStreamedAnimReaderTotals x3c_;
public:
    CFBStreamedPairOfTotals(const TSubAnimTypeToken<CFBStreamedCompression>& source);
};

class CBitLevelLoader
{
    const u8* m_data;
    size_t m_bitIdx = 0;
public:
    CBitLevelLoader(const void* data)
    : m_data(reinterpret_cast<const u8*>(data)) {}
    u32 LoadUnsigned(u8 q);
    s32 LoadSigned(u8 q);
    bool LoadBool();
};

class CSegIdToIndexConverter
{
    u32 x0_indices[96] = {-1};
public:
    CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals);
    u32 SegIdToIndex(const CSegId& id) const { return x0_indices[id]; }
};

class CFBStreamedAnimReader : public CAnimSourceReaderBase
{
    TSubAnimTypeToken<CFBStreamedCompression> x54_source;
    CSteadyStateAnimInfo x64_steadyStateInfo;
    CFBStreamedPairOfTotals x7c_totals;
    const u8* x88_bitstreamData;
    CBitLevelLoader x8c_bitLoader;
    CSegIdToIndexConverter x114_segIdToIndex;
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
