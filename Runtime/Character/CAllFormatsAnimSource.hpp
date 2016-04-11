#ifndef __PSHAG_CALLFORMATANIMSOURCE_HPP__
#define __PSHAG_CALLFORMATANIMSOURCE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "CAnimSource.hpp"

namespace urde
{
class IObjectStore;

enum class EAnimFormat
{
    Uncompressed,
    Unknown,
    BitstreamCompressed
};

class CAnimFormatUnion
{
    EAnimFormat x0_format;
    u8 x4_storage[sizeof(CAnimSource)];
    static void SubConstruct(u8* storage, EAnimFormat fmt,
                             CInputStream& in, IObjectStore& store);
public:
    CAnimFormatUnion(CInputStream& in, IObjectStore& store);
    operator CAnimSource&() {return *reinterpret_cast<CAnimSource*>(x4_storage);}
};

class CAllFormatsAnimSource : public CAnimFormatUnion
{
    zeus::CVector3f x68_;
    SObjectTag x74_tag;
public:
    CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
};

}

#endif // __PSHAG_CALLFORMATANIMSOURCE_HPP__
