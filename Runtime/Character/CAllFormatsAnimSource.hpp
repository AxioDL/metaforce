#ifndef __URDE_CALLFORMATANIMSOURCE_HPP__
#define __URDE_CALLFORMATANIMSOURCE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "CAnimSource.hpp"
#include "CFactoryMgr.hpp"

namespace urde
{
class IObjectStore;
class CAnimSourceReaderBase;

enum class EAnimFormat
{
    Uncompressed,
    Unknown,
    BitstreamCompressed,
    BitstreamCompressed24
};

class CAnimFormatUnion
{
    union
    {
        EAnimFormat x0_format;
        intptr_t _align = 0;
    };
    u8 x4_storage[sizeof(CAnimSource)];
    static void SubConstruct(u8* storage, EAnimFormat fmt,
                             CInputStream& in, IObjectStore& store);
public:
    CAnimFormatUnion(CInputStream& in, IObjectStore& store);
    ~CAnimFormatUnion();
    operator CAnimSource&() {return *reinterpret_cast<CAnimSource*>(x4_storage);}
};

class CAllFormatsAnimSource : public CAnimFormatUnion
{
    zeus::CVector3f x68_;
    SObjectTag x74_tag;
public:
    CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
    static std::shared_ptr<CAnimSourceReaderBase> GetNewReader(const TLockedToken<CAllFormatsAnimSource>& tok,
                                                               const CCharAnimTime& startTime);
};

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params);

}

#endif // __URDE_CALLFORMATANIMSOURCE_HPP__
