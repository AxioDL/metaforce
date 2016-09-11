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
    TAnimSourceInfo(const TSubAnimTypeToken<T>& token) : x4_token(token) {}
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
    friend class CFBStreamedPairOfTotals;
    friend class CFBStreamedAnimReader;
    std::unique_ptr<u8[]> x0_buffer;
    s32* x4_cumulativeInts32; /* Used to be 16 per channel */
    u8* x8_hasTrans1;
    u16* xc_segIds2;
    float* x10_computedFloats32;
    u32 x14_rotDiv;
    float x18_transMult;
    u32 x1c_curKey = 0;
    bool x20_calculated = false;
    u32 x24_boneChanCount;
    void Allocate(u32 chanCount);
public:
    CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
    void Initialize(const CFBStreamedCompression& source);
    void IncrementInto(CBitLevelLoader& loader, const CFBStreamedCompression& source,
                       CFBStreamedAnimReaderTotals& dest);
    void CalculateDown();
    bool IsCalculated() const { return x20_calculated; }
    const float* GetFloats(int chanIdx) const { return &x10_computedFloats32[chanIdx*8]; }
};

class CFBStreamedPairOfTotals
{
    friend class CFBStreamedAnimReader;

    TSubAnimTypeToken<CFBStreamedCompression> x0_source;
    u32* xc_rotsAndOffs;
    bool x10_nextSel = true;
    CFBStreamedAnimReaderTotals x14_a;
    CFBStreamedAnimReaderTotals x3c_b;
    float x78_t = 0.f;
public:
    CFBStreamedPairOfTotals(const TSubAnimTypeToken<CFBStreamedCompression>& source);
    void SetTime(CBitLevelLoader& loader, const CCharAnimTime& time);
    void DoIncrement(CBitLevelLoader& loader);
    float GetT() const { return x78_t; }
    CFBStreamedAnimReaderTotals& Next() { return x10_nextSel ? x3c_b : x14_a; }
    CFBStreamedAnimReaderTotals& Prior() { return x10_nextSel ? x14_a : x3c_b; }
    const CFBStreamedAnimReaderTotals& Next() const { return x10_nextSel ? x3c_b : x14_a; }
    const CFBStreamedAnimReaderTotals& Prior() const { return x10_nextSel ? x14_a : x3c_b; }
};

class CBitLevelLoader
{
    const u8* m_data;
    size_t m_bitIdx = 0;
public:
    CBitLevelLoader(const void* data)
    : m_data(reinterpret_cast<const u8*>(data)) {}
    void Reset() { m_bitIdx = 0; }
    u32 LoadUnsigned(u8 q);
    s32 LoadSigned(u8 q);
    bool LoadBool();
    size_t GetCurBit() const { return m_bitIdx; }
};

class CSegIdToIndexConverter
{
    u32 x0_indices[96] = {u32(-1)};
public:
    CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals);
    u32 SegIdToIndex(const CSegId& id) const { return x0_indices[id]; }
};

class CFBStreamedAnimReader : public CAnimSourceReaderBase
{
    TSubAnimTypeToken<CFBStreamedCompression> x54_source;
    CSteadyStateAnimInfo x64_steadyStateInfo;
    CFBStreamedPairOfTotals x7c_totals;
    const u8* x104_bitstreamData;
    CBitLevelLoader x108_bitLoader;
    CSegIdToIndexConverter x114_segIdToIndex;
    bool HasOffset(const CSegId& seg) const;
    zeus::CVector3f GetOffset(const CSegId& seg) const;
    zeus::CQuaternion GetRotation(const CSegId& seg) const;
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
