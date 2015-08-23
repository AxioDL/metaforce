#include "CResFactory.hpp"

namespace Retro
{

std::unique_ptr<IObj> CResFactory::Build(const SObjectTag&, const CVParamTransfer&)
{
}
void CResFactory::BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**)
{
}
void CResFactory::CancelBuild(const SObjectTag&)
{
}

}
