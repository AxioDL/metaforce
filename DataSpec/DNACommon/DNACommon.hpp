#ifndef __DNA_COMMON_HPP__
#define __DNA_COMMON_HPP__

#include <stdio.h>
#include <Athena/DNAYaml.hpp>
#include <NOD/DiscBase.hpp>
#include "HECL/HECL.hpp"
#include "HECL/Database.hpp"
#include "../SpecBase.hpp"

namespace Retro
{

extern LogVisor::LogModule LogDNACommon;
extern SpecBase* g_curSpec;

/* This comes up a great deal */
typedef Athena::io::DNA<Athena::BigEndian> BigDNA;
typedef Athena::io::DNAYaml<Athena::BigEndian> BigYAML;

/** FourCC with DNA read/write */
class DNAFourCC final : public BigYAML, public HECL::FourCC
{
public:
    DNAFourCC() : HECL::FourCC() {}
    DNAFourCC(const HECL::FourCC& other)
    : HECL::FourCC() {num = other.toUint32();}
    DNAFourCC(const char* name)
    : HECL::FourCC(name) {}
    DNAFourCC(uint32_t n)
    : HECL::FourCC(n) {}

    Delete expl;
    void read(Athena::io::IStreamReader& reader)
    {reader.readUBytesToBuf(fcc, 4);}
    void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUBytes((atUint8*)fcc, 4);}
    void read(Athena::io::YAMLDocReader& reader)
    {std::string rs = reader.readString(nullptr); strncpy(fcc, rs.c_str(), 4);}
    void write(Athena::io::YAMLDocWriter& writer) const
    {writer.writeString(nullptr, std::string(fcc, 4));}
    size_t binarySize(size_t __isz) const
    {return __isz + 4;}
};

using FourCC = HECL::FourCC;

/** PAK 32-bit Unique ID */
class UniqueID32 : public BigYAML
{
    uint32_t m_id = 0xffffffff;
public:
    Delete expl;
    operator bool() const {return m_id != 0xffffffff && m_id != 0;}
    void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint32Big();}
    void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint32Big(m_id);}
    void read(Athena::io::YAMLDocReader& reader)
    {m_id = reader.readUint32(nullptr);}
    void write(Athena::io::YAMLDocWriter& writer) const
    {writer.writeUint32(nullptr, m_id);}
    size_t binarySize(size_t __isz) const
    {return __isz + 4;}

    UniqueID32& operator=(const HECL::ProjectPath& path)
    {m_id = path.hash().val32(); return *this;}

    bool operator!=(const UniqueID32& other) const {return m_id != other.m_id;}
    bool operator==(const UniqueID32& other) const {return m_id == other.m_id;}
    uint32_t toUint32() const {return m_id;}
    std::string toString() const
    {
        char buf[9];
        snprintf(buf, 9, "%08X", m_id);
        return std::string(buf);
    }
    void clear() {m_id = 0xffffffff;}

    UniqueID32() = default;
    UniqueID32(Athena::io::IStreamReader& reader) {read(reader);}
    UniqueID32(const HECL::ProjectPath& path) {*this = path;}
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

/** PAK 64-bit Unique ID */
class UniqueID64 : public BigYAML
{
    uint64_t m_id = 0xffffffffffffffff;
public:
    Delete expl;
    operator bool() const {return m_id != 0xffffffffffffffff && m_id != 0;}
    void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint64Big();}
    void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint64Big(m_id);}
    void read(Athena::io::YAMLDocReader& reader)
    {m_id = reader.readUint64(nullptr);}
    void write(Athena::io::YAMLDocWriter& writer) const
    {writer.writeUint64(nullptr, m_id);}
    size_t binarySize(size_t __isz) const
    {return __isz + 8;}

    UniqueID64& operator=(const HECL::ProjectPath& path)
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
    UniqueID64(Athena::io::IStreamReader& reader) {read(reader);}
    UniqueID64(const HECL::ProjectPath& path) {*this = path;}
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
    void read(Athena::io::IStreamReader& reader)
    {
        m_id[0] = reader.readUint64Big();
        m_id[1] = reader.readUint64Big();
    }
    void write(Athena::io::IStreamWriter& writer) const
    {
        writer.writeUint64Big(m_id[0]);
        writer.writeUint64Big(m_id[1]);
    }
    void read(Athena::io::YAMLDocReader& reader)
    {
        std::string str = reader.readString(nullptr);
        while (str.size() < 32)
            str += '0';
        std::string hStr(str.begin(), str.begin() + 16);
        std::string lStr(str.begin() + 16, str.begin() + 32);
        m_id[0] = strtoull(hStr.c_str(), nullptr, 16);
        m_id[1] = strtoull(lStr.c_str(), nullptr, 16);
    }
    void write(Athena::io::YAMLDocWriter& writer) const
    {
        writer.writeString(nullptr, toString().c_str());
    }
    size_t binarySize(size_t __isz) const
    {return __isz + 16;}

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

