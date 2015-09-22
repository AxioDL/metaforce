#ifndef __DNACOMMON_PAK_HPP__
#define __DNACOMMON_PAK_HPP__

#include "DNACommon.hpp"

namespace Retro
{

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
    std::function<bool(PAKEntryReadStream&, const HECL::ProjectPath&)> func_a;
    std::function<bool(const SpecBase&, PAKEntryReadStream&, const HECL::ProjectPath&, PAKRouter<PAKBRIDGE>&,
                       const typename PAKBRIDGE::PAKType::Entry&, bool,
                       std::function<void(const HECL::SystemChar*)>)> func_b;
    const HECL::SystemChar* fileExts[4];
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
    using RigPair = std::pair<IDType, IDType>;
private:
    const SpecBase& m_dataSpec;
    const std::vector<BRIDGETYPE>* m_bridges = nullptr;
    std::vector<std::pair<HECL::ProjectPath,HECL::ProjectPath>> m_bridgePaths;
    size_t m_curBridgeIdx = 0;
    const HECL::ProjectPath& m_gameWorking;
    const HECL::ProjectPath& m_gameCooked;
    HECL::ProjectPath m_sharedWorking;
    HECL::ProjectPath m_sharedCooked;
    const PAKType* m_pak = nullptr;
    const NOD::DiscBase::IPartition::Node* m_node = nullptr;
    std::unordered_map<IDType, std::pair<size_t, EntryType*>> m_uniqueEntries;
    std::unordered_map<IDType, std::pair<size_t, EntryType*>> m_sharedEntries;
    std::unordered_map<IDType, RigPair> m_cmdlRigs;
public:
    PAKRouter(const SpecBase& dataSpec, const HECL::ProjectPath& working, const HECL::ProjectPath& cooked)
    : m_dataSpec(dataSpec),
      m_gameWorking(working), m_gameCooked(cooked),
      m_sharedWorking(working, "Shared"), m_sharedCooked(cooked, "Shared") {}
    void build(std::vector<BRIDGETYPE>& bridges, std::function<void(float)> progress)
    {
        m_bridges = &bridges;
        m_bridgePaths.clear();

        m_uniqueEntries.clear();
        m_sharedEntries.clear();
        m_cmdlRigs.clear();
        size_t count = 0;
        float bridgesSz = bridges.size();

        /* Route entries unique/shared per-pak */
        size_t bridgeIdx = 0;
        for (BRIDGETYPE& bridge : bridges)
        {
            const std::string& name = bridge.getName();
            HECL::SystemStringView sysName(name);

            HECL::SystemString::const_iterator extit = sysName.sys_str().end() - 4;
            HECL::SystemString baseName(sysName.sys_str().begin(), extit);

            m_bridgePaths.emplace_back(std::make_pair(HECL::ProjectPath(m_gameWorking, baseName),
                                                      HECL::ProjectPath(m_gameCooked, baseName)));

            /* Index this PAK */
            bridge.build();

            /* Add to global entry lookup */
            const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
            for (const auto& entry : pak.m_idMap)
            {
                auto sSearch = m_sharedEntries.find(entry.first);
                if (sSearch != m_sharedEntries.end())
                    continue;
                auto uSearch = m_uniqueEntries.find(entry.first);
                if (!pak.m_noShare)
                {
                    if (uSearch != m_uniqueEntries.end())
                    {
                        m_uniqueEntries.erase(uSearch);
                        m_sharedEntries[entry.first] = std::make_pair(bridgeIdx, entry.second);
                    }
                    else
                        m_uniqueEntries[entry.first] = std::make_pair(bridgeIdx, entry.second);
                }
            }

            /* Add RigPairs to global map */
            bridge.addCMDLRigPairs(m_cmdlRigs);

            progress(++count / bridgesSz);
            ++bridgeIdx;
        }
    }

    void enterPAKBridge(const BRIDGETYPE& pakBridge)
    {
        auto pit = m_bridgePaths.begin();
        size_t bridgeIdx = 0;
        for (const BRIDGETYPE& bridge : *m_bridges)
        {
            if (&bridge == &pakBridge)
            {
                pit->first.makeDir();
                pit->second.makeDir();
                m_pak = &pakBridge.getPAK();
                m_node = &pakBridge.getNode();
                m_curBridgeIdx = bridgeIdx;
                return;
            }
            ++pit;
            ++bridgeIdx;
        }
        LogDNACommon.report(LogVisor::FatalError, "PAKBridge provided to PAKRouter::enterPAKBridge() was not part of build()");
    }

