#include "CFBStreamedAnimReader.hpp"

namespace urde
{

CFBStreamedAnimReaderTotals::CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source)
{
    const CFBStreamedCompression::Header* header =
        reinterpret_cast<const CFBStreamedCompression::Header*>(source.xc_rotsAndOffs.get());
    const u32* bitmap = reinterpret_cast<const u32*>(source.xc_rotsAndOffs.get() + 9);
    u32 bitmapWordCount = (bitmap[0] + 31) / 32;

    x14_rotDiv = header->rotDiv;
    x18_transMult = header->translationMult;

    const u8* chans = reinterpret_cast<const u8*>(bitmap + bitmapWordCount + 1);
    u32 boneChanCount = *reinterpret_cast<const u32*>(chans);
    x24_boneChanCount = boneChanCount;
}

CFBStreamedPairOfTotals::CFBStreamedPairOfTotals(const TSubAnimTypeToken<CFBStreamedCompression>& source)
: x0_source(source), xc_rotsAndOffs(source->xc_rotsAndOffs.get()), x14_(*source), x3c_(*source)
{
    x0_source.Lock();

    const u32* bitmap = reinterpret_cast<const u32*>(source->xc_rotsAndOffs.get() + 9);
    u32 bitmapWordCount = (bitmap[0] + 31) / 32;

    const u8* chans = reinterpret_cast<const u8*>(bitmap + bitmapWordCount + 1);
    u32 boneChanCount = *reinterpret_cast<const u32*>(chans);
    chans += 4;

    for (int b=0 ; b<boneChanCount ; ++b)
    {
        chans += 15;

        u16 tCount = *reinterpret_cast<const u16*>(chans);
        chans += 2;

        if (tCount)
            chans += 9;
    }

    x88_ = chans;
    x8c_ = &x88_;
    x90_ = *reinterpret_cast<const u32*>(*x8c_);
}

CSegIdToIndexConverter::CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals)
{
}

CFBStreamedAnimReader::CFBStreamedAnimReader(const TSubAnimTypeToken<CFBStreamedCompression>& source, const CCharAnimTime& time)
: CAnimSourceReaderBase(std::make_unique<TAnimSourceInfo<CFBStreamedCompression>>(source), time),
  x54_source(source), x7c_(source), x114_(x7c_.x10_ ? x7c_.x14_ : x7c_.x3c_)
{
    x54_source.Lock();
    x64_steadyStateInfo.x64_duration = x54_source->GetAnimationDuration();
    x64_steadyStateInfo.x6c_curRootOffset = x54_source->x14_rootOffset;

    const CFBStreamedCompression::Header* header =
        reinterpret_cast<const CFBStreamedCompression::Header*>(x54_source->xc_rotsAndOffs.get());
    x64_steadyStateInfo.x78_ = header->unk2;

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
