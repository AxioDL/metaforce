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

/* This comes up a great deal */
typedef Athena::io::DNA<Athena::BigEndian> BigDNA;
typedef Athena::io::DNAYaml<Athena::BigEndian> BigYAML;

/* FourCC with DNA read/write */
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
    inline void read(Athena::io::IStreamReader& reader)
    {reader.readUBytesToBuf(fcc, 4);}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUBytes((atUint8*)fcc, 4);}
    inline void fromYAML(Athena::io::YAMLDocReader& reader)
    {std::string rs = reader.readString(nullptr); strncpy(fcc, rs.c_str(), 4);}
    inline void toYAML(Athena::io::YAMLDocWriter& writer) const
    {writer.writeString(nullptr, std::string(fcc, 4));}
};

using FourCC = HECL::FourCC;

/* PAK 32-bit Unique ID */
class UniqueID32 : public BigYAML
{
    uint32_t m_id = 0xffffffff;
public:
    Delete expl;
    inline operator bool() const {return m_id != 0xffffffff;}
    inline void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint32Big();}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint32Big(m_id);}
    inline void fromYAML(Athena::io::YAMLDocReader& reader)
    {m_id = reader.readUint32(nullptr);}
    inline void toYAML(Athena::io::YAMLDocWriter& writer) const
    {writer.writeUint32(nullptr, m_id);}

    inline bool operator!=(const UniqueID32& other) const {return m_id != other.m_id;}
    inline bool operator==(const UniqueID32& other) const {return m_id == other.m_id;}
    inline uint32_t toUint32() const {return m_id;}
    inline std::string toString() const
    {
        char buf[9];
        snprintf(buf, 9, "%08X", m_id);
        return std::string(buf);
    }
};

/* PAK 64-bit Unique ID */
class UniqueID64 : public BigDNA
{
    uint64_t m_id = 0xffffffffffffffff;
public:
    Delete expl;
    inline operator bool() const {return m_id != 0xffffffffffffffff;}
    inline void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint64Big();}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint64Big(m_id);}

    inline bool operator!=(const UniqueID64& other) const {return m_id != other.m_id;}
    inline bool operator==(const UniqueID64& other) const {return m_id == other.m_id;}
    inline uint64_t toUint64() const {return m_id;}
    inline std::string toString() const
    {
        char buf[17];
        snprintf(buf, 17, "%016" PRIX64, m_id);
        return std::string(buf);
    }
};

/* PAK 128-bit Unique ID */
class UniqueID128 : public BigDNA
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
    inline operator bool() const
    {return m_id[0] != 0xffffffffffffffff && m_id[1] != 0xffffffffffffffff;}
    inline void read(Athena::io::IStreamReader& reader)
    {
        m_id[0] = reader.readUint64Big();
        m_id[1] = reader.readUint64Big();
    }
    inline void write(Athena::io::IStreamWriter& writer) const
    {
        writer.writeUint64Big(m_id[0]);
        writer.writeUint64Big(m_id[1]);
    }

    inline bool operator!=(const UniqueID128& other) const
    {
#if __SSE__
        __m128i vcmp = _mm_cmpeq_epi32(m_id128, other.m_id128);
        int vmask = _mm_movemask_epi8(vcmp);
        return vmask != 0xffff;
#else
        return (m_id[0] != other.m_id[0]) || (m_id[1] != other.m_id[1]);
#endif
    }
    inline bool operator==(const UniqueID128& other) const
    {
#if __SSE__
        __m128i vcmp = _mm_cmpeq_epi32(m_id128, other.m_id128);
        int vmask = _mm_movemask_epi8(vcmp);
        return vmask == 0xffff;
#else
        return (m_id[0] == other.m_id[0]) && (m_id[1] == other.m_id[1]);
#endif
    }
    inline uint64_t toHighUint64() const {return m_id[0];}
    inline uint64_t toLowUint64() const {return m_id[1];}
    inline std::string toString() const
    {
        char buf[33];
        snprintf(buf, 33, "%016" PRIX64 "%016" PRIX64, m_id[0], m_id[1]);
        return std::string(buf);
    }
};

/* Case-insensitive comparator for std::map sorting */
struct CaseInsensitiveCompare
{
    inline bool operator()(const std::string& lhs, const std::string& rhs) const
    {
#if _WIN32
        if (_stricmp(lhs.c_str(), rhs.c_str()) < 0)
#else
        if (strcasecmp(lhs.c_str(), rhs.c_str()) < 0)
#endif
            return true;
        return false;
    }

#if _WIN32
    inline bool operator()(const std::wstring& lhs, const std::wstring& rhs) const
    {
        if (_wcsicmp(lhs.c_str(), rhs.c_str()) < 0)
            return true;
        return false;
    }
#endif
};

/* Word Bitmap reader/writer */
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
    void clear()
    {
        m_words.clear();
    }

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

/* Resource cooker function */
typedef std::function<bool(const HECL::ProjectPath&, const HECL::ProjectPath&)> ResCooker;

}

/* Hash template-specializations for UniqueID types */
namespace std
{
template<>
struct hash<Retro::DNAFourCC>
{
    inline size_t operator()(const Retro::DNAFourCC& fcc) const
    {return fcc.toUint32();}
};

template<>
struct hash<Retro::UniqueID32>
{
    inline size_t operator()(const Retro::UniqueID32& id) const
    {return id.toUint32();}
};

template<>
struct hash<Retro::UniqueID64>
{
    inline size_t operator()(const Retro::UniqueID64& id) const
    {return id.toUint64();}
};

template<>
struct hash<Retro::UniqueID128>
{
    inline size_t operator()(const Retro::UniqueID128& id) const
    {return id.toHighUint64() ^ id.toLowUint64();}
};
}

#endif // __DNA_COMMON_HPP__
