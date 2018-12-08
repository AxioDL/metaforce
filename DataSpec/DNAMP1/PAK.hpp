#pragma once

#include <unordered_map>

#include <nod/DiscBase.hpp>
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAMP1 {

struct PAK : BigDNA {
  bool m_useLzo;
  bool m_noShare;
  PAK(bool useLzo, bool noShare) : m_useLzo(useLzo), m_noShare(noShare) {}
  AT_DECL_EXPLICIT_DNA

  struct NameEntry : BigDNA {
    AT_DECL_DNA
    DNAFourCC type;
    UniqueID32 id;
    Value<atUint32> nameLen;
    String<AT_DNA_COUNT(nameLen)> name;
  };

  struct Entry : BigDNA {
    AT_DECL_DNA
    Value<atUint32> compressed;
    DNAFourCC type;
    UniqueID32 id;
    Value<atUint32> size;
    Value<atUint32> offset;
    UniqueResult unique;
    std::string name; /* backreferencing name for RE purposes */

    std::unique_ptr<atUint8[]> getBuffer(const nod::Node& pak, atUint64& szOut) const;
    inline PAKEntryReadStream beginReadStream(const nod::Node& pak, atUint64 off = 0) const {
      atUint64 sz;
      std::unique_ptr<atUint8[]> buf = getBuffer(pak, sz);
      return PAKEntryReadStream(std::move(buf), sz, off);
    }
  };

  std::vector<NameEntry> m_nameEntries;
  std::unordered_map<UniqueID32, Entry> m_entries;
  std::vector<UniqueID32> m_firstEntries;
  std::unordered_map<std::string, UniqueID32> m_nameMap;
  std::unordered_set<UniqueID32> m_dupeMREAs;

  const Entry* lookupEntry(const UniqueID32& id) const;
  const Entry* lookupEntry(std::string_view name) const;
  std::string bestEntryName(const nod::Node& pakNode, const Entry& entry, bool& named) const;

  bool mreaHasDupeResources(const UniqueID32& id) const { return m_dupeMREAs.find(id) != m_dupeMREAs.cend(); }

  using IDType = UniqueID32;
};

} // namespace DataSpec::DNAMP1
