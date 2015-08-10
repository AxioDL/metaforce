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
class FourCC final : public BigYAML, public HECL::FourCC
{
public:
    FourCC() : HECL::FourCC() {}
    FourCC(const HECL::FourCC& other)
    : HECL::FourCC() {num = other.toUint32();}
    FourCC(const char* name)
    : HECL::FourCC(name) {}
    FourCC(uint32_t n)
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

/* PAK 32-bit Unique ID */
class UniqueID32 : public BigYAML
{
    uint32_t m_id = 0;
public:
    Delete expl;
    inline operator bool() const {return m_id;}
    inline void read(Athena::io::IStreamReader& reader)
    {m_id = reader.readUint32();}
    inline void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUint32(m_id);}
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
    uint64_t m_id = 0;
public:
    Delete expl;
    inline operator bool() const {return m_id;}
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
        snprintf(buf, 17, "%016lX", m_id);
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
    UniqueID128() {m_id[0]=0; m_id[1]=0;}
    inline operator bool() const {return m_id[0] && m_id[1];}
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
        snprintf(buf, 33, "%016lX%016lX", m_id[0], m_id[1]);
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

struct UniqueResult
{
    enum Type
    {
        UNIQUE_NOTFOUND,
        UNIQUE_LEVEL,
        UNIQUE_AREA,
        UNIQUE_LAYER
    } type = UNIQUE_NOTFOUND;
    const HECL::SystemString* areaName = nullptr;
    const HECL::SystemString* layerName = nullptr;
    UniqueResult() = default;
    UniqueResult(Type tp) : type(tp) {}
    inline HECL::ProjectPath uniquePath(const HECL::ProjectPath& pakPath) const
    {
        if (type == UNIQUE_AREA)
        {
            HECL::ProjectPath areaDir(pakPath, *areaName);
            areaDir.makeDir();
            return areaDir;
        }
        else if (type == UNIQUE_LAYER)
        {
            HECL::ProjectPath areaDir(pakPath, *areaName);
            areaDir.makeDir();
            HECL::ProjectPath layerDir(areaDir, *layerName);
            layerDir.makeDir();
            return layerDir;
        }
        return pakPath;
    }
};

template <class BRIDGETYPE>
class PAKRouter;

/* Resource extractor type */
template <class PAKBRIDGE>
struct ResExtractor
{
    std::function<bool(const SpecBase&, PAKEntryReadStream&, const HECL::ProjectPath&)> func_a;
    std::function<bool(const SpecBase&, PAKEntryReadStream&, const HECL::ProjectPath&, PAKRouter<PAKBRIDGE>&,
                       const typename PAKBRIDGE::PAKType::Entry&)> func_b;
    const char* fileExt;
    unsigned weight;
};

/* PAKRouter (for detecting shared entry locations) */
template <class BRIDGETYPE>
class PAKRouter
{
public:
    using PAKType = typename BRIDGETYPE::PAKType;
    using IDType = typename PAKType::IDType;
    using EntryType = typename PAKType::Entry;
private:
    const SpecBase& m_dataSpec;
    const HECL::ProjectPath& m_gameWorking;
    const HECL::ProjectPath& m_gameCooked;
    HECL::ProjectPath m_sharedWorking;
    HECL::ProjectPath m_sharedCooked;
    const PAKType* m_pak = nullptr;
    const NOD::DiscBase::IPartition::Node* m_node = nullptr;
    HECL::ProjectPath m_pakWorking;
    HECL::ProjectPath m_pakCooked;
    std::unordered_map<typename PAKType::IDType, typename PAKType::Entry*> m_uniqueEntries;
    std::unordered_map<IDType, EntryType*> m_sharedEntries;
public:
    PAKRouter(const SpecBase& dataSpec, const HECL::ProjectPath& working, const HECL::ProjectPath& cooked)
    : m_dataSpec(dataSpec),
      m_gameWorking(working), m_gameCooked(cooked),
      m_sharedWorking(working, "Shared"), m_sharedCooked(cooked, "Shared") {}
    void build(std::vector<BRIDGETYPE>& bridges, std::function<void(float)> progress)
    {
        m_uniqueEntries.clear();
        m_sharedEntries.clear();
        size_t count = 0;
        float bridgesSz = bridges.size();

        /* Route entries unique/shared per-pak */
        for (BRIDGETYPE& bridge : bridges)
        {
            bridge.build();
            const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
            for (const auto& entry : pak.m_idMap)
            {
                auto search = m_uniqueEntries.find(entry.first);
                if (search != m_uniqueEntries.end())
                {
                    m_uniqueEntries.erase(search);
                    m_sharedEntries.insert(entry);
                }
                else
                    m_uniqueEntries.insert(entry);
            }
            progress(++count / bridgesSz);
        }
    }

    void enterPAKBridge(const BRIDGETYPE& pakBridge)
    {
        const std::string& name = pakBridge.getName();
        HECL::SystemStringView sysName(name);

        HECL::SystemString::const_iterator extit = sysName.sys_str().end() - 4;
        HECL::SystemString baseName(sysName.sys_str().begin(), extit);

        m_pakWorking.assign(m_gameWorking, baseName);
        m_pakWorking.makeDir();
        m_pakCooked.assign(m_gameCooked, baseName);
        m_pakCooked.makeDir();

        m_pak = &pakBridge.getPAK();
        m_node = &pakBridge.getNode();
    }

