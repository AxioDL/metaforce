#ifndef __DNA_COMMON_HPP__
#define __DNA_COMMON_HPP__

#include <Athena/DNA.hpp>
#include "HECL/HECL.hpp"
#include "HECL/Database.hpp"

namespace Retro
{

extern LogVisor::LogModule LogDNACommon;

/* This comes up a great deal */
typedef Athena::io::DNA<Athena::BigEndian> BigDNA;

/* FourCC with DNA read/write */
class FourCC final : public BigDNA, public HECL::FourCC
{
public:
    FourCC() : HECL::FourCC() {}
    FourCC(const HECL::FourCC& other)
    : HECL::FourCC() {num = other.toUint32();}
    FourCC(const char* name)
    : HECL::FourCC(name) {}

    Delete expl;
    inline void read(Athena::io::IStreamReader& reader)
    {reader.readUBytesToBuf(fcc, 4);}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUBytes((atUint8*)fcc, 4);}
};

/* PAK 32-bit Unique ID */
class UniqueID32 : public BigDNA
{
    uint32_t m_id;
public:
    Delete expl;
    inline void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint32();}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint32(m_id);}

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
    uint64_t m_id;
public:
    Delete expl;
    inline void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint64();}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint64(m_id);}

    inline bool operator!=(const UniqueID64& other) const {return m_id != other.m_id;}
    inline bool operator==(const UniqueID64& other) const {return m_id == other.m_id;}
    inline uint64_t toUint64() const {return m_id;}
    inline std::string toString() const
    {
        char buf[17];
        snprintf(buf, 17, "%16lX", m_id);
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
    inline void read(Athena::io::IStreamReader& reader)
    {
        m_id[0] = reader.readUint64();
        m_id[1] = reader.readUint64();
    }
    inline void write(Athena::io::IStreamWriter& writer) const
    {
        writer.writeUint64(m_id[0]);
        writer.writeUint64(m_id[1]);
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
        snprintf(buf, 33, "%16lX%16lX", m_id[0], m_id[1]);
        return std::string(buf);
    }
};

/* Case-insensitive comparator for std::map sorting */
struct CaseInsensitiveCompare
{
    inline bool operator()(const std::string& lhs, const std::string& rhs) const
    {
#if _WIN32
        if (stricmp(lhs.c_str(), rhs.c_str()) < 0)
#else
        if (strcasecmp(lhs.c_str(), rhs.c_str()) < 0)
#endif
            return true;
        return false;
    }
};

/* PAK entry stream reader */
class PAKEntryReadStream : public Athena::io::IStreamReader
{
    std::unique_ptr<atUint8[]> m_buf;
    atUint64 m_sz;
    atUint64 m_pos;
public:
    PAKEntryReadStream() {}
    operator bool() const {return m_buf.operator bool();}
    PAKEntryReadStream(const PAKEntryReadStream& other) = delete;
    PAKEntryReadStream(PAKEntryReadStream&& other) = default;
    PAKEntryReadStream& operator=(const PAKEntryReadStream& other) = delete;
    PAKEntryReadStream& operator=(PAKEntryReadStream&& other) = default;
    PAKEntryReadStream(std::unique_ptr<atUint8[]>&& buf, atUint64 sz, atUint64 pos)
    : m_buf(std::move(buf)), m_sz(sz), m_pos(pos)
    {
        if (m_pos >= m_sz)
            LogDNACommon.report(LogVisor::FatalError, "PAK stream cursor overrun");
    }
    inline void seek(atInt64 pos, Athena::SeekOrigin origin)
    {
        if (origin == Athena::Begin)
            m_pos = pos;
        else if (origin == Athena::Current)
            m_pos += pos;
        else if (origin == Athena::End)
            m_pos = m_sz + pos;
        if (m_pos >= m_sz)
            LogDNACommon.report(LogVisor::FatalError, "PAK stream cursor overrun");
    }
    inline atUint64 position() const {return m_pos;}
    inline atUint64 length() const {return m_sz;}
    inline const atUint8* data() const {return m_buf.get();}
    inline atUint64 readUBytesToBuf(void* buf, atUint64 len)
    {
        atUint64 bufEnd = m_pos + len;
        if (bufEnd > m_sz)
            len -= bufEnd - m_sz;
        memcpy(buf, m_buf.get() + m_pos, len);
        m_pos += len;
        return len;
    }
};

/* Resource extractor type */
typedef struct
{
    std::function<bool(PAKEntryReadStream&, const HECL::ProjectPath&)> func;
    const char* fileExt;
} ResExtractor;

/* Resource cooker function */
typedef std::function<bool(const HECL::ProjectPath&, const HECL::ProjectPath&)> ResCooker;

/* Language-identifiers */
extern const HECL::FourCC ENGL;
extern const HECL::FourCC FREN;
extern const HECL::FourCC GERM;
extern const HECL::FourCC SPAN;
extern const HECL::FourCC ITAL;
extern const HECL::FourCC JAPN;

/* Resource types */
extern const HECL::FourCC AFSM;
extern const HECL::FourCC AGSC;
extern const HECL::FourCC ANCS;
extern const HECL::FourCC ANIM;
extern const HECL::FourCC ATBL;
extern const HECL::FourCC CINF;
extern const HECL::FourCC CMDL;
extern const HECL::FourCC CRSC;
extern const HECL::FourCC CSKR;
extern const HECL::FourCC CSMP;
extern const HECL::FourCC CSNG;
extern const HECL::FourCC CTWK;
extern const HECL::FourCC DGRP;
extern const HECL::FourCC DPSC;
extern const HECL::FourCC DUMB;
extern const HECL::FourCC ELSC;
extern const HECL::FourCC EVNT;
extern const HECL::FourCC FONT;
extern const HECL::FourCC FRME;
extern const HECL::FourCC HINT;
extern const HECL::FourCC MAPA;
extern const HECL::FourCC MAPU;
extern const HECL::FourCC MAPW;
extern const HECL::FourCC MLVL;
extern const HECL::FourCC MREA;
extern const HECL::FourCC PART;
extern const HECL::FourCC PATH;
extern const HECL::FourCC RFRM;
extern const HECL::FourCC ROOM;
extern const HECL::FourCC SAVW;
extern const HECL::FourCC SCAN;
extern const HECL::FourCC STRG;
extern const HECL::FourCC SWHC;
extern const HECL::FourCC TXTR;
extern const HECL::FourCC WPSC;

}

/* Hash template-specializations for UniqueID types */
namespace std
{
template<>
struct hash<Retro::FourCC>
{
    inline size_t operator()(const Retro::FourCC& fcc) const
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
