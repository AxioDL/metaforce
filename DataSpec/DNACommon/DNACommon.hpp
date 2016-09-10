#ifndef __DNA_COMMON_HPP__
#define __DNA_COMMON_HPP__

#include <stdio.h>
#include <athena/DNAYaml.hpp>
#include <athena/FileReader.hpp>
#include <athena/FileWriter.hpp>
#include <nod/DiscBase.hpp>
#include "hecl/hecl.hpp"
#include "hecl/Database.hpp"
#include "../SpecBase.hpp"
#include "boo/ThreadLocalPtr.hpp"

namespace DataSpec
{

extern logvisor::Module LogDNACommon;
extern ThreadLocalPtr<SpecBase> g_curSpec;
extern ThreadLocalPtr<class PAKRouterBase> g_PakRouter;

/* This comes up a great deal */
typedef athena::io::DNA<athena::BigEndian> BigDNA;
typedef athena::io::DNAYaml<athena::BigEndian> BigYAML;

/** FourCC with DNA read/write */
class DNAFourCC final : public BigYAML, public hecl::FourCC
{
public:
    DNAFourCC() : hecl::FourCC() {}
    DNAFourCC(const hecl::FourCC& other)
    : hecl::FourCC() {num = other.toUint32();}
    DNAFourCC(const char* name)
    : hecl::FourCC(name) {}
    DNAFourCC(uint32_t n)
    : hecl::FourCC(n) {}

    Delete expl;
    void read(athena::io::IStreamReader& reader)
    {reader.readUBytesToBuf(fcc, 4);}
    void write(athena::io::IStreamWriter& writer) const
    {writer.writeUBytes((atUint8*)fcc, 4);}
    void read(athena::io::YAMLDocReader& reader)
    {std::string rs = reader.readString(nullptr); strncpy(fcc, rs.c_str(), 4);}
    void write(athena::io::YAMLDocWriter& writer) const
    {writer.writeString(nullptr, std::string(fcc, 4));}
    size_t binarySize(size_t __isz) const
    {return __isz + 4;}
};

using FourCC = hecl::FourCC;
class UniqueID32;
class UniqueID64;
class UniqueID128;

/** Common virtual interface for runtime ambiguity resolution */
class PAKRouterBase
{
protected:
    const SpecBase& m_dataSpec;
public:
    PAKRouterBase(const SpecBase& dataSpec) : m_dataSpec(dataSpec) {}
    hecl::Database::Project& getProject() const {return m_dataSpec.getProject();}
    virtual hecl::ProjectPath getWorking(const UniqueID32&, bool silenceWarnings=false) const
    {
        LogDNACommon.report(logvisor::Fatal,
        "PAKRouter IDType mismatch; expected UniqueID32 specialization");
        return hecl::ProjectPath();
    }
    virtual hecl::ProjectPath getWorking(const UniqueID64&, bool silenceWarnings=false) const
    {
        LogDNACommon.report(logvisor::Fatal,
        "PAKRouter IDType mismatch; expected UniqueID64 specialization");
        return hecl::ProjectPath();
    }
    virtual hecl::ProjectPath getWorking(const UniqueID128&, bool silenceWarnings=false) const
    {
        LogDNACommon.report(logvisor::Fatal,
        "PAKRouter IDType mismatch; expected UniqueID128 specialization");
        return hecl::ProjectPath();
    }
};

/** Globally-accessed manager allowing UniqueID* classes to directly
 *  lookup destination paths of resources */
class UniqueIDBridge
{
    friend class UniqueID32;
    friend class UniqueID64;

    static ThreadLocalPtr<hecl::Database::Project> s_Project;
public:
    template <class IDType>
    static hecl::ProjectPath TranslatePakIdToPath(const IDType& id, bool silenceWarnings=false)
    {
        PAKRouterBase* pakRouter = g_PakRouter.get();
        if (!pakRouter)
            LogDNACommon.report(logvisor::Fatal,
            "g_PakRouter must be set to non-null before calling UniqueIDBridge::TranslatePakIdToPath");
        return pakRouter->getWorking(id, silenceWarnings);
    }
    static hecl::ProjectPath MakePathFromString(const std::string& str)
    {
        hecl::Database::Project* project = s_Project.get();
        if (!project)
            LogDNACommon.report(logvisor::Fatal,
                                "UniqueIDBridge::setGlobalProject must be called before MakePathFromString");
        return hecl::ProjectPath(*project, str);
    }
    template <class IDType>
    static void TransformOldHashToNewHash(IDType& id)
    {
        id = TranslatePakIdToPath(id);
    }

