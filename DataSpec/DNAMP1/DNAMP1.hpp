#ifndef __DNAMP1_HPP__
#define __DNAMP1_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace Retro
{
namespace DNAMP1
{

extern LogVisor::LogModule Log;

/* MP1-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    HECL::Database::Project& m_project;
    const NOD::DiscBase::IPartition::Node& m_node;
    PAK m_pak;
    UniqueResult uniqueCheck(const PAK::Entry& entry);
public:
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

    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const PAK::Entry& entry);
    const std::string& getName() const {return m_node.getName();}
    const HECL::SystemString& getLevelString() const {return m_levelString;}

    using PAKType = PAK;
    const PAKType& getPAK() const {return m_pak;}
    const NOD::DiscBase::IPartition::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
};

}
}

#endif // __DNAMP1_HPP__
