#pragma once

#include <unordered_map>

#include <lzo/lzo1x.h>
#include <nod/DiscBase.hpp>
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAMP3 {

extern const hecl::FourCC CMPD;

struct PAK : BigDNA {
  bool m_noShare;
  PAK(bool noShare) : m_noShare(noShare) {}

  struct Header : BigDNA {
    AT_DECL_DNA
    Value<atUint32> version;
    Value<atUint32> headSz;
    Value<atUint8> md5sum[16];
    Seek<40, athena::Current> seek;
  } m_header;

  struct NameEntry : BigDNA {
    AT_DECL_DNA
    String<-1> name;
    DNAFourCC type;
    UniqueID64 id;
  };

  struct Entry : BigDNA {
    AT_DECL_DNA
    Value<atUint32> compressed;
    DNAFourCC type;
    UniqueID64 id;
    Value<atUint32> size;
    Value<atUint32> offset;
    UniqueResult unique;
    std::string name;

    std::unique_ptr<atUint8[]> getBuffer(const nod::Node& pak, atUint64& szOut) const;
    inline PAKEntryReadStream beginReadStream(const nod::Node& pak, atUint64 off = 0) const {
      atUint64 sz;
      std::unique_ptr<atUint8[]> buf = getBuffer(pak, sz);
      return PAKEntryReadStream(std::move(buf), sz, off);
    }
  };

  std::vector<NameEntry> m_nameEntries;
  std::unordered_map<UniqueID64, Entry> m_entries;
  std::vector<UniqueID64> m_firstEntries;
  std::unordered_map<std::string, UniqueID64> m_nameMap;
  std::unordered_set<UniqueID64> m_dupeMREAs;

  AT_DECL_EXPLICIT_DNA

  const Entry* lookupEntry(const UniqueID64& id) const;
  const Entry* lookupEntry(std::string_view name) const;
  std::string bestEntryName(const nod::Node& pakNode, const Entry& entry, bool& named) const;

  bool mreaHasDupeResources(const UniqueID64& id) const { return m_dupeMREAs.find(id) != m_dupeMREAs.cend(); }

  typedef UniqueID64 IDType;
};

} // namespace DataSpec::DNAMP3
