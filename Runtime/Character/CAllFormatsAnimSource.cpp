#include "CAllFormatsAnimSource.hpp"
#include "logvisor/logvisor.hpp"

namespace urde
{
static logvisor::Module Log("urde::CAllFormatsAnimSource");

void CAnimFormatUnion::SubConstruct(u8* storage, EAnimFormat fmt,
                                    CInputStream& in, IObjectStore& store)
{
    switch (fmt)
    {
    case EAnimFormat::Uncompressed:
        new (storage) CAnimSource(in, store);
        break;
    default:
        Log.report(logvisor::Fatal, "unable to read ANIM format %d", int(fmt));
    }
}

CAnimFormatUnion::CAnimFormatUnion(CInputStream& in, IObjectStore& store)
{
    x0_format = EAnimFormat(in.readUint32Big());
    SubConstruct(x4_storage, x0_format, in, store);
}

CAllFormatsAnimSource::CAllFormatsAnimSource(CInputStream& in,
                                             IObjectStore& store,
                                             const SObjectTag& tag)
: CAnimFormatUnion(in, store), x74_tag(tag) {}

}
