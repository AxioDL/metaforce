#ifndef __URDE_CSIMPLEPOOL_HPP__
#define __URDE_CSIMPLEPOOL_HPP__

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
    u8 x4_;
    u8 x5_;
    std::unordered_map<SObjectTag, CObjectReference*> x8_resources;
    IFactory& x18_factory;
    CVParamTransfer x1c_paramXfer;
public:
    CSimplePool(IFactory& factory);
    ~CSimplePool();
    CToken GetObj(const SObjectTag&, const CVParamTransfer&);
    CToken GetObj(const SObjectTag&);
    CToken GetObj(std::string_view);
    CToken GetObj(std::string_view, const CVParamTransfer&);
    bool HasObject(const SObjectTag&) const;
    bool ObjectIsLive(const SObjectTag&) const;
    IFactory& GetFactory() const {return x18_factory;}
    void Flush();
    void ObjectUnreferenced(const SObjectTag&);
    std::vector<SObjectTag> GetReferencedTags() const;
};

}

#endif // __URDE_CSIMPLEPOOL_HPP__
