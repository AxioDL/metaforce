#include <utility>

#define NOD_ATHENA 1
#include "SpecBase.hpp"
#include "DNAMP1/PAK.hpp"

namespace Retro
{

struct SpecMP1 : SpecBase
{
    std::map<std::string, std::pair<std::string, DNAMP1::PAK>> m_worldPaks;

    bool checkFromGCNDisc(NOD::DiscGCN& disc, ExtractOption& opts)
    {
        if (memcmp(disc.getHeader().gameID, "GM8", 3))
            return false;

        /* Iterate PAKs and build level options */
        m_worldPaks.clear();
        NOD::DiscBase::IPartition::Node& root = disc.getDataPartition()->getFSTRoot();
        for (const NOD::DiscBase::IPartition::Node& child : root)
        {
            std::string name = child.getName();
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            if (!name.compare(0, 7, "metroid") && !name.compare(8, 4, ".pak"))
            {
                /* This is a world pak */
                std::pair<std::map<std::string, std::pair<std::string, DNAMP1::PAK>>::iterator,bool> res =
                m_worldPaks.emplace(std::make_pair(name, std::make_pair(child.getName(), DNAMP1::PAK())));
                if (res.second)
                {
                    NOD::AthenaPartReadStream rs(child.beginReadStream());
                    res.first->second.second.read(rs);
                }
            }

        }

        return true;
    }
    bool readFromGCNDisc(NOD::DiscGCN& disc)
    {

    }

    bool checkFromWiiDisc(NOD::DiscWii& disc, ExtractOption& opts)
    {
        if (memcmp(disc.getHeader().gameID, "R3M", 3))
            return false;
        return true;
    }
    bool readFromWiiDisc(NOD::DiscWii& disc)
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


