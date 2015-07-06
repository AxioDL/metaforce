#ifndef __DNA_COMMON_HPP__
#define __DNA_COMMON_HPP__

#include <Athena/DNA.hpp>
#include "HECL/HECL.hpp"

namespace Retro
{

/* This comes up a great deal */
typedef Athena::io::DNA<Athena::BigEndian> BigDNA;

/* PAK 32-bit Unique ID */
class UniqueID32 : public BigDNA
{
    uint32_t m_id;
public:
    Delete expl;
    inline void read(Athena::io::IStreamReader& reader)
    {reader.readUint32();}
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
    {reader.readUint64();}
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

}

/* Hash template-specializations for UniqueID types */
namespace std
{
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
