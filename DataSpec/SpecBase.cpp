#include "SpecBase.hpp"

namespace Retro
{

bool SpecBase::canExtract(HECL::Database::Project& project,
                          const ExtractPassInfo& info, std::vector<ExtractReport>& reps)
{
    m_disc = NOD::OpenDiscFromImage(info.srcpath.c_str(), m_isWii);
    if (!m_disc)
        return false;
    const char* gameID = m_disc->getHeader().gameID;

    m_standalone = true;
    if (m_isWii && !memcmp(gameID, "R3M", 3))
        m_standalone = false;

    if (m_standalone && !checkStandaloneID(gameID))
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

    if (m_standalone)
        return checkFromStandaloneDisc(project, *m_disc.get(), *regstr, info.extractArgs, reps);
    else
        return checkFromTrilogyDisc(project, *m_disc.get(), *regstr, info.extractArgs, reps);
}

void SpecBase::doExtract(HECL::Database::Project& project, const ExtractPassInfo& info)
{
    if (m_isWii)
    {
        /* Extract update partition for repacking later */
        const HECL::SystemString& target = project.getProjectRootPath().getAbsolutePath();
        NOD::DiscBase::IPartition* update = m_disc->getUpdatePartition();
        if (update)
            update->getFSTRoot().extractToDirectory(target, info.force);

        if (!m_standalone)
        {
            NOD::DiscBase::IPartition* data = m_disc->getDataPartition();
            const NOD::DiscBase::IPartition::Node& root = data->getFSTRoot();
            for (const NOD::DiscBase::IPartition::Node& child : root)
                if (child.getKind() == NOD::DiscBase::IPartition::Node::NODE_FILE)
                    child.extractToDirectory(target, info.force);
        }
    }
    extractFromDisc(project, *m_disc.get(), info.force);
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
