#ifndef __URDE_IFACTORY_HPP__
#define __URDE_IFACTORY_HPP__

#include <memory>
#include "RetroTypes.hpp"

namespace urde
{
class CVParamTransfer;
class IObj;

using CFactoryFnReturn = std::unique_ptr<IObj>;
using FFactoryFunc = std::function<CFactoryFnReturn(const urde::SObjectTag& tag,
                                                    urde::CInputStream& in,
                                                    const urde::CVParamTransfer& vparms)>;
using FMemFactoryFunc = std::function<CFactoryFnReturn(const urde::SObjectTag& tag,
                                                       std::unique_ptr<u8[]>&& in, u32 len,
                                                       const urde::CVParamTransfer& vparms)>;

class IFactory
{
public:
    virtual ~IFactory() = default;
    virtual CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&)=0;
    virtual void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**)=0;
    virtual void CancelBuild(const SObjectTag&)=0;
    virtual bool CanBuild(const SObjectTag&)=0;
    virtual const SObjectTag* GetResourceIdByName(const char*) const=0;
};

}

#endif // __URDE_IFACTORY_HPP__
