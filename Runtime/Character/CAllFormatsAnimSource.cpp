#include "CAllFormatsAnimSource.hpp"
#include "logvisor/logvisor.hpp"
#include "CSimplePool.hpp"
#include "CAnimSourceReader.hpp"

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

CAnimFormatUnion::~CAnimFormatUnion()
{
    switch (x0_format)
    {
    case EAnimFormat::Uncompressed:
        reinterpret_cast<CAnimSource*>(x4_storage)->~CAnimSource();
    default: break;
    }
}

std::shared_ptr<CAnimSourceReaderBase>
CAllFormatsAnimSource::GetNewReader(const TLockedToken<CAllFormatsAnimSource>& tok,
                                    const CCharAnimTime& startTime)
{
    return std::make_shared<CAnimSourceReader>(tok, startTime);
}

CAllFormatsAnimSource::CAllFormatsAnimSource(CInputStream& in,
                                             IObjectStore& store,
                                             const SObjectTag& tag)
: CAnimFormatUnion(in, store), x74_tag(tag) {}

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& params)
{
    CSimplePool* sp = static_cast<CSimplePool*>(
        static_cast<TObjOwnerParam<IObjectStore*>*>(
            params.GetObj())->GetParam());
    return TToken<CAllFormatsAnimSource>::GetIObjObjectFor(
        std::make_unique<CAllFormatsAnimSource>(in, *sp, tag));
}

}
