#include <utility>
#include <stdio.h>
#include <string.h>

#include "SpecBase.hpp"
#include "DNAMP1/DNAMP1.hpp"

namespace Retro
{

static LogVisor::LogModule Log("Retro::SpecMP1");
extern HECL::Database::DataSpecEntry SpecEntMP1;

struct SpecMP1 : SpecBase
{
    bool checkStandaloneID(const char* id) const
    {
        if (!memcmp(id, "GM8", 3))
            return true;
        return false;
    }

    std::vector<const NOD::DiscBase::IPartition::Node*> m_nonPaks;
    std::vector<DNAMP1::PAKBridge> m_paks;
    std::map<std::string, DNAMP1::PAKBridge*, CaseInsensitiveCompare> m_orderedPaks;

    HECL::ProjectPath m_workPath;
    HECL::ProjectPath m_cookPath;
    PAKRouter<DNAMP1::PAKBridge> m_pakRouter;

    SpecMP1(HECL::Database::Project& project)
    : SpecBase(project),
      m_workPath(project.getProjectRootPath(), _S("MP1")),
      m_cookPath(project.getProjectCookedPath(SpecEntMP1), _S("MP1")),
      m_pakRouter(*this, m_workPath, m_cookPath) {}

    void buildPaks(NOD::DiscBase::IPartition::Node& root,
                   const std::vector<HECL::SystemString>& args,
                   ExtractReport& rep)
    {
        m_nonPaks.clear();
        m_paks.clear();
        for (const NOD::DiscBase::IPartition::Node& child : root)
        {
            bool isPak = false;
            const std::string& name = child.getName();
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
            if (name.size() > 4)
            {
                std::string::iterator extit = lowerName.end() - 4;
                if (!std::string(extit, lowerName.end()).compare(".pak"))
                {
                    /* This is a pak */
                    isPak = true;
                    std::string lowerBase(lowerName.begin(), extit);

                    /* Needs filter */
                    bool good = true;
                    if (args.size())
                    {
                        good = false;
                        if (!lowerName.compare(0, 7, "metroid"))
                        {
                            HECL::SystemChar idxChar = lowerName[7];
                            for (const HECL::SystemString& arg : args)
                            {
                                if (arg.size() == 1 && iswdigit(arg[0]))
                                    if (arg[0] == idxChar)
                                        good = true;
                            }
                        }
                        else
                            good = true;

                        if (!good)
                        {
                            for (const HECL::SystemString& arg : args)
                            {
#if HECL_UCS2
                                std::string lowerArg = HECL::WideToUTF8(arg);
#else
                                std::string lowerArg = arg;
#endif
                                std::transform(lowerArg.begin(), lowerArg.end(), lowerArg.begin(), tolower);
                                if (!lowerArg.compare(0, lowerBase.size(), lowerBase))
                                    good = true;
                            }
                        }
                    }

                    m_paks.emplace_back(m_project, child, good);

                }
            }

            if (!isPak)
                m_nonPaks.push_back(&child);
        }

        /* Sort PAKs alphabetically */
        m_orderedPaks.clear();
        for (DNAMP1::PAKBridge& dpak : m_paks)
            m_orderedPaks[dpak.getName()] = &dpak;

        /* Assemble extract report */
        for (const std::pair<std::string, DNAMP1::PAKBridge*>& item : m_orderedPaks)
        {
            if (!item.second->m_doExtract)
                continue;
            rep.childOpts.emplace_back();
            ExtractReport& childRep = rep.childOpts.back();
            HECL::SystemStringView nameView(item.first);
            childRep.name = nameView;
            childRep.desc = item.second->getLevelString();
        }
    }

    bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                 const HECL::SystemString& regstr,
                                 const std::vector<HECL::SystemString>& args,
                                 std::vector<ExtractReport>& reps)
    {
        NOD::DiscGCN::IPartition* partition = disc.getDataPartition();
        std::unique_ptr<uint8_t[]> dolBuf = partition->getDOLBuf();
        const char* buildInfo = (char*)memmem(dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16) + 19;

        if (!buildInfo)
            return false;

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP1");
        rep.desc = _S("Metroid Prime ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            HECL::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");
        }

        /* Iterate PAKs and build level options */
        NOD::DiscBase::IPartition::Node& root = partition->getFSTRoot();
        buildPaks(root, args, rep);

