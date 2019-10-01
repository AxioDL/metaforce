#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "PAK.hpp"
#include "zeus/CMatrix4f.hpp"

namespace DataSpec::DNAMP1 {

extern logvisor::Module Log;

/* MP1-specific, one-shot PAK traversal/extraction class */
class PAKBridge {
  const nod::Node& m_node;
  PAK m_pak;

public:
  bool m_doExtract;
  using Level = DataSpec::Level<UniqueID32>;
  std::unordered_map<UniqueID32, Level> m_levelDeps;
  UniqueID32 m_levelId;
  hecl::SystemString m_levelString;

  PAKBridge(const nod::Node& node, bool doExtract = true);
  void build();
  static ResExtractor<PAKBridge> LookupExtractor(const nod::Node& pakNode, const PAK& pak, const PAK::Entry& entry);
  std::string_view getName() const { return m_node.getName(); }
  UniqueID32 getLevelId() const { return m_levelId; }
  hecl::SystemStringView getLevelString() const { return m_levelString; }
  using PAKType = PAK;
  const PAKType& getPAK() const { return m_pak; }
  const nod::Node& getNode() const { return m_node; }

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const;

  void addPATHToMREA(PAKRouter<PAKBridge>& pakRouter, std::unordered_map<UniqueID32, UniqueID32>& pathToMrea) const;

  void addMAPATransforms(PAKRouter<PAKBridge>& pakRouter, std::unordered_map<UniqueID32, zeus::CMatrix4f>& addTo,
                         std::unordered_map<UniqueID32, hecl::ProjectPath>& pathOverrides) const;
};

} // namespace DataSpec::DNAMP1
