#include "PAK.hpp"
#include "AGSC.hpp"

namespace DataSpec::DNAMP2 {

std::string PAK::bestEntryName(const nod::Node& pakNode, const Entry& entry, std::string& catalogueName) const {
  std::unordered_map<UniqueID32, Entry>::const_iterator search;
  if (entry.type == FOURCC('AGSC') && (search = m_entries.find(entry.id)) != m_entries.cend()) {
    /* Use internal AGSC name for entry */
    auto rs = search->second.beginReadStream(pakNode);
    AGSC::Header header;
    header.read(rs);
    catalogueName = header.groupName;
    return fmt::format(fmt("{}_{}"), header.groupName, entry.id);
  }

  return DNAMP1::PAK::bestEntryName(pakNode, entry, catalogueName);
}

} // namespace DataSpec::DNAMP2
