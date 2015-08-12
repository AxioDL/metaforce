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
    std::unordered_map<UniqueID32, Area> m_areaDeps;
    HECL::SystemString m_levelString;
    UniqueResult uniqueCheck(const PAK::Entry& entry);
public:
    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const PAK::Entry& entry);
    inline const std::string& getName() const {return m_node.getName();}
    inline const HECL::SystemString& getLevelString() const {return m_levelString;}

    typedef PAK PAKType;
    inline const PAKType& getPAK() const {return m_pak;}
    inline const NOD::DiscBase::IPartition::Node& getNode() const {return m_node;}
};

}
}

#endif // __DNAMP1_HPP__
