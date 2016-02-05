#ifndef __RETRO_CSIMPLEPOOL_HPP__
#define __RETRO_CSIMPLEPOOL_HPP__

#include "IObjectStore.hpp"
#include "RetroTypes.hpp"
#include "IVParamObj.hpp"
#include <unordered_map>

namespace Retro
{
class IFactory;
class CObjectReference;

class CSimplePool : public IObjectStore
{
    std::unordered_map<SObjectTag, CObjectReference*> x4_resources;
    IFactory& x30_factory;
    CVParamTransfer x34_paramXfer;
public:
    CSimplePool(IFactory& factory);
    CToken GetObj(const SObjectTag&, const CVParamTransfer&);
    CToken GetObj(const SObjectTag&);
    CToken GetObj(char const*);
    CToken GetObj(char const*, const CVParamTransfer&);
    void HasObject(const SObjectTag&) const;
    void ObjectIsLive(const SObjectTag&) const;
    IFactory& GetFactory() const {return x30_factory;}
    void Flush();
    void ObjectUnreferenced(const SObjectTag&);
};

}

#endif // __RETRO_CSIMPLEPOOL_HPP__
