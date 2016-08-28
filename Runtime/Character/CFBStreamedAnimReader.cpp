#include "CFBStreamedAnimReader.hpp"

namespace urde
{

void CFBStreamedAnimReaderTotals::Allocate(u32 chanCount)
{
    u32 chan2 = chanCount * 2;
    u32 chan32 = chanCount * 32;

    x0_buffer.reset(new u8[chan32 + chanCount + chan2 + chan32]);
    x4_cumulativeInts32 = reinterpret_cast<s32*>(x0_buffer.get());
    x8_hasTrans1 = reinterpret_cast<u8*>(x4_cumulativeInts32 + chanCount * 8);
    xc_segIds2 = reinterpret_cast<u16*>(x8_hasTrans1 + chanCount);
    x10_computedFloats32 = reinterpret_cast<float*>(xc_segIds2 + chanCount);
}

void CFBStreamedAnimReaderTotals::Initialize(const CFBStreamedCompression& source)
{
    const u8* chans = source.GetPerChannelHeaders();
    u32 boneChanCount = *reinterpret_cast<const u32*>(chans);
    chans += 4;

    if (source.m_pc)
    {
        for (int b=0 ; b<boneChanCount ; ++b)
        {
            xc_segIds2[b] = *reinterpret_cast<const u32*>(chans);
            chans += 8;

            s32* cumulativesOut = &x4_cumulativeInts32[8*b];
            const s32* cumulativesIn = reinterpret_cast<const s32*>(chans);
            cumulativesOut[0] = cumulativesIn[0] >> 8;
            cumulativesOut[1] = cumulativesIn[1] >> 8;
            cumulativesOut[2] = cumulativesIn[2] >> 8;
            chans += 12;

            u32 tCount = *reinterpret_cast<const u32*>(chans);
            chans += 4;
            if (tCount)
            {
                x8_hasTrans1[b] = true;
                const s32* cumulativesIn = reinterpret_cast<const s32*>(chans);
                cumulativesOut[3] = cumulativesIn[0] >> 8;
                cumulativesOut[4] = cumulativesIn[1] >> 8;
                cumulativesOut[5] = cumulativesIn[2] >> 8;
                chans += 12;
            }
            else
                x8_hasTrans1[b] = false;
        }
    }
    else
    {
        for (int b=0 ; b<boneChanCount ; ++b)
        {
            xc_segIds2[b] = *reinterpret_cast<const u32*>(chans);
            chans += 6;

            s32* cumulativesOut = &x4_cumulativeInts32[8*b];
            cumulativesOut[0] = *reinterpret_cast<const s16*>(chans);
            cumulativesOut[1] = *reinterpret_cast<const s16*>(chans + 3);
            cumulativesOut[2] = *reinterpret_cast<const s16*>(chans + 6);
            chans += 9;

            u16 tCount = *reinterpret_cast<const u16*>(chans);
            chans += 2;
            if (tCount)
            {
                x8_hasTrans1[b] = true;
                cumulativesOut[3] = *reinterpret_cast<const s16*>(chans);
                cumulativesOut[4] = *reinterpret_cast<const s16*>(chans + 3);
                cumulativesOut[5] = *reinterpret_cast<const s16*>(chans + 6);
                chans += 9;
            }
            else
                x8_hasTrans1[b] = false;
        }
    }
}

CFBStreamedAnimReaderTotals::CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source)
{
    const CFBStreamedCompression::Header& header = source.MainHeader();
    x14_rotDiv = header.rotDiv;
    x18_transMult = header.translationMult;

    const u8* chans = source.GetPerChannelHeaders();
    x24_boneChanCount = *reinterpret_cast<const u32*>(chans);
    Allocate(x24_boneChanCount);
    Initialize(source);
}

