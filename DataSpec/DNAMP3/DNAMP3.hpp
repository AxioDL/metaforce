#ifndef __DNAMP3_HPP__
#define __DNAMP3_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec
{
namespace DNAMP3
{

extern logvisor::Module Log;

/* MP3-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    hecl::Database::Project& m_project;
    const nod::Node& m_node;
    PAK m_pak;
public:
    bool m_doExtract;
    using Level = Level<UniqueID64>;
    std::unordered_map<UniqueID64, Level> m_levelDeps;
    hecl::SystemString m_levelString;

    PAKBridge(hecl::Database::Project& project,
              const nod::Node& node,
              bool doExtract=true);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const PAK::Entry& entry);
    inline const std::string& getName() const {return m_node.getName();}
    inline hecl::SystemString getLevelString() const {return m_levelString;}

    using PAKType = PAK;
    inline const PAKType& getPAK() const {return m_pak;}
    inline const nod::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID64, std::pair<UniqueID64, UniqueID64>>& addTo) const;
};

}
}

#endif // __DNAMP3_HPP__
