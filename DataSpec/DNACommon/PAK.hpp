#ifndef __DNACOMMON_PAK_HPP__
#define __DNACOMMON_PAK_HPP__

#include "DNACommon.hpp"

namespace DataSpec
{

/** PAK entry stream reader */
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
    void seek(atInt64 pos, Athena::SeekOrigin origin)
    {
        if (origin == Athena::Begin)
            m_pos = pos;
        else if (origin == Athena::Current)
            m_pos += pos;
        else if (origin == Athena::End)
            m_pos = m_sz + pos;
        if (m_pos > m_sz)
            LogDNACommon.report(LogVisor::FatalError, "PAK stream cursor overrun");
    }
    atUint64 position() const {return m_pos;}
    atUint64 length() const {return m_sz;}
    const atUint8* data() const {return m_buf.get();}
    atUint64 readUBytesToBuf(void* buf, atUint64 len)
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
    enum class Type
    {
        NotFound,
        Pak,
        Level,
        Area,
        Layer
    } m_type = Type::NotFound;
    const HECL::SystemString* m_levelName = nullptr;
    const HECL::SystemString* m_areaName = nullptr;
    const HECL::SystemString* m_layerName = nullptr;
    UniqueResult() = default;
    UniqueResult(Type tp) : m_type(tp) {}

    template <class PAKBRIDGE>
    void checkEntry(const PAKBRIDGE& pakBridge, const typename PAKBRIDGE::PAKType::Entry& entry);

    HECL::ProjectPath uniquePath(const HECL::ProjectPath& pakPath) const;
};

template <class BRIDGETYPE>
class PAKRouter;

/** Resource extractor type */
template <class PAKBRIDGE>
struct ResExtractor
{
    std::function<bool(PAKEntryReadStream&, const HECL::ProjectPath&)> func_a;
    std::function<bool(const SpecBase&, PAKEntryReadStream&, const HECL::ProjectPath&, PAKRouter<PAKBRIDGE>&,
                       const typename PAKBRIDGE::PAKType::Entry&, bool,
                       std::function<void(const HECL::SystemChar*)>)> func_b;
    const HECL::SystemChar* fileExts[4];
    unsigned weight;
    std::function<void(const SpecBase&, PAKEntryReadStream&, PAKRouter<PAKBRIDGE>&,
                       typename PAKBRIDGE::PAKType::Entry&)> func_name;
};

/** Level hierarchy representation */
template <class IDType>
struct Level
{
    HECL::SystemString name;
    struct Area
    {
        HECL::SystemString name;
        struct Layer
        {
            HECL::SystemString name;
            bool active;
            std::unordered_set<IDType> resources;
        };
        std::vector<Layer> layers;
        std::unordered_set<IDType> resources;
    };
    std::unordered_map<IDType, Area> areas;
};

/** PAKRouter (for detecting shared entry locations) */
template <class BRIDGETYPE>
class PAKRouter : public PAKRouterBase
{
public:
    using PAKType = typename BRIDGETYPE::PAKType;
    using IDType = typename PAKType::IDType;
    using EntryType = typename PAKType::Entry;
    using RigPair = std::pair<IDType, IDType>;
private:
    const std::vector<BRIDGETYPE>* m_bridges = nullptr;
    std::vector<std::pair<HECL::ProjectPath,HECL::ProjectPath>> m_bridgePaths;
    size_t m_curBridgeIdx = 0;
    const HECL::ProjectPath& m_gameWorking;
    const HECL::ProjectPath& m_gameCooked;
    HECL::ProjectPath m_sharedWorking;
    HECL::ProjectPath m_sharedCooked;
    const PAKType* m_pak = nullptr;
    const NOD::Node* m_node = nullptr;
    std::unordered_map<IDType, std::pair<size_t, EntryType*>> m_uniqueEntries;
    std::unordered_map<IDType, std::pair<size_t, EntryType*>> m_sharedEntries;
    std::unordered_map<IDType, RigPair> m_cmdlRigs;
public:
    PAKRouter(const SpecBase& dataSpec, const HECL::ProjectPath& working, const HECL::ProjectPath& cooked)
    : PAKRouterBase(dataSpec),
      m_gameWorking(working), m_gameCooked(cooked),
      m_sharedWorking(working, "Shared"), m_sharedCooked(cooked, "Shared") {}

    void build(std::vector<BRIDGETYPE>& bridges, std::function<void(float)> progress);

    void enterPAKBridge(const BRIDGETYPE& pakBridge);

    HECL::ProjectPath getWorking(const EntryType* entry,
                                 const ResExtractor<BRIDGETYPE>& extractor) const;
    HECL::ProjectPath getWorking(const EntryType* entry) const;
    HECL::ProjectPath getWorking(const IDType& id) const;
    HECL::ProjectPath getCooked(const EntryType* entry) const;
    HECL::ProjectPath getCooked(const IDType& id) const;

    HECL::SystemString getResourceRelativePath(const EntryType& a, const IDType& b) const;

    std::string getBestEntryName(const EntryType& entry) const;
    std::string getBestEntryName(const IDType& entry) const;

    bool extractResources(const BRIDGETYPE& pakBridge, bool force,
                          std::function<void(const HECL::SystemChar*, float)> progress);

    const typename BRIDGETYPE::PAKType::Entry* lookupEntry(const IDType& entry,
                                                           const NOD::Node** nodeOut=nullptr,
                                                           bool silenceWarnings=false,
                                                           bool currentPAK=false) const;

    template <typename DNA>
    bool lookupAndReadDNA(const IDType& id, DNA& out, bool silenceWarnings=false)
    {
        const NOD::Node* node;
        const EntryType* entry = lookupEntry(id, &node, silenceWarnings);
        if (!entry)
            return false;
        PAKEntryReadStream rs = entry->beginReadStream(*node);
        out.read(rs);
        return true;
    }

    const RigPair* lookupCMDLRigPair(const IDType& id) const;

    HECL::ProjectPath getAreaLayerWorking(const IDType& areaId, int layerIdx) const;
    HECL::ProjectPath getAreaLayerCooked(const IDType& areaId, int layerIdx) const;
};

}

#endif // __DNACOMMON_PAK_HPP__
