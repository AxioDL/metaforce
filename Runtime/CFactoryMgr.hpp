#ifndef __PSHAG_CFACTORYMGR_HPP__
#define __PSHAG_CFACTORYMGR_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"
#include "IOStreams.hpp"

namespace urde
{
struct SObjectTag;
class CVParamTransfer;
class IObj;

using CFactoryFnReturn = std::unique_ptr<IObj>;
using FFactoryFunc = std::function<CFactoryFnReturn(const urde::SObjectTag& tag,
                                                    urde::CInputStream& in,
                                                    const urde::CVParamTransfer& vparms)>;
using FMemFactoryFunc = std::function<CFactoryFnReturn(const urde::SObjectTag& tag,
                                                       std::unique_ptr<u8[]>&& in, u32 len,
                                                       const urde::CVParamTransfer& vparms)>;
class CFactoryMgr
{
    std::unordered_map<FourCC, FFactoryFunc> m_factories;
    std::unordered_map<FourCC, FMemFactoryFunc> m_memFactories;
public:
    CFactoryFnReturn MakeObject(const SObjectTag& tag, urde::CInputStream& in,
                                const CVParamTransfer& paramXfer);
    bool CanMakeMemory(const urde::SObjectTag& tag) const;
    CFactoryFnReturn MakeObjectFromMemory(const SObjectTag& tag, std::unique_ptr<u8[]>&& buf, int size, bool compressed,
                                          const CVParamTransfer& paramXfer);
    void AddFactory(FourCC key, FFactoryFunc func)
    {
        m_factories[key] = func;
    }
    void AddFactory(FourCC key, FMemFactoryFunc func)
    {
        m_memFactories[key] = func;
    }
};

}

#endif // __PSHAG_CFACTORYMGR_HPP__
