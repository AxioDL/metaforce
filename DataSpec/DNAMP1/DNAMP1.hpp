#ifndef __DNAMP1_HPP__
#define __DNAMP1_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"
#include "zeus/CMatrix4f.hpp"

namespace DataSpec
{
namespace DNAMP1
{

extern logvisor::Module Log;

/* MP1-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    hecl::Database::Project& m_project;
    const nod::Node& m_node;
    PAK m_pak;
public:
    bool m_doExtract;
    using Level = DataSpec::Level<UniqueID32>;
    std::unordered_map<UniqueID32, Level> m_levelDeps;
    hecl::SystemString m_levelString;

    PAKBridge(hecl::Database::Project& project,
              const nod::Node& node,
              bool doExtract=true);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const PAK& pak, const PAK::Entry& entry);
    const std::string& getName() const {return m_node.getName();}
    const hecl::SystemString& getLevelString() const {return m_levelString;}
    using PAKType = PAK;
    const PAKType& getPAK() const {return m_pak;}
    const nod::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, std::string>>& cskrCinfToAncs) const;

    void addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, zeus::CMatrix4f>& addTo,
            std::unordered_map<UniqueID32, hecl::ProjectPath>& pathOverrides) const;
};

}
}

#endif // __DNAMP1_HPP__
