#include "CFBStreamedAnimReader.hpp"
#include "CSegIdList.hpp"
#include "CSegStatementSet.hpp"

namespace urde
{

void CFBStreamedAnimReaderTotals::Allocate(u32 chanCount)
{
    u32 chan2 = chanCount * 2;
    u32 chan32 = chanCount * 32;

    size_t sz = chan32 + chanCount + chan2 + chan32;
    x0_buffer.reset(new u8[sz]);
    memset(x0_buffer.get(), 0, sz);
    x4_cumulativeInts32 = reinterpret_cast<s32*>(x0_buffer.get());
    x8_hasTrans1 = reinterpret_cast<u8*>(x4_cumulativeInts32 + chanCount * 8);
    xc_segIds2 = reinterpret_cast<u16*>(x8_hasTrans1 + chanCount);
    x10_computedFloats32 = reinterpret_cast<float*>(xc_segIds2 + chanCount);
}

void CFBStreamedAnimReaderTotals::Initialize(const CFBStreamedCompression& source)
{
    x1c_curKey = 0;
    x20_calculated = false;
    const u8* chans = source.GetPerChannelHeaders();
    u32 boneChanCount = *reinterpret_cast<const u32*>(chans);
    chans += 4;

    if (source.m_pc)
    {
        for (unsigned b=0 ; b<boneChanCount ; ++b)
        {
            xc_segIds2[b] = *reinterpret_cast<const u32*>(chans);
            chans += 8;

            s32* cumulativesOut = &x4_cumulativeInts32[8*b];
            const s32* cumulativesIn = reinterpret_cast<const s32*>(chans);
            cumulativesOut[0] = 0;
            cumulativesOut[1] = cumulativesIn[0] >> 8;
            cumulativesOut[2] = cumulativesIn[1] >> 8;
            cumulativesOut[3] = cumulativesIn[2] >> 8;
            chans += 12;

            u32 tCount = *reinterpret_cast<const u32*>(chans);
            chans += 4;
            if (tCount)
            {
                x8_hasTrans1[b] = true;
                const s32* cumulativesIn = reinterpret_cast<const s32*>(chans);
                cumulativesOut[4] = cumulativesIn[0] >> 8;
                cumulativesOut[5] = cumulativesIn[1] >> 8;
                cumulativesOut[6] = cumulativesIn[2] >> 8;
                chans += 12;
            }
            else
                x8_hasTrans1[b] = false;
        }
    }
    else
    {
        for (unsigned b=0 ; b<boneChanCount ; ++b)
        {
            xc_segIds2[b] = *reinterpret_cast<const u32*>(chans);
            chans += 6;

            s32* cumulativesOut = &x4_cumulativeInts32[8*b];
            cumulativesOut[0] = 0;
            cumulativesOut[1] = *reinterpret_cast<const s16*>(chans);
            cumulativesOut[2] = *reinterpret_cast<const s16*>(chans + 3);
            cumulativesOut[3] = *reinterpret_cast<const s16*>(chans + 6);
            chans += 9;

            u16 tCount = *reinterpret_cast<const u16*>(chans);
            chans += 2;
            if (tCount)
            {
                x8_hasTrans1[b] = true;
                cumulativesOut[4] = *reinterpret_cast<const s16*>(chans);
                cumulativesOut[5] = *reinterpret_cast<const s16*>(chans + 3);
                cumulativesOut[6] = *reinterpret_cast<const s16*>(chans + 6);
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
    dest.x20_calculated = false;

    const u8* chans = source.GetPerChannelHeaders();
    u32 boneChanCount = *reinterpret_cast<const u32*>(chans);
    chans += 4;

    if (source.m_pc)
    {
        for (unsigned b=0 ; b<boneChanCount ; ++b)
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
        for (unsigned b=0 ; b<boneChanCount ; ++b)
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

    dest.x1c_curKey = x1c_curKey + 1;
}

void CFBStreamedAnimReaderTotals::CalculateDown()
{
    for (unsigned b=0 ; b<x24_boneChanCount ; ++b)
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
    x20_calculated = true;
}

CFBStreamedPairOfTotals::CFBStreamedPairOfTotals(const TSubAnimTypeToken<CFBStreamedCompression>& source)
: x0_source(source), xc_rotsAndOffs(source->xc_rotsAndOffs.get()), x14_a(*source), x3c_b(*source)
{
}

void CFBStreamedPairOfTotals::SetTime(CBitLevelLoader& loader, const CCharAnimTime& time)
{
    /* Implementation is a bit different than original;
     * T evaluated pre-emptively with key indices.
     * CalculateDown is also called here as needed. */

    const CFBStreamedCompression::Header& header = x0_source->MainHeader();
    CCharAnimTime interval(header.interval);
    const u32* timeBitmap = x0_source->GetTimes();
    CCharAnimTime priorTime(0);
    CCharAnimTime curTime(0);

    int prior = -1;
    int next = -1;
    int cur = 0;
    for (unsigned b=0 ; b<timeBitmap[0] ; ++b)
    {
        int word = b / 32;
        int bit = b % 32;
        if ((timeBitmap[word+1] >> bit) & 1)
        {
            if (curTime <= time)
            {
                prior = cur;
                priorTime = curTime;
            }
            else if (curTime > time)
            {
                next = cur;
                if (prior == -1)
                {
                    prior = cur;
                    priorTime = curTime;
                    x78_t = 0.f;
                }
                else
                    x78_t = (time - priorTime) / (curTime - priorTime);

                break;
            }
            ++cur;
        }
        curTime += interval;
    }

    if (prior != -1 && u32(prior) < Prior().x1c_curKey)
    {
        Prior().Initialize(*x0_source);
        Next().Initialize(*x0_source);
        loader.Reset();
    }

    if (next != -1)
        while (u32(next) > Next().x1c_curKey)
            DoIncrement(loader);

    if (!Prior().IsCalculated())
        Prior().CalculateDown();
    if (!Next().IsCalculated())
        Next().CalculateDown();
}

void CFBStreamedPairOfTotals::DoIncrement(CBitLevelLoader& loader)
{
    x10_nextSel ^= 1;
    Prior().IncrementInto(loader, *x0_source, Next());
}

u32 CBitLevelLoader::LoadUnsigned(u8 q)
{
    u32 byteCur = (m_bitIdx / 32) * 4;
    u32 bitRem = m_bitIdx % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    u32 tempBuf = *reinterpret_cast<const u32*>(m_data + byteCur) >> bitRem;

    /* If this shift underflows the value, buffer the next 32 bits */
    /* And tack onto shifted buffer */
    if ((bitRem + q) > 32)
    {
        u32 tempBuf2 = *reinterpret_cast<const u32*>(m_data + byteCur + 4);
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
    u32 tempBuf = *reinterpret_cast<const u32*>(m_data + byteCur) >> bitRem;

    /* If this shift underflows the value, buffer the next 32 bits */
    /* And tack onto shifted buffer */
    if ((bitRem + q) > 32)
    {
        u32 tempBuf2 = *reinterpret_cast<const u32*>(m_data + byteCur + 4);
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
    u32 tempBuf = *reinterpret_cast<const u32*>(m_data + byteCur) >> bitRem;

    /* That's it */
    m_bitIdx += 1;
    return tempBuf & 0x1;
}

CSegIdToIndexConverter::CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals)
{
    std::fill(std::begin(x0_indices), std::end(x0_indices), -1);
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
  x7c_totals(source), x104_bitstreamData(source->GetBitstreamPointer()), x108_bitLoader(x104_bitstreamData),
  x114_segIdToIndex(x7c_totals.x10_nextSel ? x7c_totals.x14_a : x7c_totals.x3c_b)
{
    x7c_totals.SetTime(x108_bitLoader, CCharAnimTime());
    PostConstruct(time);
}

bool CFBStreamedAnimReader::HasOffset(const CSegId& seg) const
{
    s32 idx = x114_segIdToIndex.SegIdToIndex(seg);
    if (idx == -1)
        return false;
    return x7c_totals.Prior().x8_hasTrans1[idx];
}

zeus::CVector3f CFBStreamedAnimReader::GetOffset(const CSegId& seg) const
{
    s32 idx = x114_segIdToIndex.SegIdToIndex(seg);
    if (idx == -1)
        return {};
    const float* af = x7c_totals.Prior().GetFloats(idx);
    const float* bf = x7c_totals.Next().GetFloats(idx);
    zeus::CVector3f a(af[4], af[5], af[6]);
    zeus::CVector3f b(bf[4], bf[5], bf[6]);
    return zeus::CVector3f::lerp(a, b, x7c_totals.GetT());
}

zeus::CQuaternion CFBStreamedAnimReader::GetRotation(const CSegId& seg) const
{
    s32 idx = x114_segIdToIndex.SegIdToIndex(seg);
    if (idx == -1)
        return {};
    const float* af = x7c_totals.Prior().GetFloats(idx);
    const float* bf = x7c_totals.Next().GetFloats(idx);
    zeus::CQuaternion a(af[0], af[1], af[2], af[3]);
    zeus::CQuaternion b(bf[0], bf[1], bf[2], bf[3]);
    return zeus::CQuaternion::slerp(a, b, x7c_totals.GetT());
}

SAdvancementResults CFBStreamedAnimReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                                                  const CCharAnimTime& startOff) const
{
    SAdvancementResults res = {};

    CCharAnimTime resolveTime = xc_curTime + startOff;
    CCharAnimTime animDur = x54_source->GetAnimationDuration();
    if (resolveTime >= animDur || dt.EqualsZero())
        return res;

    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, resolveTime);
    zeus::CQuaternion priorQ = GetRotation(3);
    zeus::CVector3f priorV = GetOffset(3);

    CCharAnimTime nextTime = resolveTime + dt;
    if (nextTime > animDur)
    {
        nextTime = animDur;
        res.x0_remTime = nextTime - animDur;
    }

    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, nextTime);
    zeus::CQuaternion nextQ = GetRotation(3);
    zeus::CVector3f nextV = GetOffset(3);

    res.x8_deltas.xc_rotDelta = priorQ.inverse() * nextQ;
    if (HasOffset(3))
        res.x8_deltas.x0_posDelta = res.x8_deltas.xc_rotDelta.transform(nextV - priorV);

    return res;
}

void CFBStreamedAnimReader::VSetPhase(float ph)
{
    xc_curTime = x64_steadyStateInfo.GetDuration() * ph;
    x7c_totals.SetTime(x108_bitLoader, xc_curTime);
    if (x54_source->HasPOIData())
    {
        UpdatePOIStates();
        if (!xc_curTime.GreaterThanZero())
        {
            x14_passedBoolCount = 0;
            x18_passedIntCount = 0;
            x1c_passedParticleCount = 0;
            x20_passedSoundCount = 0;
        }
    }
}

SAdvancementResults CFBStreamedAnimReader::VReverseView(const CCharAnimTime& time)
{
    return {};
}

std::unique_ptr<IAnimReader> CFBStreamedAnimReader::VClone() const
{
    return std::make_unique<CFBStreamedAnimReader>(x54_source, xc_curTime);
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const
{
    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, xc_curTime);

    for (const CSegId& id : list.GetList())
    {
        CAnimPerSegmentData& out = setOut[id];
        out.x0_rotation = GetRotation(id);
        out.x1c_hasOffset = HasOffset(id);
        if (out.x1c_hasOffset)
            out.x10_offset = GetOffset(id);
    }
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list,
                                                CSegStatementSet& setOut,
                                                const CCharAnimTime& time) const
{
    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, time);

    for (const CSegId& id : list.GetList())
    {
        CAnimPerSegmentData& out = setOut[id];
        out.x0_rotation = GetRotation(id);
        out.x1c_hasOffset = HasOffset(id);
        if (out.x1c_hasOffset)
            out.x10_offset = GetOffset(id);
    }
}

SAdvancementResults CFBStreamedAnimReader::VAdvanceView(const CCharAnimTime& dt)
{
    SAdvancementResults res = {};

    CCharAnimTime animDur = x54_source->GetAnimationDuration();
    if (xc_curTime == animDur)
    {
        xc_curTime = CCharAnimTime();
        x7c_totals.SetTime(x108_bitLoader, xc_curTime);
        res.x0_remTime = dt;
        return res;
    }
    else if (dt.EqualsZero())
    {
        return res;
    }

    zeus::CQuaternion priorQ = GetRotation(3);
    zeus::CVector3f priorV = GetOffset(3);

    xc_curTime += dt;
    CCharAnimTime overTime;
    if (xc_curTime > animDur)
    {
        overTime = xc_curTime - animDur;
        xc_curTime = animDur;
    }

    x7c_totals.SetTime(x108_bitLoader, xc_curTime);
    if (x54_source->HasPOIData())
        UpdatePOIStates();

    zeus::CQuaternion nextQ = GetRotation(3);
    zeus::CVector3f nextV = GetOffset(3);

    res.x0_remTime = overTime;
    res.x8_deltas.xc_rotDelta = nextQ * priorQ.inverse();
    if (HasOffset(3))
        res.x8_deltas.x0_posDelta = nextQ.inverse().transform(nextV - priorV);

    return res;
}

CCharAnimTime CFBStreamedAnimReader::VGetTimeRemaining() const
{
    return x54_source->GetAnimationDuration() - xc_curTime;
}

CSteadyStateAnimInfo CFBStreamedAnimReader::VGetSteadyStateAnimInfo() const
{
    return x64_steadyStateInfo;
}

bool CFBStreamedAnimReader::VHasOffset(const CSegId& seg) const
{
    return HasOffset(seg);
}

zeus::CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg) const
{
    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, xc_curTime);
    return GetOffset(seg);
}

zeus::CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const
{
    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, time);
    return GetOffset(seg);
}

zeus::CQuaternion CFBStreamedAnimReader::VGetRotation(const CSegId& seg) const
{
    const_cast<CFBStreamedAnimReader*>(this)->x7c_totals.SetTime
        (const_cast<CFBStreamedAnimReader*>(this)->x108_bitLoader, xc_curTime);
    return GetRotation(seg);
}

template class TAnimSourceInfo<CFBStreamedCompression>;

}