    static void setThreadProject(hecl::Database::Project& project)
    {
        s_Project.reset(&project);
    }
};

/** PAK 32-bit Unique ID */
class UniqueID32 : public BigYAML
{
protected:
    uint32_t m_id = 0xffffffff;
public:
    static UniqueID32 kInvalidId;
    Delete expl;
    operator bool() const {return m_id != 0xffffffff && m_id != 0;}
    void read(athena::io::IStreamReader& reader)
    {m_id = reader.readUint32Big();}
    void write(athena::io::IStreamWriter& writer) const
    {writer.writeUint32Big(m_id);}
    void read(athena::io::YAMLDocReader& reader)
    {
        *this = UniqueIDBridge::MakePathFromString(reader.readString(nullptr));
    }
    void write(athena::io::YAMLDocWriter& writer) const
    {
        if (!operator bool())
            return;
        hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
        if (!path)
            return;
        writer.writeString(nullptr, path.getRelativePathUTF8());
    }
    size_t binarySize(size_t __isz) const
    {return __isz + 4;}

    UniqueID32& operator=(const hecl::ProjectPath& path)
    {m_id = path.hash().val32(); return *this;}

    bool operator!=(const UniqueID32& other) const {return m_id != other.m_id;}
    bool operator==(const UniqueID32& other) const {return m_id == other.m_id;}
    uint32_t toUint32() const {return m_id;}
    uint64_t toUint64() const {return m_id;}
    std::string toString() const
    {
        char buf[9];
        snprintf(buf, 9, "%08X", m_id);
        return std::string(buf);
    }
    void clear() {m_id = 0xffffffff;}

    UniqueID32() = default;
    UniqueID32(athena::io::IStreamReader& reader) {read(reader);}
    UniqueID32(const hecl::ProjectPath& path) {*this = path;}
    UniqueID32(const char* hexStr)
    {
        char copy[9];
        strncpy(copy, hexStr, 8);
        copy[8] = '\0';
        m_id = strtoul(copy, nullptr, 16);
    }
    UniqueID32(const wchar_t* hexStr)
    {
        wchar_t copy[9];
        wcsncpy(copy, hexStr, 8);
        copy[8] = L'\0';
        m_id = wcstoul(copy, nullptr, 16);
    }

    static constexpr size_t BinarySize() {return 4;}
};

class AuxiliaryID32 : public UniqueID32
{
    const hecl::SystemChar* m_auxStr;
    const hecl::SystemChar* m_addExtension;
    UniqueID32 m_baseId;
public:
    AuxiliaryID32(const hecl::SystemChar* auxStr,
                  const hecl::SystemChar* addExtension=nullptr)
    : m_auxStr(auxStr), m_addExtension(addExtension) {}

    AuxiliaryID32& operator=(const hecl::ProjectPath& path)
    {
        m_id = path.ensureAuxInfo(m_auxStr).hash().val32();
        return *this;
    }

    AuxiliaryID32& operator=(const UniqueID32& id)
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

    void read(athena::io::IStreamReader& reader)
    {
        m_id = reader.readUint32Big();
        m_baseId = *this;
    }

    void write(athena::io::IStreamWriter& writer) const
    {
        writer.writeUint32Big(m_id);
    }

    void read(athena::io::YAMLDocReader& reader)
    {
        hecl::ProjectPath readPath = UniqueIDBridge::MakePathFromString(reader.readString(nullptr));
        *this = readPath.ensureAuxInfo(m_auxStr);
    }

    void write(athena::io::YAMLDocWriter& writer) const
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
        hecl::SystemUTF8View ufx8AuxStr(m_auxStr);
        writer.writeString(nullptr, path.getRelativePathUTF8() + '|' + ufx8AuxStr);
    }

    const UniqueID32& getBaseId() const {return m_baseId;}
};

