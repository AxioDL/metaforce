#include <utility>

#define NOD_ATHENA 1
#include "SpecBase.hpp"
#include "DNAMP1/PAK.hpp"
#include "DNAMP2/MLVL.hpp"
#include "DNAMP2/STRG.hpp"

namespace Retro
{

static LogVisor::LogModule Log("Retro::SpecMP2");

struct SpecMP2 : SpecBase
{
    bool checkStandaloneID(const char* id) const
    {
        if (!memcmp(id, "G2M", 3))
            return true;
        return false;
    }

    struct DiscPAK
    {
        const NOD::DiscBase::IPartition::Node& node;
        DNAMP1::PAK pak;
        DiscPAK(const NOD::DiscBase::IPartition::Node& n) : node(n) {}
    };
    std::vector<DiscPAK> m_paks;
    std::map<std::string, DiscPAK*, CaseInsensitiveCompare> m_orderedPaks;

    void buildPaks(NOD::DiscBase::IPartition::Node& root,
                   const std::vector<HECL::SystemString>& args,
                   ExtractReport& rep)
    {
        m_paks.clear();
        for (const NOD::DiscBase::IPartition::Node& child : root)
        {
            const std::string& name = child.getName();
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
            if (name.size() > 4)
            {
                std::string::iterator extit = lowerName.end() - 4;
                if (!std::string(extit, lowerName.end()).compare(".pak"))
                {
                    /* This is a pak */
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

                    if (good)
                    {
                        m_paks.emplace_back(child);
                        NOD::AthenaPartReadStream rs(child.beginReadStream());
                        m_paks.back().pak.read(rs);
                    }
                }
            }
        }

        /* Sort PAKs alphabetically */
        m_orderedPaks.clear();
        for (DiscPAK& dpak : m_paks)
            m_orderedPaks[dpak.node.getName()] = &dpak;

        /* Assemble extract report */
        for (const std::pair<std::string, DiscPAK*>& item : m_orderedPaks)
        {
            rep.childOpts.emplace_back();
            ExtractReport& childRep = rep.childOpts.back();
            childRep.name = item.first;

            DNAMP1::PAK& pak = item.second->pak;
            for (DNAMP1::PAK::Entry& entry : pak.m_entries)
            {
                if (entry.type == MLVL)
                {
                    PAKEntryReadStream rs = entry.beginReadStream(item.second->node);
                    DNAMP2::MLVL mlvl;
                    mlvl.read(rs);
                    const DNAMP1::PAK::Entry* nameEnt = pak.lookupEntry(mlvl.worldNameId);
                    if (nameEnt)
                    {
                        PAKEntryReadStream rs = nameEnt->beginReadStream(item.second->node);
                        DNAMP2::STRG mlvlName;
                        mlvlName.read(rs);
                        if (childRep.desc.size())
                            childRep.desc += _S(", ");
                        childRep.desc += mlvlName.getSystemString(ENGL, 0);
                    }
                }
            }
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

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP2");
        rep.desc = _S("Metroid Prime 2 ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            HECL::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView.sys_str() + _S(")");
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
        std::vector<HECL::SystemString> mp2args;
        if (args.size())
        {
            /* Needs filter */
            for (const HECL::SystemString& arg : args)
            {
                size_t slashPos = arg.find(_S('/'));
                if (slashPos == HECL::SystemString::npos)
                    slashPos = arg.find(_S('\\'));
                if (slashPos != HECL::SystemString::npos)
                {
                    HECL::SystemString lowerArg(arg.begin(), arg.begin() + slashPos);
                    HECL::ToLower(lowerArg);
                    if (!lowerArg.compare(_S("mp2")))
                        mp2args.emplace_back(HECL::SystemString(arg.begin() + slashPos + 1, arg.end()));
                }
            }
        }

        NOD::DiscGCN::IPartition* partition = disc.getDataPartition();
        NOD::DiscBase::IPartition::Node& root = partition->getFSTRoot();
        NOD::DiscBase::IPartition::Node::DirectoryIterator dolIt = root.find("rs5mp2_p.dol");
        if (dolIt == root.end())
            return false;

        std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
        const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP2");
        rep.desc = _S("Metroid Prime 2 ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            HECL::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView.sys_str() + _S(")");
        }

        /* Iterate PAKs and build level options */
        NOD::DiscBase::IPartition::Node::DirectoryIterator mp2It = root.find("MP2");
        if (mp2It == root.end())
            return false;
        buildPaks(*mp2It, mp2args, rep);

        return true;
    }

    bool extractFromDisc(NOD::DiscBase& disc, const HECL::Database::Project& project)
    {
    }

    bool checkFromProject(HECL::Database::Project& proj)
    {
    }
    bool readFromProject(HECL::Database::Project& proj)
    {
    }

    bool visitGameObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
    {
    }
    struct LevelSpec : public ILevelSpec
    {
        bool visitLevelObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
        {
        }
        struct AreaSpec : public IAreaSpec
        {
            bool visitAreaObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
            {
            }
        };
        bool visitAreas(std::function<bool(const IAreaSpec&)>)
        {
        }
    };
    bool visitLevels(std::function<bool(const ILevelSpec&)>)
    {
    }
};

static HECL::Database::DataSpecEntry SpecMP2
(
    _S("MP2"),
    _S("Data specification for original Metroid Prime 2 engine"),
    [](HECL::Database::DataSpecTool tool) -> HECL::Database::IDataSpec* {return new struct SpecMP2;}
);

}
