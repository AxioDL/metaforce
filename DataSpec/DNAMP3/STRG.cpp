#include "STRG.hpp"
#include "DNAMP3.hpp"

namespace DataSpec::DNAMP3 {

void STRG::_read(athena::io::IStreamReader& reader) {
  atUint32 langCount = reader.readUint32Big();
  atUint32 strCount = reader.readUint32Big();

  atUint32 nameCount = reader.readUint32Big();
  atUint32 nameTableSz = reader.readUint32Big();
  if (nameTableSz) {
    std::unique_ptr<uint8_t[]> nameTableBuf(new uint8_t[nameTableSz]);
    reader.readUBytesToBuf(nameTableBuf.get(), nameTableSz);
    struct NameIdxEntry {
      atUint32 nameOff;
      atUint32 strIdx;
    }* nameIndex = (NameIdxEntry*)nameTableBuf.get();
    for (atUint32 n = 0; n < nameCount; ++n) {
      const char* name = (char*)(nameTableBuf.get() + hecl::SBig(nameIndex[n].nameOff));
      names[name] = hecl::SBig(nameIndex[n].strIdx);
    }
  }

  std::vector<DNAFourCC> readLangs;
  readLangs.reserve(langCount);
  for (atUint32 l = 0; l < langCount; ++l) {
    DNAFourCC lang;
    lang.read(reader);
    readLangs.emplace_back(lang);
  }
  std::unique_ptr<atUint32[]> strOffs(new atUint32[langCount * strCount]);
  for (atUint32 l = 0; l < langCount; ++l) {
    reader.readUint32Big();
    for (atUint32 s = 0; s < strCount; ++s)
      strOffs[l * strCount + s] = reader.readUint32Big();
  }

  atUint64 strBase = reader.position();
  langs.clear();
  langs.reserve(langCount);
  for (atUint32 l = 0; l < langCount; ++l) {
    std::vector<std::string> strs;
    for (atUint32 s = 0; s < strCount; ++s) {
      reader.seek(strBase + strOffs[l * strCount + s], athena::SeekOrigin::Begin);
      atUint32 len = reader.readUint32Big();
      strs.emplace_back(reader.readString(len));
    }
    langs.emplace_back(readLangs[l], strs);
  }

  langMap.clear();
  langMap.reserve(langCount);
  for (std::pair<DNAFourCC, std::vector<std::string>>& item : langs)
    langMap.emplace(item.first, &item.second);
}

template <>
void STRG::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
  atUint32 magic = reader.readUint32Big();
  if (magic != 0x87654321) {
    Log.report(logvisor::Error, fmt("invalid STRG magic"));
    return;
  }

  atUint32 version = reader.readUint32Big();
  if (version != 3) {
    Log.report(logvisor::Error, fmt("invalid STRG version"));
    return;
  }

  _read(reader);
}

