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
    const NOD::Node& m_node;
    PAK m_pak;
public:
    bool m_doExtract;
    using Level = Level<UniqueID32>;
    std::unordered_map<UniqueID32, Level> m_levelDeps;
    HECL::SystemString m_levelString;

    PAKBridge(HECL::Database::Project& project,
              const NOD::Node& node,
              bool doExtract=true);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const PAK::Entry& entry);
    const std::string& getName() const {return m_node.getName();}
    const HECL::SystemString& getLevelString() const {return m_levelString;}
    using PAKType = PAK;
    const PAKType& getPAK() const {return m_pak;}
    const NOD::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
};

}
}

#endif // __DNAMP1_HPP__
