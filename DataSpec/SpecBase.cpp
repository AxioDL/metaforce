#include "SpecBase.hpp"

namespace Retro
{

bool SpecBase::canExtract(const ExtractPassInfo& info, HECL::SystemString& reasonNo)
{
    bool isWii;
    std::unique_ptr<NOD::DiscBase> disc = NOD::OpenDiscFromImage(info.srcpath.c_str(), isWii);
    if (!disc)
    {
        reasonNo = _S("Not a valid Nintendo disc image");
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

    HECL::SystemStringView gameIDView(std::string(gameID, 6));
    HECL::SystemStringView gameNameView(disc->getHeader().gameTitle);
    reasonNo = gameIDView.sys_str() + _S(" (") + gameNameView.sys_str() + _S(") is not supported");
    return false;
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
