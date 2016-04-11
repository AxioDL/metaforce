#include "CAnimSourceReader.hpp"

namespace urde
{

CAnimSourceReaderBase::CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo,
                                             const CCharAnimTime& time)
: x4_sourceInfo(std::move(sourceInfo)), xc_curTime(time) {}

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken<CAnimSource>& source,
                                     const CCharAnimTime& time)
: CAnimSourceReaderBase(std::make_unique<CAnimSourceInfo>(source), CCharAnimTime()),
  x54_source(source)
{
    CAnimSource* sourceData = x54_source.GetObj();
}

}