void CFBStreamedAnimReaderTotals::IncrementInto(CBitLevelLoader& loader,
                                                const CFBStreamedCompression& source,
                                                CFBStreamedAnimReaderTotals& dest)
{
    const u8* chans = source.GetPerChannelHeaders();
    u32 boneChanCount = *reinterpret_cast<const u32*>(chans);
    chans += 4;

    if (source.m_pc)
    {
        for (int b=0 ; b<boneChanCount ; ++b)
        {
            chans += 8;

            const s32* cumulativesIn = &x4_cumulativeInts32[8*b];
            s32* cumulativesOut = &dest.x4_cumulativeInts32[8*b];
            const s32* qsIn = reinterpret_cast<const s32*>(chans);
            cumulativesOut[0] = loader.LoadBool();
            cumulativesOut[1] = cumulativesIn[1] + loader.LoadSigned(qsIn[0] & 0xff);
            cumulativesOut[2] = cumulativesIn[2] + loader.LoadSigned(qsIn[1] & 0xff);
            cumulativesOut[3] = cumulativesIn[3] + loader.LoadSigned(qsIn[2] & 0xff);
            chans += 12;

            u32 tCount = *reinterpret_cast<const u32*>(chans);
            chans += 4;
            if (tCount)
            {
                const s32* qsIn = reinterpret_cast<const s32*>(chans);
                cumulativesOut[4] = cumulativesIn[4] + loader.LoadSigned(qsIn[0] & 0xff);
                cumulativesOut[5] = cumulativesIn[5] + loader.LoadSigned(qsIn[1] & 0xff);
                cumulativesOut[6] = cumulativesIn[6] + loader.LoadSigned(qsIn[2] & 0xff);
                chans += 12;
            }
        }
    }
    else
    {
        for (int b=0 ; b<boneChanCount ; ++b)
        {
            chans += 6;

            const s32* cumulativesIn = &x4_cumulativeInts32[8*b];
            s32* cumulativesOut = &dest.x4_cumulativeInts32[8*b];
            cumulativesOut[0] = loader.LoadBool();
            cumulativesOut[1] = cumulativesIn[1] + loader.LoadSigned(*reinterpret_cast<const u8*>(chans + 2));
            cumulativesOut[2] = cumulativesIn[2] + loader.LoadSigned(*reinterpret_cast<const u8*>(chans + 5));
            cumulativesOut[3] = cumulativesIn[3] + loader.LoadSigned(*reinterpret_cast<const u8*>(chans + 8));
            chans += 9;

            u16 tCount = *reinterpret_cast<const u16*>(chans);
            chans += 2;
            if (tCount)
            {
                cumulativesOut[4] = cumulativesIn[4] + loader.LoadSigned(*reinterpret_cast<const u8*>(chans + 2));
                cumulativesOut[5] = cumulativesIn[5] + loader.LoadSigned(*reinterpret_cast<const u8*>(chans + 5));
                cumulativesOut[6] = cumulativesIn[5] + loader.LoadSigned(*reinterpret_cast<const u8*>(chans + 8));
                chans += 9;
            }
        }
    }
}

void CFBStreamedAnimReaderTotals::CalculateDown()
{
    for (int b=0 ; b<x24_boneChanCount ; ++b)
    {
        const s32* cumulativesIn = &x4_cumulativeInts32[8*b];
        float* compOut = &x10_computedFloats32[8*b];

        float q = M_PIF / 2.f / float(x14_rotDiv);
        compOut[1] = std::sin(cumulativesIn[1] * q);
        compOut[2] = std::sin(cumulativesIn[2] * q);
        compOut[3] = std::sin(cumulativesIn[3] * q);

        compOut[0] = std::sqrt(std::max(1.f - (compOut[1] * compOut[1] +
                                               compOut[2] * compOut[2] +
                                               compOut[3] * compOut[3]), 0.f));
        if (cumulativesIn[0])
            compOut[0] = -compOut[0];

        if (x8_hasTrans1[b])
        {
            compOut[4] = cumulativesIn[4] * x18_transMult;
            compOut[5] = cumulativesIn[5] * x18_transMult;
            compOut[6] = cumulativesIn[6] * x18_transMult;
        }
    }
}

CFBStreamedPairOfTotals::CFBStreamedPairOfTotals(const TSubAnimTypeToken<CFBStreamedCompression>& source)
: x0_source(source), xc_rotsAndOffs(source->xc_rotsAndOffs.get()), x14_(*source), x3c_(*source)
{
}