template <>
void STRG::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  const athena::io::YAMLNode* root = reader.getRootNode();

  /* Validate Pass */
  if (root->m_type == YAML_MAPPING_NODE) {
    for (const auto& lang : root->m_mapChildren) {
      if (!lang.first.compare("names"))
        continue;
      if (lang.first.size() != 4) {
        Log.report(logvisor::Warning, fmt("STRG language string '{}' must be exactly 4 characters; skipping"), lang.first);
        return;
      }
      if (lang.second->m_type != YAML_SEQUENCE_NODE) {
        Log.report(logvisor::Warning, fmt("STRG language string '{}' must contain a sequence; skipping"), lang.first);
        return;
      }
      for (const auto& str : lang.second->m_seqChildren) {
        if (str->m_type != YAML_SCALAR_NODE) {
          Log.report(logvisor::Warning, fmt("STRG language '{}' must contain all scalars; skipping"), lang.first);
          return;
        }
      }
    }
  } else {
    Log.report(logvisor::Warning, fmt("STRG must have a mapping root node; skipping"));
    return;
  }

  const athena::io::YAMLNode* nameYAML = root->findMapChild("names");
  names.clear();
  if (nameYAML && nameYAML->m_type == YAML_MAPPING_NODE)
    for (const auto& item : nameYAML->m_mapChildren)
      if (item.second->m_type == YAML_SCALAR_NODE)
        names[item.first] = athena::io::NodeToVal<atInt32>(item.second.get());

  langs.clear();
  langs.reserve(root->m_mapChildren.size());
  for (const auto& item : root->m_mapChildren) {
    if (!item.first.compare("names") || item.first.size() != 4 || item.second->m_type != YAML_SEQUENCE_NODE)
      continue;

    std::vector<std::string> strs;
    for (const auto& node : item.second->m_seqChildren)
      if (node->m_type == YAML_SCALAR_NODE)
        strs.emplace_back(node->m_scalarString);
    langs.emplace_back(std::make_pair(DNAFourCC(item.first.c_str()), std::move(strs)));
  }

  langMap.clear();
  langMap.reserve(langs.size());
  for (std::pair<DNAFourCC, std::vector<std::string>>& item : langs)
    langMap.emplace(item.first, &item.second);
}

template <>
void STRG::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  writer.writeUint32Big(0x87654321);
  writer.writeUint32Big(3);
  writer.writeUint32Big(langs.size());
  atUint32 strCount = STRG::count();
  writer.writeUint32Big(strCount);

  atUint32 nameTableSz = names.size() * 8;
  for (const auto& name : names)
    nameTableSz += name.first.size() + 1;
  writer.writeUint32Big(names.size());
  writer.writeUint32Big(nameTableSz);
  atUint32 offset = names.size() * 8;
  for (const auto& name : names) {
    writer.writeUint32Big(offset);
    writer.writeInt32Big(name.second);
    offset += name.first.size() + 1;
  }
  for (const auto& name : names)
    writer.writeString(name.first);

  for (const auto& lang : langs)
    lang.first.write(writer);

  offset = 0;
  for (const auto& lang : langs) {
    atUint32 langSz = 0;
    for (const std::string& str : lang.second)
      langSz += str.size() + 5;
    writer.writeUint32Big(langSz);

    for (const std::string& str : lang.second) {
      writer.writeUint32Big(offset);
      offset += str.size() + 5;
    }
  }

  for (atUint32 s = 0; s < strCount; ++s) {
    for (const auto& lang : langs) {
      if (s >= lang.second.size()) {
        writer.writeUint32Big(1);
        writer.writeUByte(0);
      } else {
        const std::string& str = lang.second[s];
        writer.writeUint32Big(str.size() + 1);
        writer.writeString(str);
      }
    }
  }
}

template <>
void STRG::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 24;
  __isz += names.size() * 8;
  for (const auto& name : names)
    __isz += name.first.size() + 1;

  __isz += langs.size() * 4;

  for (const auto& lang : langs)
    __isz += 4 + lang.second.size() * 4;

  size_t strCount = STRG::count();
  for (atUint32 s = 0; s < strCount; ++s) {
    for (const auto& lang : langs) {
      if (s >= lang.second.size()) {
        __isz += 5;
      } else {
        const std::string& str = lang.second[s];
        __isz += str.size() + 5;
      }
    }
  }
}

template <>
void STRG::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  for (const auto& item : langs) {
    if (auto v = writer.enterSubVector(item.first.toString().c_str()))
      for (const std::string& str : item.second)
        writer.writeString(nullptr, str);
  }

  if (names.size()) {
    if (auto rec = writer.enterSubRecord("names"))
      for (const auto& item : names)
        if (auto rec = writer.enterSubRecord(item.first.c_str()))
          writer.writeInt32(nullptr, item.second);
  }
}

const char* STRG::DNAType() { return "urde::DNAMP3::STRG"; }

} // namespace DataSpec::DNAMP3
