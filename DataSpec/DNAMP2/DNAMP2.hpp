#ifndef __DNAMP2_HPP__
#define __DNAMP2_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "../DNAMP1/PAK.hpp"

namespace Retro
{
namespace DNAMP2
{

extern LogVisor::LogModule Log;

/* MP2-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    HECL::Database::Project& m_project;
    const NOD::DiscBase::IPartition::Node& m_node;
    DNAMP1::PAK m_pak;
    struct Level
    {
        HECL::SystemString name;
        struct Area
        {
            HECL::SystemString name;
            struct Layer
            {
                HECL::SystemString name;
                std::unordered_set<UniqueID32> resources;
            };
            std::vector<Layer> layers;
            std::unordered_set<UniqueID32> resources;
        };
        std::unordered_map<UniqueID32, Area> areas;
    };
    std::unordered_map<UniqueID32, Level> m_levelDeps;
    HECL::SystemString m_levelString;
    UniqueResult uniqueCheck(const DNAMP1::PAK::Entry& entry);
public:
    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const DNAMP1::PAK::Entry& entry);
    const std::string& getName() const {return m_node.getName();}
    const HECL::SystemString& getLevelString() const {return m_levelString;}

    using PAKType = DNAMP1::PAK;
    const PAKType& getPAK() const {return m_pak;}
    const NOD::DiscBase::IPartition::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
};

}
}

#endif // __DNAMP2_HPP__
