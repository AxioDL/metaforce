#include "Runtime/Character/CAllFormatsAnimSource.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Character/CAnimSourceReader.hpp"
#include "Runtime/Character/CFBStreamedAnimReader.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
static logvisor::Module Log("metaforce::CAllFormatsAnimSource");

void CAnimFormatUnion::SubConstruct(u8* storage, EAnimFormat fmt, CInputStream& in, IObjectStore& store) {
  switch (fmt) {
  case EAnimFormat::Uncompressed:
    new (storage) CAnimSource(in, store);
    break;
  case EAnimFormat::BitstreamCompressed:
    new (storage) CFBStreamedCompression(in, store, false);
    break;
  case EAnimFormat::BitstreamCompressed24:
    new (storage) CFBStreamedCompression(in, store, true);
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("unable to read ANIM format {}"), int(fmt));
  }
}

CAnimFormatUnion::CAnimFormatUnion(CInputStream& in, IObjectStore& store) {
  x0_format = EAnimFormat(in.ReadLong());
  SubConstruct(x4_storage, x0_format, in, store);
}

CAnimFormatUnion::~CAnimFormatUnion() {
  switch (x0_format) {
  case EAnimFormat::Uncompressed:
    reinterpret_cast<CAnimSource*>(x4_storage)->~CAnimSource();
    break;
  case EAnimFormat::BitstreamCompressed:
  case EAnimFormat::BitstreamCompressed24:
    reinterpret_cast<CFBStreamedCompression*>(x4_storage)->~CFBStreamedCompression();
    break;
  default:
    break;
  }
}

std::shared_ptr<IAnimReader> CAllFormatsAnimSource::GetNewReader(const TLockedToken<CAllFormatsAnimSource>& tok,
                                                                 const CCharAnimTime& startTime) {
  switch (tok->x0_format) {
  case EAnimFormat::Uncompressed:
    return std::make_shared<CAnimSourceReader>(tok, startTime);
  case EAnimFormat::BitstreamCompressed:
  case EAnimFormat::BitstreamCompressed24:
    return std::make_shared<CFBStreamedAnimReader>(tok, startTime);
  default:
    break;
  }
  return {};
}

CAllFormatsAnimSource::CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag)
: CAnimFormatUnion(in, store), x74_tag(tag) {}

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef) {
  CSimplePool* sp = params.GetOwnedObj<CSimplePool*>();
  return TToken<CAllFormatsAnimSource>::GetIObjObjectFor(std::make_unique<CAllFormatsAnimSource>(in, *sp, tag));
}

} // namespace metaforce