u32 CBitLevelLoader::LoadUnsigned(u8 q)
{
    u32 byteCur = (m_bitIdx / 32) * 4;
    u32 bitRem = m_bitIdx % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    u32 tempBuf = hecl::SBig(*reinterpret_cast<const u32*>(m_data + byteCur)) >> bitRem;

    /* If this shift underflows the value, buffer the next 32 bits */
    /* And tack onto shifted buffer */
    if ((bitRem + q) > 32)
    {
        u32 tempBuf2 = hecl::SBig(*reinterpret_cast<const u32*>(m_data + byteCur + 4));
        tempBuf |= (tempBuf2 << (32 - bitRem));
    }

    /* Mask it */
    u32 mask = (1 << q) - 1;
    tempBuf &= mask;

    /* Return delta value */
    m_bitIdx += q;
    return tempBuf;
}

s32 CBitLevelLoader::LoadSigned(u8 q)
{
    u32 byteCur = (m_bitIdx / 32) * 4;
    u32 bitRem = m_bitIdx % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    u32 tempBuf = hecl::SBig(*reinterpret_cast<const u32*>(m_data + byteCur)) >> bitRem;

    /* If this shift underflows the value, buffer the next 32 bits */
    /* And tack onto shifted buffer */
    if ((bitRem + q) > 32)
    {
        u32 tempBuf2 = hecl::SBig(*reinterpret_cast<const u32*>(m_data + byteCur + 4));
        tempBuf |= (tempBuf2 << (32 - bitRem));
    }

    /* Mask it */
    u32 mask = (1 << q) - 1;
    tempBuf &= mask;

    /* Sign extend */
    u32 sign = (tempBuf >> (q - 1)) & 0x1;
    if (sign)
        tempBuf |= ~0u << q;

    /* Return delta value */
    m_bitIdx += q;
    return s32(tempBuf);
}

bool CBitLevelLoader::LoadBool()
{
    u32 byteCur = (m_bitIdx / 32) * 4;
    u32 bitRem = m_bitIdx % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    u32 tempBuf = hecl::SBig(*reinterpret_cast<const u32*>(m_data + byteCur)) >> bitRem;

    /* That's it */
    m_bitIdx += 1;
    return tempBuf & 0x1;
}

CSegIdToIndexConverter::CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals)
{
    for (u32 b=0 ; b<totals.x24_boneChanCount ; ++b)
    {
        u16 segId = totals.xc_segIds2[b];
        if (segId >= 96)
            continue;
        x0_indices[segId] = b;
    }
}

CFBStreamedAnimReader::CFBStreamedAnimReader(const TSubAnimTypeToken<CFBStreamedCompression>& source, const CCharAnimTime& time)
: CAnimSourceReaderBase(std::make_unique<TAnimSourceInfo<CFBStreamedCompression>>(source), time), x54_source(source),
  x64_steadyStateInfo(source->IsLooping(), source->GetAnimationDuration(), source->GetRootOffset()),
  x7c_totals(source), x88_bitstreamData(source->GetBitstreamPointer()), x8c_bitLoader(x88_bitstreamData),
  x114_segIdToIndex(x7c_totals.x10_ ? x7c_totals.x14_ : x7c_totals.x3c_)
{
    PostConstruct(time);
}

SAdvancementResults CFBStreamedAnimReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                                                  const CCharAnimTime& startOff) const
{
}

void CFBStreamedAnimReader::VSetPhase(float)
{
}

SAdvancementResults CFBStreamedAnimReader::VReverseView(const CCharAnimTime& time)
{
}

std::shared_ptr<IAnimReader> CFBStreamedAnimReader::VClone() const
{
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const
{
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list,
                                                CSegStatementSet& setOut,
                                                const CCharAnimTime& time) const
{
}

SAdvancementResults CFBStreamedAnimReader::VAdvanceView(const CCharAnimTime& a)
{
}

CCharAnimTime CFBStreamedAnimReader::VGetTimeRemaining() const
{
}

CSteadyStateAnimInfo CFBStreamedAnimReader::VGetSteadyStateAnimInfo() const
{
}

bool CFBStreamedAnimReader::VHasOffset(const CSegId& seg) const
{
}

zeus::CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg) const
{
}

zeus::CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const
{
}

zeus::CQuaternion CFBStreamedAnimReader::VGetRotation(const CSegId& seg) const
{
}

template class TAnimSourceInfo<CFBStreamedCompression>;

}
