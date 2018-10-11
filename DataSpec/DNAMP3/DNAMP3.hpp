#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::DNAMP3
{

extern logvisor::Module Log;

/* MP3-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    const nod::Node& m_node;
    PAK m_pak;
public:
    bool m_doExtract;
    using Level = DataSpec::Level<UniqueID64>;
    std::unordered_map<UniqueID64, Level> m_levelDeps;
    hecl::SystemString m_levelString;

    PAKBridge(const nod::Node& node, bool doExtract=true);
    void build();
    static ResExtractor<PAKBridge> LookupExtractor(const nod::Node& pakNode, const PAK& pak, const PAK::Entry& entry);
    std::string_view getName() const {return m_node.getName();}
    hecl::SystemStringView getLevelString() const {return m_levelString;}

    using PAKType = PAK;
    const PAKType& getPAK() const {return m_pak;}
    const nod::Node& getNode() const {return m_node;}

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID64>& charAssoc) const;

    void addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID64, zeus::CMatrix4f>& addTo,
            std::unordered_map<UniqueID64, hecl::ProjectPath>& pathOverrides) const;
};

}

