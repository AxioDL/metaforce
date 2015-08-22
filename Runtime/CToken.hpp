#ifndef __RETRO_CTOKEN_HPP__
#define __RETRO_CTOKEN_HPP__

#include <memory>
#include "RetroTypes.hpp"
#include "CVParamTransfer.hpp"
#include "IObjectStore.hpp"

namespace Retro
{
class IObjectStore;
class IObj;

class CObjectReference
{
    u16 x0_refCount = 0;
    bool x3_loading = false;
    SObjectTag x4_objTag;
    IObjectStore* xC_objectStore = nullptr;
    std::unique_ptr<IObj> x10_object;
    CVParamTransfer x14_params;
public:
    CObjectReference(IObjectStore& objStore, std::unique_ptr<IObj>&& obj,
                     const SObjectTag& objTag, CVParamTransfer buildParams)
        : x4_objTag(objTag), xC_objectStore(&objStore),
          x10_object(std::move(obj)), x14_params(buildParams) {}
    CObjectReference(std::unique_ptr<IObj>&& obj);

    bool IsLoading() const {return x3_loading;}
    void Unlock() {}
    void RemoveReference() {}
    void CancelLoad() {}
    void Unload() {}
    void GetObject() {}

};

class CToken
{
    CObjectReference* x0_objRef;
    bool x4_lockHeld = false;
public:
    ~CToken()
    {
        if (x4_lockHeld)

    }
};

template<class T>
class TLockedToken
{
};

}

#endif // __RETRO_CTOKEN_HPP__
