#include <utility>

#define NOD_ATHENA 1
#include "SpecBase.hpp"
#include "DNAMP1/PAK.hpp"
#include "DNAMP1/MLVL.hpp"
#include "DNAMP1/STRG.hpp"

namespace Retro
{

struct SpecMP1 : SpecBase
{
    struct DiscPAK
    {
        const NOD::DiscBase::IPartition::Node& node;
        DNAMP1::PAK pak;
        DiscPAK(const NOD::DiscBase::IPartition::Node& n) : node(n) {}
    };
    std::vector<DiscPAK> m_paks;
    std::map<std::string, DiscPAK*, CaseInsensitiveCompare> m_orderedPaks;

    void buildPaks(NOD::DiscBase::IPartition::Node& root,
                   const std::vector<const HECL::SystemString*>& args,
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
                            for (const HECL::SystemString* arg : args)
                            {
                                if (arg->size() == 1 && iswdigit((*arg)[0]))
                                    if ((*arg)[0] == idxChar)
                                        good = true;
                            }
                        }
                        else
                            good = true;

                        if (!good)
                        {
                            for (const HECL::SystemString* arg : args)
                            {
#if HECL_UCS2
                                std::string lowerArg = HECL::WideToUTF8(*arg);
#else
                                std::string lowerArg = *arg;
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
        for (std::pair<std::string, DiscPAK*> item : m_orderedPaks)
        {
            rep.childOpts.emplace_back();
            ExtractReport& childRep = rep.childOpts.back();
            childRep.name = item.first;

            DNAMP1::PAK& pak = item.second->pak;
            for (DNAMP1::PAK::Entry& entry : pak.m_entries)
            {
                static const HECL::FourCC MLVLfourcc("MLVL");
                if (entry.type == MLVLfourcc)
                {
                    NOD::AthenaPartReadStream rs(item.second->node.beginReadStream(entry.offset));
                    DNAMP1::MLVL mlvl;
                    mlvl.read(rs);
                    const DNAMP1::PAK::Entry* nameEnt = pak.lookupEntry(mlvl.worldNameId);
                    if (nameEnt)
                    {
                        DNAMP1::STRG mlvlName;
                        NOD::AthenaPartReadStream rs(item.second->node.beginReadStream(nameEnt->offset));
                        mlvlName.read(rs);
                        if (childRep.desc.size())
                            childRep.desc += _S(", ");
#if HECL_UCS2
                        childRep.desc += mlvlName.langs[0].strings[0];
#else
                        childRep.desc += HECL::WideToUTF8(mlvlName.langs[0].strings[0]);
#endif
                    }
                }
            }
        }
    }

    bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                 const std::string& regstr,
                                 const std::vector<const HECL::SystemString*>& args,
                                 std::vector<ExtractReport>& reps)
    {
        NOD::DiscGCN::IPartition* partition = disc.getDataPartition();
        std::unique_ptr<uint8_t[]> dolBuf = partition->getDOLBuf();
        const char* buildInfo = (char*)memmem(dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16) + 19;

        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = "MP1";
        rep.desc = "Metroid Prime " + regstr;
        if (buildInfo)
            rep.desc += " (" + std::string(buildInfo) + ")";

        /* Iterate PAKs and build level options */
        NOD::DiscBase::IPartition::Node& root = disc.getDataPartition()->getFSTRoot();
        buildPaks(root, args, rep);

        return true;
    }

    bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                              const std::string& regstr,
                              const std::vector<const HECL::SystemString*>& args,
                              std::vector<ExtractReport>& reps)
    {
        return true;
    }
    bool extractFromDisc()
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

HECL::Database::DataSpecEntry SpecMP1 =
{
    _S("MP1"),
    _S("Data specification for original Metroid Prime engine"),
    [](HECL::Database::DataSpecTool) -> HECL::Database::IDataSpec* {return new struct SpecMP1;}
};

}


