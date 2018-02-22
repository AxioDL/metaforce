#include "DNACommon.hpp"
#include "PAK.hpp"
#include "boo/ThreadLocalPtr.hpp"

namespace DataSpec
{

logvisor::Module LogDNACommon("urde::DNACommon");
ThreadLocalPtr<SpecBase> g_curSpec;
ThreadLocalPtr<PAKRouterBase> g_PakRouter;
ThreadLocalPtr<hecl::blender::Token> g_ThreadBlenderToken;
ThreadLocalPtr<hecl::Database::Project> UniqueIDBridge::s_Project;
UniqueID32 UniqueID32::kInvalidId;

template <class IDType>
hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const IDType& id, bool silenceWarnings)
{
    /* Try PAKRouter first (only available at extract) */
    PAKRouterBase* pakRouter = g_PakRouter.get();
    if (pakRouter)
    {
        hecl::ProjectPath path = pakRouter->getWorking(id, silenceWarnings);
        if (path)
            return path;
    }

    /* Try project cache second (populated with paths read from YAML resources) */
    hecl::Database::Project* project = s_Project.get();
    if (!project)
    {
        if (pakRouter)
        {
            if (hecl::VerbosityLevel >= 1 && !silenceWarnings && id)
                LogDNACommon.report(logvisor::Warning,
                                    "unable to translate %s to path", id.toString().c_str());
            return {};
        }
        LogDNACommon.report(logvisor::Fatal,
        "g_PakRouter or s_Project must be set to non-null before "
        "calling UniqueIDBridge::TranslatePakIdToPath");
    }

    const hecl::ProjectPath* search = project->lookupBridgePath(id.toUint64());
    if (!search)
    {
        if (hecl::VerbosityLevel >= 1 && !silenceWarnings && id)
            LogDNACommon.report(logvisor::Warning,
                                "unable to translate %s to path", id.toString().c_str());
        return {};
    }
    return *search;
}
template
hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const UniqueID32& id, bool silenceWarnings);
template
hecl::ProjectPath UniqueIDBridge::TranslatePakIdToPath(const UniqueID64& id, bool silenceWarnings);

template <class IDType>
hecl::ProjectPath UniqueIDBridge::MakePathFromString(std::string_view str)
{
    if (str.empty())
        return {};
    hecl::Database::Project* project = s_Project.get();
    if (!project)
        LogDNACommon.report(logvisor::Fatal,
                            "UniqueIDBridge::setGlobalProject must be called before MakePathFromString");
    hecl::ProjectPath path = hecl::ProjectPath(*project, str);
    project->addBridgePathToCache(IDType(path).toUint64(), path);
    return path;
}
template
hecl::ProjectPath UniqueIDBridge::MakePathFromString<UniqueID32>(std::string_view str);
template
hecl::ProjectPath UniqueIDBridge::MakePathFromString<UniqueID64>(std::string_view str);

template <class IDType>
void UniqueIDBridge::TransformOldHashToNewHash(IDType& id)
{
    id = TranslatePakIdToPath(id);
}
template
void UniqueIDBridge::TransformOldHashToNewHash(UniqueID32& id);
template
void UniqueIDBridge::TransformOldHashToNewHash(UniqueID64& id);

void UniqueIDBridge::setThreadProject(hecl::Database::Project& project)
{
    s_Project.reset(&project);
}

/** PAK 32-bit Unique ID */
template <>
void UniqueID32::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{assign(reader.readUint32Big());}
template <>
void UniqueID32::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{writer.writeUint32Big(m_id);}
template <>
void UniqueID32::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader)
{
    *this = UniqueIDBridge::MakePathFromString<UniqueID32>(reader.readString(nullptr));
}
template <>
void UniqueID32::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer)
{
    if (!operator bool())
        return;
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
    if (!path)
        return;
    writer.writeString(nullptr, path.getAuxInfo().size() ?
        (std::string(path.getRelativePathUTF8()) + '|' + path.getAuxInfoUTF8().data()) :
         path.getRelativePathUTF8());
}
template <>
void UniqueID32::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s)
{s += 4;}

std::string UniqueID32::toString() const
{
    char buf[9];
    snprintf(buf, 9, "%08X", m_id);
    return std::string(buf);
}