    HECL::ProjectPath getWorking(const EntryType* entry,
                                 const ResExtractor<BRIDGETYPE>& extractor) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getWorkingPath()");
        auto uniqueSearch = m_uniqueEntries.find(entry->id);
        if (uniqueSearch != m_uniqueEntries.end())
        {
            const HECL::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].first;
            pakPath.makeDir();
            HECL::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
#if HECL_UCS2
            HECL::SystemString entName = HECL::UTF8ToWide(m_pak->bestEntryName(*entry));
#else
            HECL::SystemString entName = m_pak->bestEntryName(*entry);
#endif
            if (extractor.fileExts[0] && !extractor.fileExts[1])
                entName += extractor.fileExts[0];
            return HECL::ProjectPath(uniquePath, entName);
        }
        auto sharedSearch = m_sharedEntries.find(entry->id);
        if (sharedSearch != m_sharedEntries.end())
        {
            const HECL::ProjectPath& pakPath = m_bridgePaths[m_curBridgeIdx].first;
            HECL::ProjectPath uniquePathPre = entry->unique.uniquePath(pakPath);
#if HECL_UCS2
            HECL::SystemString entBase = HECL::UTF8ToWide(m_pak->bestEntryName(*entry));
#else
            HECL::SystemString entBase = m_pak->bestEntryName(*entry);
#endif
            HECL::SystemString entName = entBase;
            if (extractor.fileExts[0] && !extractor.fileExts[1])
                entName += extractor.fileExts[0];
            HECL::ProjectPath sharedPath(m_sharedWorking, entName);
            HECL::ProjectPath uniquePath(uniquePathPre, entName);
            if (extractor.func_a || extractor.func_b)
            {
                if (extractor.fileExts[0] && !extractor.fileExts[1])
                    uniquePath.makeLinkTo(sharedPath);
                else
                {
                    for (int e=0 ; e<4 ; ++e)
                    {
                        if (!extractor.fileExts[e])
                            break;
                        HECL::SystemString entName = entBase + extractor.fileExts[e];
                        HECL::ProjectPath sharedPath(m_sharedWorking, entName);
                        HECL::ProjectPath uniquePath(uniquePathPre, entName);
                        uniquePath.makeLinkTo(sharedPath);
                    }
                }
            }
            m_sharedWorking.makeDir();
            return sharedPath;
        }
        const EntryType* singleSearch = m_pak->lookupEntry(entry->id);
        if (singleSearch)
        {
            const HECL::ProjectPath& pakPath = m_bridgePaths[m_curBridgeIdx].first;
            pakPath.makeDir();
#if HECL_UCS2
            HECL::SystemString entName = HECL::UTF8ToWide(m_pak->bestEntryName(*entry));
#else
            HECL::SystemString entName = m_pak->bestEntryName(*entry);
#endif
            if (extractor.fileExts[0] && !extractor.fileExts[1])
                entName += extractor.fileExts[0];
            return HECL::ProjectPath(pakPath, entName);
        }
        LogDNACommon.report(LogVisor::FatalError, "Unable to find entry %s", entry->id.toString().c_str());
        return HECL::ProjectPath();
    }

    HECL::ProjectPath getWorking(const EntryType* entry) const
    {
        return getWorking(entry, BRIDGETYPE::LookupExtractor(*entry));
    }

    HECL::ProjectPath getWorking(const IDType& id) const
    {
        return getWorking(lookupEntry(id));
    }

    HECL::ProjectPath getCooked(const EntryType* entry) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getCookedPath()");
        auto uniqueSearch = m_uniqueEntries.find(entry->id);
        if (uniqueSearch != m_uniqueEntries.end())
        {
            const HECL::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].second;
            pakPath.makeDir();
            HECL::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
            return HECL::ProjectPath(uniquePath, m_pak->bestEntryName(*entry));
        }
        auto sharedSearch = m_sharedEntries.find(entry->id);
        if (sharedSearch != m_sharedEntries.end())
        {
            m_sharedCooked.makeDir();
            return HECL::ProjectPath(m_sharedCooked, m_pak->bestEntryName(*entry));
        }
        const EntryType* singleSearch = m_pak->lookupEntry(entry->id);
        if (singleSearch)
        {
            const HECL::ProjectPath& pakPath = m_bridgePaths[m_curBridgeIdx].second;
            pakPath.makeDir();
            return HECL::ProjectPath(pakPath, m_pak->bestEntryName(*entry));
        }
        LogDNACommon.report(LogVisor::FatalError, "Unable to find entry %s", entry->id.toString().c_str());
        return HECL::ProjectPath();
    }

    HECL::ProjectPath getCooked(const IDType& id) const
    {
        return getCooked(lookupEntry(id));
    }

    HECL::SystemString getResourceRelativePath(const EntryType& a, const IDType& b) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getResourceRelativePath()");
        const typename BRIDGETYPE::PAKType::Entry* be = lookupEntry(b);
        if (!be)
            return HECL::SystemString();
        HECL::ProjectPath aPath = getWorking(&a, BRIDGETYPE::LookupExtractor(a));
        HECL::SystemString ret;
        for (int i=0 ; i<aPath.levelCount() ; ++i)
            ret += _S("../");
        HECL::ProjectPath bPath = getWorking(be, BRIDGETYPE::LookupExtractor(*be));
        ret += bPath.getRelativePath();
        return ret;
    }

    std::string getBestEntryName(const EntryType& entry) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getBestEntryName()");
        return m_pak->bestEntryName(entry);
    }

    std::string getBestEntryName(const IDType& entry) const
    {
        if (!m_pak)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getBestEntryName()");
        const typename BRIDGETYPE::PAKType::Entry* e = m_pak->lookupEntry(entry);
        if (!e)
            return entry.toString();
        return m_pak->bestEntryName(*e);
    }

    bool extractResources(const BRIDGETYPE& pakBridge, bool force,
                          std::function<void(const HECL::SystemChar*, float)> progress)
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
                
                std::string bestName = getBestEntryName(*item.second);
                HECL::SystemStringView bestNameView(bestName);
                float thisFac = ++count / fsz;
                progress(bestNameView.sys_str().c_str(), thisFac);

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
                        extractor.func_a(s, working);
                    }
                }
                else if (extractor.func_b) /* Needs PAKRouter access */
                {
                    if (force || working.getPathType() == HECL::ProjectPath::PT_NONE)
                    {
                        PAKEntryReadStream s = item.second->beginReadStream(*m_node);
                        extractor.func_b(m_dataSpec, s, working, *this, *item.second, force,
                                         [&progress, thisFac](const HECL::SystemChar* update)
                                         {
                                             progress(update, thisFac);
                                         });
                    }
                }
            }
        }

        return true;
    }

    const typename BRIDGETYPE::PAKType::Entry* lookupEntry(const IDType& entry,
                                                           const NOD::DiscBase::IPartition::Node** nodeOut=nullptr,
                                                           bool silenceWarnings=false) const
    {
        if (!m_bridges)
            LogDNACommon.report(LogVisor::FatalError,
            "PAKRouter::build() must be called before PAKRouter::lookupEntry()");
        if (m_pak)
        {
            const EntryType* ent = m_pak->lookupEntry(entry);
            if (ent)
            {
                if (nodeOut)
                    *nodeOut = m_node;
                return ent;
            }
        }
        for (const BRIDGETYPE& bridge : *m_bridges)
        {
            const PAKType& pak = bridge.getPAK();
            const EntryType* ent = pak.lookupEntry(entry);
            if (ent)
            {
                if (nodeOut)
                    *nodeOut = &bridge.getNode();
                return ent;
            }
        }
        if (!silenceWarnings)
            LogDNACommon.report(LogVisor::Warning, "unable to find PAK entry %s", entry.toString().c_str());
        if (nodeOut)
            *nodeOut = nullptr;
        return nullptr;
    }

    template <typename DNA>
    bool lookupAndReadDNA(const IDType& id, DNA& out, bool silenceWarnings=false)
    {
        const NOD::DiscBase::IPartition::Node* node;
        const EntryType* entry = lookupEntry(id, &node, silenceWarnings);
        if (!entry)
            return false;
        PAKEntryReadStream rs = entry->beginReadStream(*node);
        out.read(rs);
        return true;
    }

    const RigPair* lookupCMDLRigPair(const IDType& id) const
    {
        auto search = m_cmdlRigs.find(id);
        if (search == m_cmdlRigs.end())
            return nullptr;
        return &search->second;
    }
};

}

#endif // __DNACOMMON_PAK_HPP__