/** PAK 64-bit Unique ID */
class UniqueID64 : public BigYAML
{
    uint64_t m_id = 0xffffffffffffffff;
public:
    Delete expl;
    operator bool() const {return m_id != 0xffffffffffffffff && m_id != 0;}
    void read(athena::io::IStreamReader& reader)
    {m_id = reader.readUint64Big();}
    void write(athena::io::IStreamWriter& writer) const
    {writer.writeUint64Big(m_id);}
    void read(athena::io::YAMLDocReader& reader)
    {
        *this = UniqueIDBridge::MakePathFromString(reader.readString(nullptr));
    }
    void write(athena::io::YAMLDocWriter& writer) const
    {
        if (!operator bool())
            return;
        hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
        if (!path)
            return;
        writer.writeString(nullptr, path.getRelativePathUTF8());
    }
    size_t binarySize(size_t __isz) const
    {return __isz + 8;}

    UniqueID64& operator=(const hecl::ProjectPath& path)
    {m_id = path.hash().val64(); return *this;}

    bool operator!=(const UniqueID64& other) const {return m_id != other.m_id;}
    bool operator==(const UniqueID64& other) const {return m_id == other.m_id;}
    uint64_t toUint64() const {return m_id;}
    std::string toString() const
    {
        char buf[17];
        snprintf(buf, 17, "%016" PRIX64, m_id);
        return std::string(buf);
    }
    void clear() {m_id = 0xffffffffffffffff;}

    UniqueID64() = default;
    UniqueID64(athena::io::IStreamReader& reader) {read(reader);}
    UniqueID64(const hecl::ProjectPath& path) {*this = path;}
    UniqueID64(const char* hexStr)
    {
        char copy[17];
        strncpy(copy, hexStr, 16);
        copy[16] = '\0';
#if _WIN32
        m_id = _strtoui64(copy, nullptr, 16);
#else
        m_id = strtouq(copy, nullptr, 16);
#endif
    }
    UniqueID64(const wchar_t* hexStr)
    {
        wchar_t copy[17];
        wcsncpy(copy, hexStr, 16);
        copy[16] = L'\0';
#if _WIN32
        m_id = _wcstoui64(copy, nullptr, 16);
#else
        m_id = wcstoull(copy, nullptr, 16);
#endif
    }

    static constexpr size_t BinarySize() {return 8;}
};

/** PAK 128-bit Unique ID */
class UniqueID128 : public BigYAML
{
    union
    {
        uint64_t m_id[2];
#if __SSE__
        __m128i m_id128;
#endif
    };
public:
    Delete expl;
    UniqueID128() {m_id[0]=0xffffffffffffffff; m_id[1]=0xffffffffffffffff;}
    operator bool() const
    {return m_id[0] != 0xffffffffffffffff && m_id[0] != 0 && m_id[1] != 0xffffffffffffffff && m_id[1] != 0;}
    void read(athena::io::IStreamReader& reader)
    {
        m_id[0] = reader.readUint64Big();
        m_id[1] = reader.readUint64Big();
    }
    void write(athena::io::IStreamWriter& writer) const
    {
        writer.writeUint64Big(m_id[0]);
        writer.writeUint64Big(m_id[1]);
    }
    void read(athena::io::YAMLDocReader& reader)
    {
        *this = UniqueIDBridge::MakePathFromString(reader.readString(nullptr));
    }
    void write(athena::io::YAMLDocWriter& writer) const
    {
        if (!operator bool())
            return;
        hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(*this);
        if (!path)
            return;
        writer.writeString(nullptr, path.getRelativePathUTF8());
    }
    size_t binarySize(size_t __isz) const
    {return __isz + 16;}

    UniqueID128& operator=(const hecl::ProjectPath& path)
    {
        m_id[0] = path.hash().val64();
        m_id[1] = 0;
        return *this;
    }

