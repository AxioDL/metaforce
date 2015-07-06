#include "SpecBase.hpp"

namespace Retro
{

LogVisor::LogModule LogModule("RetroDataSpec");

bool SpecBase::canExtract(const ExtractPassInfo& info)
{
    bool isWii;
    std::unique_ptr<NOD::DiscBase> disc = NOD::OpenDiscFromImage(info.srcpath.c_str(), isWii);
    if (!disc)
    {
        LogModule.report(LogVisor::Error, _S("'%s' not a valid Nintendo disc image"), info.srcpath.c_str());
        return false;
    }
    const char* gameID = disc->getHeader().gameID;

    if (isWii)
    {
        if (!memcmp(gameID, "R3ME01", 6))
            return true;
        if (!memcmp(gameID, "R3MP01", 6))
            return true;
        if (!memcmp(gameID, "R3IJ01", 6))
            return true;
    }
    else
    {
        if (!memcmp(gameID, "GM8E01", 6))
            return true;
    }

    LogModule.report(LogVisor::Error, "%.6s (%s) is not supported", gameID, disc->getHeader().gameTitle);
    return false;
}

void SpecBase::doExtract(const HECL::Database::Project& project, const ExtractPassInfo& info)
{
}

bool SpecBase::canCook(const HECL::Database::Project& project, const CookTaskInfo& info)
{
}

void SpecBase::doCook(const HECL::Database::Project& project, const CookTaskInfo& info)
{
}

bool SpecBase::canPackage(const HECL::Database::Project& project, const PackagePassInfo& info)
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
