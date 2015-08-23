#ifndef __RETRO_IFACTORY_HPP__
#define __RETRO_IFACTORY_HPP__

#include <memory>
#include "RetroTypes.hpp"

namespace Retro
{
class CVParamTransfer;
class IObj;

class IFactory
{
public:
    virtual ~IFactory() {}
    virtual std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&)=0;
    virtual void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**)=0;
    virtual void CancelBuild(const SObjectTag&)=0;
    virtual bool CanBuild(const SObjectTag&)=0;
    virtual u32 GetResourceIdByName(const char*) const=0;
};

}

#endif // __RETRO_IFACTORY_HPP__