    bool operator!=(const UniqueID128& other) const
    {
#if __SSE__
        __m128i vcmp = _mm_cmpeq_epi32(m_id128, other.m_id128);
        int vmask = _mm_movemask_epi8(vcmp);
        return vmask != 0xffff;
#else
        return (m_id[0] != other.m_id[0]) || (m_id[1] != other.m_id[1]);
#endif
    }
    bool operator==(const UniqueID128& other) const
    {
#if __SSE__
        __m128i vcmp = _mm_cmpeq_epi32(m_id128, other.m_id128);
        int vmask = _mm_movemask_epi8(vcmp);
        return vmask == 0xffff;
#else
        return (m_id[0] == other.m_id[0]) && (m_id[1] == other.m_id[1]);
#endif
    }
    void clear() {m_id[0] = 0xffffffffffffffff; m_id[1] = 0xffffffffffffffff;}
    uint64_t toHighUint64() const {return m_id[0];}
    uint64_t toLowUint64() const {return m_id[1];}
    std::string toString() const
    {
        char buf[33];
        snprintf(buf, 33, "%016" PRIX64 "%016" PRIX64, m_id[0], m_id[1]);
        return std::string(buf);
    }

    static constexpr size_t BinarySize() {return 16;}
};

/** Word Bitmap reader/writer */
class WordBitmap
{
    std::vector<atUint32> m_words;
    size_t m_bitCount = 0;
public:
    void read(athena::io::IStreamReader& reader, size_t bitCount)
    {
        m_bitCount = bitCount;
        size_t wordCount = (bitCount + 31) / 32;
        m_words.clear();
        m_words.reserve(wordCount);
        for (size_t w=0 ; w<wordCount ; ++w)
            m_words.push_back(reader.readUint32Big());
    }
    void write(athena::io::IStreamWriter& writer) const
    {
        for (atUint32 word : m_words)
            writer.writeUint32Big(word);
    }
    size_t binarySize(size_t __isz) const
    {
        return __isz + m_words.size() * 4;
    }
    size_t getBitCount() const {return m_bitCount;}
    bool getBit(size_t idx) const
    {
        size_t wordIdx = idx / 32;
        if (wordIdx >= m_words.size())
            return false;
        size_t wordCur = idx % 32;
        return (m_words[wordIdx] >> wordCur) & 0x1;
    }
    void setBit(size_t idx)
    {
        size_t wordIdx = idx / 32;
        while (wordIdx >= m_words.size())
            m_words.push_back(0);
        size_t wordCur = idx % 32;
        m_words[wordIdx] |= (1 << wordCur);
    }
    void unsetBit(size_t idx)
    {
        size_t wordIdx = idx / 32;
        while (wordIdx >= m_words.size())
            m_words.push_back(0);
        size_t wordCur = idx % 32;
        m_words[wordIdx] &= ~(1 << wordCur);
    }
    void clear() {m_words.clear();}

    class Iterator : public std::iterator<std::forward_iterator_tag, bool>
    {
        friend class WordBitmap;
        const WordBitmap& m_bmp;
        size_t m_idx = 0;
        Iterator(const WordBitmap& bmp, size_t idx) : m_bmp(bmp), m_idx(idx) {}
    public:
        Iterator& operator++() {++m_idx; return *this;}
        bool operator*() {return m_bmp.getBit(m_idx);}
        bool operator!=(const Iterator& other) const {return m_idx != other.m_idx;}
    };
    Iterator begin() const {return Iterator(*this, 0);}
    Iterator end() const {return Iterator(*this, m_bitCount);}
};

/** Resource cooker function */
typedef std::function<bool(const hecl::ProjectPath&, const hecl::ProjectPath&)> ResCooker;

}

/* Hash template-specializations for UniqueID types */
namespace std
{
template<>
struct hash<DataSpec::DNAFourCC>
{
    size_t operator()(const DataSpec::DNAFourCC& fcc) const
    {return fcc.toUint32();}
};

template<>
struct hash<DataSpec::UniqueID32>
{
    size_t operator()(const DataSpec::UniqueID32& id) const
    {return id.toUint32();}
};

template<>
struct hash<DataSpec::UniqueID64>
{
    size_t operator()(const DataSpec::UniqueID64& id) const
    {return id.toUint64();}
};

template<>
struct hash<DataSpec::UniqueID128>
{
    size_t operator()(const DataSpec::UniqueID128& id) const
    {return id.toHighUint64() ^ id.toLowUint64();}
};
}

#endif // __DNA_COMMON_HPP__
