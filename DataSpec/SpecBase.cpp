#include "SpecBase.hpp"

namespace Retro
{

bool SpecBase::canExtract(const HECL::Database::Project& project, const ExtractPassInfo& info,
                          HECL::SystemString& reasonNo)
{

}

void SpecBase::doExtract(const HECL::Database::Project& project, const ExtractPassInfo& info)
{
}

bool SpecBase::canCook(const HECL::Database::Project& project, const CookTaskInfo& info,
                       HECL::SystemString& reasonNo)
{
}

void SpecBase::doCook(const HECL::Database::Project& project, const CookTaskInfo& info)
{
}

bool SpecBase::canPackage(const HECL::Database::Project& project, const PackagePassInfo& info,
                          HECL::SystemString& reasonNo)
{
}

void SpecBase::gatherDependencies(const HECL::Database::Project& project, const PackagePassInfo& info,
                                  std::unordered_set<HECL::ProjectPath>& implicitsOut)
{
}

void SpecBase::doPackage(const HECL::Database::Project& project, const PackagePassInfo& info)
{
}

}
