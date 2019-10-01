#include "DNACommon.hpp"
#include "PAK.hpp"
#include "boo/ThreadLocalPtr.hpp"

namespace DataSpec {

logvisor::Module LogDNACommon("urde::DNACommon");
ThreadLocalPtr<SpecBase> g_curSpec;
ThreadLocalPtr<PAKRouterBase> g_PakRouter;
ThreadLocalPtr<hecl::blender::Token> g_ThreadBlenderToken;
ThreadLocalPtr<hecl::Database::Project> UniqueIDBridge::s_Project;
UniqueID32 UniqueID32::kInvalidId;

template <class IDType>
hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const IDType& id, bool silenceWarnings) {
  /* Try PAKRouter first (only available at extract) */
  PAKRouterBase* pakRouter = g_PakRouter.get();
  if (pakRouter) {
    hecl::ProjectPath path = pakRouter->getWorking(id, silenceWarnings);
    if (path)
      return path;
  }

  /* Try project cache second (populated with paths read from YAML resources) */
  hecl::Database::Project* project = s_Project.get();
  if (!project) {
    if (pakRouter) {
      if (hecl::VerbosityLevel >= 1 && !silenceWarnings && id.isValid())
        LogDNACommon.report(logvisor::Warning, fmt("unable to translate {} to path"), id);
      return {};
    }
    LogDNACommon.report(logvisor::Fatal,
                        fmt("g_PakRouter or s_Project must be set to non-null before "
                            "calling UniqueIDBridge::TranslatePakIdToPath"));
    return {};
  }

  const hecl::ProjectPath* search = project->lookupBridgePath(id.toUint64());
  if (!search) {
    if (hecl::VerbosityLevel >= 1 && !silenceWarnings && id.isValid())
      LogDNACommon.report(logvisor::Warning, fmt("unable to translate {} to path"), id);
    return {};
  }
  return *search;
}
template hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const UniqueID32& id, bool silenceWarnings);
template hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const UniqueID64& id, bool silenceWarnings);
template hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const UniqueID128& id, bool silenceWarnings);

template <class IDType>
hecl::ProjectPath UniqueIDBridge::MakePathFromString(std::string_view str) {
  if (str.empty())
    return {};
  hecl::Database::Project* project = s_Project.get();
  if (!project)
    LogDNACommon.report(logvisor::Fatal, fmt("UniqueIDBridge::setGlobalProject must be called before MakePathFromString"));
  hecl::ProjectPath path = hecl::ProjectPath(*project, str);
  project->addBridgePathToCache(IDType(path).toUint64(), path);
  return path;
}
template hecl::ProjectPath UniqueIDBridge::MakePathFromString<UniqueID32>(std::string_view str);
template hecl::ProjectPath UniqueIDBridge::MakePathFromString<UniqueID64>(std::string_view str);

void UniqueIDBridge::SetThreadProject(hecl::Database::Project& project) { s_Project.reset(&project); }

/** PAK 32-bit Unique ID */
template <>
void UniqueID32::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  assign(reader.readUint32Big());
}
template <>
void UniqueID32::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  writer.writeUint32Big(m_id);
}
template <>
void UniqueID32::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader) {
  *this = UniqueIDBridge::MakePathFromString<UniqueID32>(reader.readString());
}
template <>
void UniqueID32::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer) {
  if (!isValid())
    return;
  hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
  if (!path)
    return;
  writer.writeString(path.getEncodableStringUTF8());
}
template <>
void UniqueID32::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
}

std::string UniqueID32::toString() const {
  return fmt::format(fmt("{}"), *this);
}

template <>
void UniqueID32Zero::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  UniqueID32::Enumerate<BigDNA::Read>(reader);
}
template <>
void UniqueID32Zero::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  writer.writeUint32Big(isValid() ? m_id : 0);
}
template <>
void UniqueID32Zero::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader) {
  UniqueID32::Enumerate<BigDNA::ReadYaml>(reader);
}
template <>
void UniqueID32Zero::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer) {
  UniqueID32::Enumerate<BigDNA::WriteYaml>(writer);
}
template <>
void UniqueID32Zero::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  UniqueID32::Enumerate<BigDNA::BinarySize>(s);
}

/** PAK 64-bit Unique ID */
template <>
void UniqueID64::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  assign(reader.readUint64Big());
}
template <>
void UniqueID64::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  writer.writeUint64Big(m_id);
}
template <>
void UniqueID64::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader) {
  *this = UniqueIDBridge::MakePathFromString<UniqueID64>(reader.readString());
}
template <>
void UniqueID64::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer) {
  if (!isValid())
    return;
  hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
  if (!path)
    return;
  writer.writeString(path.getEncodableStringUTF8());
}
template <>
void UniqueID64::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 8;
}

std::string UniqueID64::toString() const {
  return fmt::format(fmt("{}"), *this);
}

/** PAK 128-bit Unique ID */
template <>
void UniqueID128::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  m_id.id[0] = reader.readUint64Big();
  m_id.id[1] = reader.readUint64Big();
}
template <>
void UniqueID128::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  writer.writeUint64Big(m_id.id[0]);
  writer.writeUint64Big(m_id.id[1]);
}
template <>
void UniqueID128::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader) {
  *this = UniqueIDBridge::MakePathFromString<UniqueID128>(reader.readString());
}
template <>
void UniqueID128::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer) {
  if (!isValid())
    return;
  hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
  if (!path)
    return;
  writer.writeString(path.getEncodableStringUTF8());
}
template <>
void UniqueID128::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 16;
}

std::string UniqueID128::toString() const {
  return fmt::format(fmt("{}"), *this);
}

/** Word Bitmap reader/writer */
void WordBitmap::read(athena::io::IStreamReader& reader, size_t bitCount) {
  m_bitCount = bitCount;
  size_t wordCount = (bitCount + 31) / 32;
  m_words.clear();
  m_words.reserve(wordCount);
  for (size_t w = 0; w < wordCount; ++w)
    m_words.push_back(reader.readUint32Big());
}
void WordBitmap::write(athena::io::IStreamWriter& writer) const {
  for (atUint32 word : m_words)
    writer.writeUint32Big(word);
}
void WordBitmap::binarySize(size_t& __isz) const { __isz += m_words.size() * 4; }

hecl::ProjectPath GetPathBeginsWith(const hecl::DirectoryEnumerator& dEnum, const hecl::ProjectPath& parentPath,
                                    hecl::SystemStringView test) {
  for (const auto& ent : dEnum)
    if (hecl::StringUtils::BeginsWith(ent.m_name, test))
      return hecl::ProjectPath(parentPath, ent.m_name);
  return {};
}

} // namespace DataSpec
