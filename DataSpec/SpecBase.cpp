#include "SpecBase.hpp"

namespace Retro
{

bool SpecBase::canExtract(HECL::Database::Project& project,
                          const ExtractPassInfo& info, std::vector<ExtractReport>& reps)
{
    bool isWii;
    m_disc = NOD::OpenDiscFromImage(info.srcpath.c_str(), isWii);
    if (!m_disc)
        return false;
    const char* gameID = m_disc->getHeader().gameID;

    bool standalone = true;
    if (isWii && !memcmp(gameID, "R3M", 3))
        standalone = false;

    if (standalone && !checkStandaloneID(gameID))
        return false;

    char region = m_disc->getHeader().gameID[3];
    static const HECL::SystemString regNONE = _S("");
    static const HECL::SystemString regE = _S("NTSC");
    static const HECL::SystemString regJ = _S("NTSC-J");
    static const HECL::SystemString regP = _S("PAL");
    const HECL::SystemString* regstr = &regNONE;
    switch (region)
    {
    case 'E':
        regstr = &regE;
        break;
    case 'J':
        regstr = &regJ;
        break;
    case 'P':
        regstr = &regP;
        break;
    }

    if (standalone)
        return checkFromStandaloneDisc(project, *m_disc.get(), *regstr, info.extractArgs, reps);
    else
        return checkFromTrilogyDisc(project, *m_disc.get(), *regstr, info.extractArgs, reps);
}

void SpecBase::doExtract(HECL::Database::Project& project, const ExtractPassInfo&)
{
    extractFromDisc(project, *m_disc.get());
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
