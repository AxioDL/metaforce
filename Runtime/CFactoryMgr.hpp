#ifndef __PSHAG_CFACTORYMGR_HPP__
#define __PSHAG_CFACTORYMGR_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"
#include "IOStreams.hpp"

namespace pshag
{
class SObjectTag;
class CVParamTransfer;
class IObj;

using CFactoryFnReturn = std::unique_ptr<IObj>;
using FFactoryFunc = std::function<CFactoryFnReturn(const pshag::SObjectTag& tag,
                                                    pshag::CInputStream& in,
                                                    const pshag::CVParamTransfer& vparms)>;
class CFactoryMgr
{
    std::unordered_map<FourCC, FFactoryFunc> m_factories;
public:
    CFactoryFnReturn MakeObject(const SObjectTag& tag, pshag::CInputStream& in,
                                const CVParamTransfer& paramXfer);
    CFactoryFnReturn MakeObjectFromMemory(const SObjectTag& tag, void* buf, int size, bool compressed,
                                          const CVParamTransfer& paramXfer);
    void AddFactory(FourCC key, FFactoryFunc func)
    {
        m_factories[key] = func;
    }
};

}

#endif // __PSHAG_CFACTORYMGR_HPP__
