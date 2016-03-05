#ifndef __PSHAG_IFACTORY_HPP__
#define __PSHAG_IFACTORY_HPP__

#include <memory>
#include "RetroTypes.hpp"

namespace urde
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
    virtual const SObjectTag* GetResourceIdByName(const char*) const=0;
};

}

#endif // __PSHAG_IFACTORY_HPP__
