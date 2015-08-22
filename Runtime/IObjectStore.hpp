#ifndef __RETRO_IOBJECTSTORE_HPP__
#define __RETRO_IOBJECTSTORE_HPP__

namespace Retro
{

class IObjectStore
{
public:
    /*
    GetObj((SObjectTag const &,CVParamTransfer const &))
    GetObj((SObjectTag const &))
    GetObj((char const *))
    GetObj((char const *,CVParamTransfer const &))
    HasObject(const(SObjectTag const &))
    .data6:80352C6C                 .long CSimplePool::ObjectIsLive(const(SObjectTag const &))
    .data6:80352C70                 .long CSimplePool::GetFactory(const(void))
    .data6:80352C74                 .long CSimplePool::Flush((void))
    .data6:80352C78                 .long CSimplePool::ObjectUnreferenced((SObjectTag const &))
    */
};

}

#endif // __RETRO_IOBJECTSTORE_HPP__