/** Class that automatically converts between hash and path for DNA usage */
template <class IDTYPE>
class PAKPath : public BigYAML
{
    HECL::ProjectPath m_path;
    IDTYPE m_id;
public:
    HECL::ProjectPath getPath() const
    {
        if (m_path)
            return m_path;
        if (!g_curSpec)
            LogDNACommon.report(LogVisor::FatalError, "current DataSpec not set for PAKPath");
        if (m_id)
            return g_curSpec->getWorking(m_id);
        return HECL::ProjectPath();
    }
    operator HECL::ProjectPath() const {return getPath();}
    operator const IDTYPE&() const {return m_id;}

    Delete _d;
    void read(Athena::io::IStreamReader& reader)
    {m_id.read(reader);}
    void write(Athena::io::IStreamWriter& writer) const
    {m_id.write(writer);}
    void read(Athena::io::YAMLDocReader& reader)
    {
        if (!g_curSpec)
            LogDNACommon.report(LogVisor::FatalError, "current DataSpec not set for PAKPath");
        std::string path = reader.readString(nullptr);
        if (path.empty())
        {
            m_path.clear();
            m_id.clear();
            return;
        }
        m_path.assign(g_curSpec->getProject(), path);
        m_id = m_path;
    }
    void write(Athena::io::YAMLDocWriter& writer) const
    {
        if (m_path)
        {
            writer.writeString(nullptr, m_path.getRelativePathUTF8());
            return;
        }
        writer.writeString(nullptr, getPath().getRelativePathUTF8());
    }

    size_t binarySize(size_t __isz) const
    {return __isz + IDTYPE::BinarySize();}
};
using PAKPath32 = PAKPath<UniqueID32>;
using PAKPath64 = PAKPath<UniqueID64>;

/** Word Bitmap reader/writer */
class WordBitmap
{
    std::vector<atUint32> m_words;
    size_t m_bitCount = 0;
public:
    void read(Athena::io::IStreamReader& reader, size_t bitCount)
    {
        m_bitCount = bitCount;
        size_t wordCount = (bitCount + 31) / 32;
        m_words.clear();
        m_words.reserve(wordCount);
        for (size_t w=0 ; w<wordCount ; ++w)
            m_words.push_back(reader.readUint32Big());
    }
    void write(Athena::io::IStreamWriter& writer) const
    {
        for (atUint32 word : m_words)
            writer.writeUint32(word);
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
typedef std::function<bool(const HECL::ProjectPath&, const HECL::ProjectPath&)> ResCooker;

}

/* Hash template-specializations for UniqueID types */
namespace std
{
template<>
struct hash<Retro::DNAFourCC>
{
    size_t operator()(const Retro::DNAFourCC& fcc) const
    {return fcc.toUint32();}
};

template<>
struct hash<Retro::UniqueID32>
{
    size_t operator()(const Retro::UniqueID32& id) const
    {return id.toUint32();}
};

template<>
struct hash<Retro::UniqueID64>
{
    size_t operator()(const Retro::UniqueID64& id) const
    {return id.toUint64();}
};

template<>
struct hash<Retro::UniqueID128>
{
    size_t operator()(const Retro::UniqueID128& id) const
    {return id.toHighUint64() ^ id.toLowUint64();}
};
}

#endif // __DNA_COMMON_HPP__
