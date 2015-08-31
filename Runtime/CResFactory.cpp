#include "CResFactory.hpp"
#include "IObj.hpp"

namespace Retro
{

std::unique_ptr<IObj> CResFactory::Build(const SObjectTag&, const CVParamTransfer&)
{
    return std::unique_ptr<IObj>();
}
void CResFactory::BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**)
{
}
void CResFactory::CancelBuild(const SObjectTag&)
{
}

}