AuxiliaryID32& AuxiliaryID32::operator=(const hecl::ProjectPath& path)
{
    assign(path.ensureAuxInfo(m_auxStr).hash().val32());
    return *this;
}

AuxiliaryID32& AuxiliaryID32::operator=(const UniqueID32& id)
{
    m_baseId = id;
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id);
    if (path)
    {
        if (m_addExtension)
            path = path.getWithExtension(m_addExtension);
        *this = path;
    }
    return *this;
}

template <>
void AuxiliaryID32::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{
    assign(reader.readUint32Big());
    m_baseId = *this;
}
template <>
void AuxiliaryID32::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{
    writer.writeUint32Big(m_id);
}
template <>
void AuxiliaryID32::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader)
{
    hecl::ProjectPath readPath = UniqueIDBridge::MakePathFromString<UniqueID32>(reader.readString(nullptr));
    *this = readPath.ensureAuxInfo(m_auxStr);
}
template <>
void AuxiliaryID32::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer)
{
    if (!operator bool())
        return;
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this, true);
    if (!path)
        path = UniqueIDBridge::TranslatePakIdToPath(m_baseId);
    if (!path)
        return;
    if (m_addExtension)
        path = path.getWithExtension(m_addExtension);
    hecl::SystemUTF8Conv ufx8AuxStr(m_auxStr);
    writer.writeString(nullptr, std::string(path.getRelativePathUTF8()) + '|' + ufx8AuxStr);
}


/** PAK 64-bit Unique ID */
template <>
void UniqueID64::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{assign(reader.readUint64Big());}
template <>
void UniqueID64::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{writer.writeUint64Big(m_id);}
template <>
void UniqueID64::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader)
{
    *this = UniqueIDBridge::MakePathFromString<UniqueID64>(reader.readString(nullptr));
}
template <>
void UniqueID64::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer)
{
    if (!operator bool())
        return;
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
    if (!path)
        return;
    writer.writeString(nullptr, path.getAuxInfo().size() ?
                                (std::string(path.getRelativePathUTF8()) + '|' + path.getAuxInfoUTF8().data()) :
                                path.getRelativePathUTF8());
}
template <>
void UniqueID64::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s)
{s += 8;}

std::string UniqueID64::toString() const
{
    char buf[17];
    snprintf(buf, 17, "%016" PRIX64, m_id);
    return std::string(buf);
}

/** PAK 128-bit Unique ID */
template <>
void UniqueID128::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{
    m_id[0] = reader.readUint64Big();
    m_id[1] = reader.readUint64Big();
}
template <>
void UniqueID128::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{
    writer.writeUint64Big(m_id[0]);
    writer.writeUint64Big(m_id[1]);
}
template <>
void UniqueID128::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader)
{
    *this = UniqueIDBridge::MakePathFromString<UniqueID128>(reader.readString(nullptr));
}
template <>
void UniqueID128::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer)
{
    if (!operator bool())
        return;
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
    if (!path)
        return;
    writer.writeString(nullptr, path.getAuxInfo().size() ?
                                (std::string(path.getRelativePathUTF8()) + '|' + path.getAuxInfoUTF8().data()) :
                                path.getRelativePathUTF8());
}
template <>
void UniqueID128::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s)
{s += 16;}

std::string UniqueID128::toString() const
{
    char buf[33];
    snprintf(buf, 33, "%016" PRIX64 "%016" PRIX64, m_id[0], m_id[1]);
    return std::string(buf);
}


/** Word Bitmap reader/writer */
void WordBitmap::read(athena::io::IStreamReader& reader, size_t bitCount)
{
    m_bitCount = bitCount;
    size_t wordCount = (bitCount + 31) / 32;
    m_words.clear();
    m_words.reserve(wordCount);
    for (size_t w=0 ; w<wordCount ; ++w)
        m_words.push_back(reader.readUint32Big());
}
void WordBitmap::write(athena::io::IStreamWriter& writer) const
{
    for (atUint32 word : m_words)
        writer.writeUint32Big(word);
}
size_t WordBitmap::binarySize(size_t __isz) const
{
    return __isz + m_words.size() * 4;
}

}