    HECL::ProjectPath getWorking(const typename BRIDGETYPE::PAKType::Entry* entry,
                                 const ResExtractor<BRIDGETYPE>& extractor) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getWorkingPath()");
        auto uniqueSearch = m_uniqueEntries.find(entry->id);
        if (uniqueSearch != m_uniqueEntries.end())
        {
            HECL::ProjectPath uniquePath = entry->unique.uniquePath(m_pakWorking);
            HECL::SystemString entName = m_pak->bestEntryName(*entry);
            if (extractor.fileExt)
                entName += extractor.fileExt;
            return HECL::ProjectPath(uniquePath, entName);
        }
        auto sharedSearch = m_sharedEntries.find(entry->id);
        if (sharedSearch != m_sharedEntries.end())
        {
            HECL::ProjectPath uniquePathPre = entry->unique.uniquePath(m_pakWorking);
            HECL::SystemString entName = m_pak->bestEntryName(*entry);
            if (extractor.fileExt)
                entName += extractor.fileExt;
            HECL::ProjectPath sharedPath(m_sharedWorking, entName);
            HECL::ProjectPath uniquePath(uniquePathPre, entName);
            if (extractor.func_a || extractor.func_b)
                uniquePath.makeLinkTo(sharedPath);
            m_sharedWorking.makeDir();
            return sharedPath;
        }
        LogDNACommon.report(LogVisor::FatalError, "Unable to find entry %s", entry->id.toString().c_str());
        return HECL::ProjectPath();
    }

    HECL::ProjectPath getCooked(const typename BRIDGETYPE::PAKType::Entry* entry) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getCookedPath()");
        auto uniqueSearch = m_uniqueEntries.find(entry->id);
        if (uniqueSearch != m_uniqueEntries.end())
        {
            HECL::ProjectPath uniquePath = entry->unique.uniquePath(m_pakCooked);
            return HECL::ProjectPath(uniquePath, m_pak->bestEntryName(*entry));
        }
        auto sharedSearch = m_sharedEntries.find(entry->id);
        if (sharedSearch != m_sharedEntries.end())
        {
            m_sharedCooked.makeDir();
            return HECL::ProjectPath(m_sharedCooked, m_pak->bestEntryName(*entry));
        }
        LogDNACommon.report(LogVisor::FatalError, "Unable to find entry %s", entry->id.toString().c_str());
        return HECL::ProjectPath();
    }

    HECL::SystemString getResourceRelativePath(const typename BRIDGETYPE::PAKType::Entry& a,
                                               const typename BRIDGETYPE::PAKType::IDType& b) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getResourceRelativePath()");
        const typename BRIDGETYPE::PAKType::Entry* be = m_pak->lookupEntry(b);
        if (!be)
            return HECL::SystemString();
        HECL::ProjectPath aPath = getWorking(&a, BRIDGETYPE::LookupExtractor(a));
        HECL::SystemString ret;
        for (int i=0 ; i<aPath.levelCount() ; ++i)
            ret += "../";
        HECL::ProjectPath bPath = getWorking(be, BRIDGETYPE::LookupExtractor(*be));
        ret += bPath.getRelativePath();
        return ret;
    }

    std::string getBestEntryName(const typename BRIDGETYPE::PAKType::Entry& entry) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getBestEntryName()");
        return m_pak->bestEntryName(entry);
    }

    std::string getBestEntryName(const typename BRIDGETYPE::PAKType::IDType& entry) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getBestEntryName()");
        const typename BRIDGETYPE::PAKType::Entry* e = m_pak->lookupEntry(entry);
        if (!e)
            return entry.toString();
        return m_pak->bestEntryName(*e);
    }

    bool extractResources(const BRIDGETYPE& pakBridge, bool force, std::function<void(float)> progress)
    {
        enterPAKBridge(pakBridge);
        size_t count = 0;
        size_t sz = m_pak->m_idMap.size();
        float fsz = sz;
        for (unsigned w=0 ; count<sz ; ++w)
        {
            for (const auto& item : m_pak->m_idMap)
            {
                ResExtractor<BRIDGETYPE> extractor = BRIDGETYPE::LookupExtractor(*item.second);
                if (extractor.weight != w)
                    continue;

                HECL::ProjectPath cooked = getCooked(item.second);
                if (force || cooked.getPathType() == HECL::ProjectPath::PT_NONE)
                {
                    PAKEntryReadStream s = item.second->beginReadStream(*m_node);
                    FILE* fout = HECL::Fopen(cooked.getAbsolutePath().c_str(), _S("wb"));
                    fwrite(s.data(), 1, s.length(), fout);
                    fclose(fout);
                }

                HECL::ProjectPath working = getWorking(item.second, extractor);
                if (extractor.func_a) /* Doesn't need PAKRouter access */
                {
                    if (force || working.getPathType() == HECL::ProjectPath::PT_NONE)
                    {
                        PAKEntryReadStream s = item.second->beginReadStream(*m_node);
                        extractor.func_a(m_dataSpec, s, working);
                    }
                }
                else if (extractor.func_b) /* Needs PAKRouter access */
                {
                    if (force || working.getPathType() == HECL::ProjectPath::PT_NONE)
                    {
                        PAKEntryReadStream s = item.second->beginReadStream(*m_node);
                        extractor.func_b(m_dataSpec, s, working, *this, *item.second);
                    }
                }

                progress(++count / fsz);
            }
        }

        return true;
    }
};

/* Resource cooker function */
typedef std::function<bool(const HECL::ProjectPath&, const HECL::ProjectPath&)> ResCooker;

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