        return true;
    }

    bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                              const HECL::SystemString& regstr,
                              const std::vector<HECL::SystemString>& args,
                              std::vector<ExtractReport>& reps)
    {
        std::vector<HECL::SystemString> mp1args;
        bool doExtract = false;
        if (args.size())
        {
            /* Needs filter */
            for (const HECL::SystemString& arg : args)
            {
                HECL::SystemString lowerArg = arg;
                HECL::ToLower(lowerArg);
                if (!lowerArg.compare(0, 3, _S("mp1")))
                {
                    doExtract = true;
                    size_t slashPos = arg.find(_S('/'));
                    if (slashPos == HECL::SystemString::npos)
                        slashPos = arg.find(_S('\\'));
                    if (slashPos != HECL::SystemString::npos)
                        mp1args.emplace_back(HECL::SystemString(arg.begin() + slashPos + 1, arg.end()));
                }
            }
        }
        else
            doExtract = true;

        if (!doExtract)
            return false;

        NOD::DiscGCN::IPartition* partition = disc.getDataPartition();
        NOD::DiscBase::IPartition::Node& root = partition->getFSTRoot();
        NOD::DiscBase::IPartition::Node::DirectoryIterator dolIt = root.find("rs5mp1_p.dol");
        if (dolIt == root.end())
            return false;

        std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
        const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP1");
        rep.desc = _S("Metroid Prime ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            HECL::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");
        }

        /* Iterate PAKs and build level options */
        NOD::DiscBase::IPartition::Node::DirectoryIterator mp1It = root.find("MP1");
        if (mp1It == root.end())
            return false;
        buildPaks(*mp1It, mp1args, rep);

        return true;
    }

    bool extractFromDisc(NOD::DiscBase&, bool force, FExtractProgress progress)
    {
        progress(_S("Indexing PAKs"), _S(""), 2, 0.0);
        m_pakRouter.build(m_paks, [&progress](float factor)
        {
            progress(_S("Indexing PAKs"), _S(""), 2, factor);
        });
        progress(_S("Indexing PAKs"), _S(""), 2, 1.0);

        m_workPath.makeDir();
        progress(_S("MP1 Root"), _S(""), 3, 0.0);
        int prog = 0;
        for (const NOD::DiscBase::IPartition::Node* node : m_nonPaks)
        {
            node->extractToDirectory(m_workPath.getAbsolutePath(), force);
            HECL::SystemStringView nameView(node->getName());
            progress(_S("MP1 Root"), nameView.sys_str().c_str(), 3, prog++ / (float)m_nonPaks.size());
        }
        progress(_S("MP1 Root"), _S(""), 3, 1.0);

        const HECL::ProjectPath& cookPath = m_project.getProjectCookedPath(SpecEntMP1);
        cookPath.makeDir();
        m_cookPath.makeDir();

        int compIdx = 4;
        prog = 0;
        for (std::pair<std::string, DNAMP1::PAKBridge*> pair : m_orderedPaks)
        {
            DNAMP1::PAKBridge& pak = *pair.second;
            if (!pak.m_doExtract)
                continue;

            const std::string& name = pak.getName();
            HECL::SystemStringView sysName(name);

            progress(sysName.sys_str().c_str(), _S(""), compIdx, 0.0);
            m_pakRouter.extractResources(pak, force,
            [&progress, &sysName, &compIdx](const HECL::SystemChar* substr, float factor)
            {
                progress(sysName.sys_str().c_str(), substr, compIdx, factor);
            });
            progress(sysName.sys_str().c_str(), _S(""), compIdx++, 1.0);
        }

        return true;
    }

    bool checkFromProject()
    {
        return false;
    }
    bool readFromProject()
    {
        return false;
    }

    bool visitGameObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
    {
        return false;
    }
    struct LevelSpec : public ILevelSpec
    {
        bool visitLevelObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
        {
            return false;
        }
        struct AreaSpec : public IAreaSpec
        {
            bool visitAreaObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
            {
                return false;
            }
        };
        bool visitAreas(std::function<bool(const IAreaSpec&)>)
        {
            return false;
        }
    };
    bool visitLevels(std::function<bool(const ILevelSpec&)>)
    {
        return false;
    }
};

HECL::Database::DataSpecEntry SpecEntMP1 =
{
    _S("MP1"),
    _S("Data specification for original Metroid Prime engine"),
    [](HECL::Database::Project& project, HECL::Database::DataSpecTool)
    -> HECL::Database::IDataSpec* {return new struct SpecMP1(project);}
};

}


