#ifndef __DNAMP3_HPP__
#define __DNAMP3_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::DNAMP3
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
    using Level = DataSpec::Level<UniqueID64>;
    std::unordered_map<UniqueID64, Level> m_levelDeps;
    hecl::SystemString m_levelString;

    PAKBridge(hecl::Database::Project& project,
              const nod::Node& node,
              bool doExtract=true);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const PAK& pak, const PAK::Entry& entry);
    std::string_view getName() const {return m_node.getName();}
    hecl::SystemStringView getLevelString() const {return m_levelString;}

    using PAKType = PAK;
    const PAKType& getPAK() const {return m_pak;}
    const nod::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID64, std::pair<UniqueID64, UniqueID64>>& addTo,
            std::unordered_map<UniqueID64, std::pair<UniqueID64, std::string>>& cskrCinfToChar) const;

    void addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID64, zeus::CMatrix4f>& addTo,
            std::unordered_map<UniqueID64, hecl::ProjectPath>& pathOverrides) const;
};

}

#endif // __DNAMP3_HPP__
