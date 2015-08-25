#ifndef __RETRO_CSIMPLEPOOL_HPP__
#define __RETRO_CSIMPLEPOOL_HPP__

#include "IObjectStore.hpp"

namespace Retro
{
class IFactory;

class CSimplePool : public IObjectStore
{
public:
    CSimplePool(IFactory&)
    {
    }
    IObj& GetObj(const SObjectTag&, const CVParamTransfer&) {}
    IObj& GetObj(const SObjectTag&) {}
    IObj& GetObj(char const*) {}
    IObj& GetObj(char const*, const CVParamTransfer&) {}
    void HasObject(const SObjectTag&) const {}
    void ObjectIsLive(const SObjectTag&) const {}
    IFactory& GetFactory() const {}
    void Flush() {}
    void ObjectUnreferenced(const SObjectTag&) {}
};

}

#endif // __RETRO_CSIMPLEPOOL_HPP__
