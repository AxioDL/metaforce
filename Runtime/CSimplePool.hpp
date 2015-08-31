#ifndef __RETRO_CSIMPLEPOOL_HPP__
#define __RETRO_CSIMPLEPOOL_HPP__

#include "IObjectStore.hpp"

namespace Retro
{
class IFactory;

class CSimplePool : public IObjectStore
{
    IFactory& m_factory;
public:
    CSimplePool(IFactory& factory)
    : m_factory(factory)
    {
    }
    IObj* GetObj(const SObjectTag&, const CVParamTransfer&) {return nullptr;}
    IObj* GetObj(const SObjectTag&) {return nullptr;}
    IObj* GetObj(char const*) {return nullptr;}
    IObj* GetObj(char const*, const CVParamTransfer&) {return nullptr;}
    void HasObject(const SObjectTag&) const {}
    void ObjectIsLive(const SObjectTag&) const {}
    IFactory& GetFactory() const {return m_factory;}
    void Flush() {}
    void ObjectUnreferenced(const SObjectTag&) {}
};

}

#endif // __RETRO_CSIMPLEPOOL_HPP__
