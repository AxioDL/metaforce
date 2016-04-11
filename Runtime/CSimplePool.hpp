#ifndef __PSHAG_CSIMPLEPOOL_HPP__
#define __PSHAG_CSIMPLEPOOL_HPP__

#include "IObjectStore.hpp"
#include "RetroTypes.hpp"
#include "IVParamObj.hpp"
#include <unordered_map>

namespace urde
{
class IFactory;
class CObjectReference;

class CSimplePool : public IObjectStore
{
protected:
    std::list<std::pair<SObjectTag, CObjectReference*>> x4_resources;
    //std::unordered_map<SObjectTag, CObjectReference*> x4_resources;
    IFactory& x30_factory;
    CVParamTransfer x34_paramXfer;
public:
    CSimplePool(IFactory& factory);
    CToken GetObj(const SObjectTag&, const CVParamTransfer&);
    CToken GetObj(const SObjectTag&);
    CToken GetObj(const char*);
    CToken GetObj(const char*, const CVParamTransfer&);
    bool HasObject(const SObjectTag&) const;
    bool ObjectIsLive(const SObjectTag&) const;
    IFactory& GetFactory() const {return x30_factory;}
    void Flush();
    void ObjectUnreferenced(const SObjectTag&);
};

}

#endif // __PSHAG_CSIMPLEPOOL_HPP__
