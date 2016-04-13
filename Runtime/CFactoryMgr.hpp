#ifndef __URDE_CFACTORYMGR_HPP__
#define __URDE_CFACTORYMGR_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"
#include "IOStreams.hpp"
#include "IFactory.hpp"

namespace urde
{
struct SObjectTag;
class CVParamTransfer;
class IObj;

class CFactoryMgr
{
    std::unordered_map<FourCC, FFactoryFunc> m_factories;
    std::unordered_map<FourCC, FMemFactoryFunc> m_memFactories;
public:
    CFactoryFnReturn MakeObject(const SObjectTag& tag, urde::CInputStream& in,
                                const CVParamTransfer& paramXfer);
    bool CanMakeMemory(const urde::SObjectTag& tag) const;
    CFactoryFnReturn MakeObjectFromMemory(const SObjectTag& tag,
                                          std::unique_ptr<u8[]>&& buf,
                                          int size, bool compressed,
                                          const CVParamTransfer& paramXfer);
    void AddFactory(FourCC key, FFactoryFunc func) {m_factories[key] = func;}
    void AddFactory(FourCC key, FMemFactoryFunc func) {m_memFactories[key] = func;}
};

}

#endif // __URDE_CFACTORYMGR_HPP__
