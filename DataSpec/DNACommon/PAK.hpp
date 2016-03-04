#ifndef __DNACOMMON_PAK_HPP__
#define __DNACOMMON_PAK_HPP__

#include "DNACommon.hpp"
#include "NamedResourceCatalog.hpp"

namespace DataSpec
{

/* PAK entry stream reader */
class PAKEntryReadStream : public athena::io::IStreamReader
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
            LogDNACommon.report(logvisor::Fatal, "PAK stream cursor overrun");
    }
    void seek(atInt64 pos, athena::SeekOrigin origin)
    {
        if (origin == athena::Begin)
            m_pos = pos;
        else if (origin == athena::Current)
            m_pos += pos;
        else if (origin == athena::End)
            m_pos = m_sz + pos;
        if (m_pos > m_sz)
            LogDNACommon.report(logvisor::Fatal, "PAK stream cursor overrun");
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
    const hecl::SystemString* m_levelName = nullptr;
    const hecl::SystemString* m_areaName = nullptr;
    const hecl::SystemString* m_layerName = nullptr;
    UniqueResult() = default;
    UniqueResult(Type tp) : m_type(tp) {}

    template<class PAKBRIDGE>
    void checkEntry(const PAKBRIDGE& pakBridge, const typename PAKBRIDGE::PAKType::Entry& entry)
    {
        UniqueResult::Type resultType = UniqueResult::Type::NotFound;
        bool foundOneLayer = false;
        const hecl::SystemString* levelName = nullptr;
        typename PAKBRIDGE::PAKType::IDType levelId;
        typename PAKBRIDGE::PAKType::IDType areaId;
        unsigned layerIdx;
        for (const auto& lpair : pakBridge.m_levelDeps)
        {
            if (entry.id == lpair.first)
            {
                levelName = &lpair.second.name;
                resultType = UniqueResult::Type::Level;
                break;
            }

            for (const auto& pair : lpair.second.areas)
            {
                unsigned l=0;
                for (const auto& layer : pair.second.layers)
                {
                    if (layer.resources.find(entry.id) != layer.resources.end())
                    {
                        if (foundOneLayer)
                        {
                            if (areaId == pair.first)
                            {
                                resultType = UniqueResult::Type::Area;
                            }
                            else if (levelId == lpair.first)
                            {
                                resultType = UniqueResult::Type::Level;
                                break;
                            }
                            else
                            {
                                m_type = UniqueResult::Type::Pak;
                                return;
                            }
                            continue;
                        }
                        else
                            resultType = UniqueResult::Type::Layer;
                        levelName = &lpair.second.name;
                        levelId = lpair.first;
                        areaId = pair.first;
                        layerIdx = l;
                        foundOneLayer = true;
                    }
                    ++l;
                }
                if (pair.second.resources.find(entry.id) != pair.second.resources.end())
                {
                    if (foundOneLayer)
                    {
                        if (areaId == pair.first)
                        {
                            resultType = UniqueResult::Type::Area;
                        }
                        else if (levelId == lpair.first)
                        {
                            resultType = UniqueResult::Type::Level;
                            break;
                        }
                        else
                        {
                            m_type = UniqueResult::Type::Pak;
                            return;
                        }
                        continue;
                    }
                    else
                        resultType = UniqueResult::Type::Area;
                    levelName = &lpair.second.name;
                    levelId = lpair.first;
                    areaId = pair.first;
                    foundOneLayer = true;
                }
            }
        }
        m_type = resultType;
        m_levelName = levelName;
        if (resultType == UniqueResult::Type::Layer || resultType == UniqueResult::Type::Area)
        {
            const typename PAKBRIDGE::Level::Area& area = pakBridge.m_levelDeps.at(levelId).areas.at(areaId);
            m_areaName = &area.name;
            if (resultType == UniqueResult::Type::Layer)
            {
                const typename PAKBRIDGE::Level::Area::Layer& layer = area.layers[layerIdx];
                m_layerName = &layer.name;
            }
        }
    }

    hecl::ProjectPath uniquePath(const hecl::ProjectPath& pakPath) const
    {
        if (m_type == Type::Pak)
            return pakPath;

        hecl::ProjectPath levelDir;
        if (m_levelName)
            levelDir.assign(pakPath, *m_levelName);
        else
            levelDir = pakPath;
        levelDir.makeDir();

        if (m_type == Type::Area)
        {
            hecl::ProjectPath areaDir(levelDir, *m_areaName);
            areaDir.makeDir();
            return areaDir;
        }
        else if (m_type == Type::Layer)
        {
            hecl::ProjectPath areaDir(levelDir, *m_areaName);
            areaDir.makeDir();
            hecl::ProjectPath layerDir(areaDir, *m_layerName);
            layerDir.makeDir();
            return layerDir;
        }

        return levelDir;
    }
};

template <class BRIDGETYPE>
class PAKRouter;

/* Resource extractor type */
template <class PAKBRIDGE>
struct ResExtractor
{
    std::function<bool(PAKEntryReadStream&, const hecl::ProjectPath&)> func_a;
    std::function<bool(const SpecBase&, PAKEntryReadStream&, const hecl::ProjectPath&, PAKRouter<PAKBRIDGE>&,
                       const typename PAKBRIDGE::PAKType::Entry&, bool,
                       std::function<void(const hecl::SystemChar*)>)> func_b;
    const hecl::SystemChar* fileExts[4];
    unsigned weight;
    std::function<void(const SpecBase&, PAKEntryReadStream&, PAKRouter<PAKBRIDGE>&,
                       typename PAKBRIDGE::PAKType::Entry&)> func_name;
};

/* Level hierarchy representation */
template <class IDType>
struct Level
{
    hecl::SystemString name;
    struct Area
    {
        hecl::SystemString name;
        struct Layer
        {
            hecl::SystemString name;
            bool active;
            std::unordered_set<IDType> resources;
        };
        std::vector<Layer> layers;
        std::unordered_set<IDType> resources;
    };
    std::unordered_map<IDType, Area> areas;
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
    NamedResourceCatalog<IDType> m_catalog;
    const SpecBase& m_dataSpec;
    const std::vector<BRIDGETYPE>* m_bridges = nullptr;
    std::vector<std::pair<hecl::ProjectPath,hecl::ProjectPath>> m_bridgePaths;
    size_t m_curBridgeIdx = 0;
    const hecl::ProjectPath& m_gameWorking;
    const hecl::ProjectPath& m_gameCooked;
    hecl::ProjectPath m_sharedWorking;
    hecl::ProjectPath m_sharedCooked;
    const PAKType* m_pak = nullptr;
    const nod::Node* m_node = nullptr;
    std::unordered_map<IDType, std::pair<size_t, EntryType*>> m_uniqueEntries;
    std::unordered_map<IDType, std::pair<size_t, EntryType*>> m_sharedEntries;
    std::unordered_map<IDType, RigPair> m_cmdlRigs;
public:
    PAKRouter(const SpecBase& dataSpec, const hecl::ProjectPath& working, const hecl::ProjectPath& cooked)
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
            hecl::SystemStringView sysName(name);

            hecl::SystemString::const_iterator extit = sysName.sys_str().end() - 4;
            hecl::SystemString baseName(sysName.sys_str().begin(), extit);

            m_bridgePaths.emplace_back(std::make_pair(hecl::ProjectPath(m_gameWorking, baseName),
                                                      hecl::ProjectPath(m_gameCooked, baseName)));

            /* Index this PAK */
            bridge.build();

            /* Add to global entry lookup */
            const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
            for (const auto& entry : pak.m_idMap)
            {
                if (!pak.m_noShare)
                {
                    auto sSearch = m_sharedEntries.find(entry.first);
                    if (sSearch != m_sharedEntries.end())
                        continue;
                    auto uSearch = m_uniqueEntries.find(entry.first);
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
            bridge.addCMDLRigPairs(*this, m_cmdlRigs);

            /* Add named resources to catalog */
            for (const auto& namedEntry : pak.m_nameEntries)
                m_catalog.addNamedResource(namedEntry.name, namedEntry.id, namedEntry.type);

            progress(++count / bridgesSz);
            ++bridgeIdx;
        }

        hecl::SystemString catalogPath = hecl::ProjectPath(m_gameCooked, "catalog.yaml").getAbsolutePath();
        FILE* catalog = hecl::Fopen(catalogPath.c_str(), _S("wb"));
        if (catalog)
        {
            m_catalog.toYAMLFile(catalog);
            fclose(catalog);
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
        LogDNACommon.report(logvisor::Fatal, "PAKBridge provided to PAKRouter::enterPAKBridge() was not part of build()");
    }

    hecl::ProjectPath getWorking(const EntryType* entry,
                                 const ResExtractor<BRIDGETYPE>& extractor) const
    {
        if (!m_pak)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getWorkingPath()");
        if (m_pak->m_noShare)
        {
            const EntryType* singleSearch = m_pak->lookupEntry(entry->id);
            if (singleSearch)
            {
                const hecl::ProjectPath& pakPath = m_bridgePaths[m_curBridgeIdx].first;
                pakPath.makeDir();
    #if HECL_UCS2
                hecl::SystemString entName = hecl::UTF8ToWide(m_pak->bestEntryName(*entry));
    #else
                hecl::SystemString entName = m_pak->bestEntryName(*entry);
    #endif
                if (extractor.fileExts[0] && !extractor.fileExts[1])
                    entName += extractor.fileExts[0];
                return hecl::ProjectPath(pakPath, entName);
            }
        }

        auto uniqueSearch = m_uniqueEntries.find(entry->id);
        if (uniqueSearch != m_uniqueEntries.end())
        {
            const hecl::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].first;
            pakPath.makeDir();
            hecl::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
#if HECL_UCS2
            hecl::SystemString entName = hecl::UTF8ToWide(m_pak->bestEntryName(*entry));
#else
            hecl::SystemString entName = m_pak->bestEntryName(*entry);
#endif
            if (extractor.fileExts[0] && !extractor.fileExts[1])
                entName += extractor.fileExts[0];
            return hecl::ProjectPath(uniquePath, entName);
        }

        auto sharedSearch = m_sharedEntries.find(entry->id);
        if (sharedSearch != m_sharedEntries.end())
        {
            const hecl::ProjectPath& pakPath = m_bridgePaths[m_curBridgeIdx].first;
            hecl::ProjectPath uniquePathPre = entry->unique.uniquePath(pakPath);
#if HECL_UCS2
            hecl::SystemString entBase = hecl::UTF8ToWide(m_pak->bestEntryName(*entry));
#else
            hecl::SystemString entBase = m_pak->bestEntryName(*entry);
#endif
            hecl::SystemString entName = entBase;
            if (extractor.fileExts[0] && !extractor.fileExts[1])
                entName += extractor.fileExts[0];
            hecl::ProjectPath sharedPath(m_sharedWorking, entName);
            m_sharedWorking.makeDir();
            return sharedPath;
        }

        LogDNACommon.report(logvisor::Fatal, "Unable to find entry %s", entry->id.toString().c_str());
        return hecl::ProjectPath();
    }

    hecl::ProjectPath getWorking(const EntryType* entry) const
    {
        return getWorking(entry, BRIDGETYPE::LookupExtractor(*entry));
    }

    hecl::ProjectPath getWorking(const IDType& id) const
    {
        return getWorking(lookupEntry(id));
    }

    hecl::ProjectPath getCooked(const EntryType* entry) const
    {
        if (!m_pak)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getCookedPath()");
        if (m_pak->m_noShare)
        {
            const EntryType* singleSearch = m_pak->lookupEntry(entry->id);
            if (singleSearch)
            {
                const hecl::ProjectPath& pakPath = m_bridgePaths[m_curBridgeIdx].second;
                pakPath.makeDir();
                return hecl::ProjectPath(pakPath, m_pak->bestEntryName(*entry));
            }
        }
        auto uniqueSearch = m_uniqueEntries.find(entry->id);
        if (uniqueSearch != m_uniqueEntries.end())
        {
            const hecl::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].second;
            pakPath.makeDir();
            hecl::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
            return hecl::ProjectPath(uniquePath, m_pak->bestEntryName(*entry));
        }
        auto sharedSearch = m_sharedEntries.find(entry->id);
        if (sharedSearch != m_sharedEntries.end())
        {
            m_sharedCooked.makeDir();
            return hecl::ProjectPath(m_sharedCooked, m_pak->bestEntryName(*entry));
        }
        LogDNACommon.report(logvisor::Fatal, "Unable to find entry %s", entry->id.toString().c_str());
        return hecl::ProjectPath();
    }

    hecl::ProjectPath getCooked(const IDType& id) const
    {
        return getCooked(lookupEntry(id));
    }

    hecl::SystemString getResourceRelativePath(const EntryType& a, const IDType& b) const
    {
        if (!m_pak)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getResourceRelativePath()");
        const typename BRIDGETYPE::PAKType::Entry* be = lookupEntry(b);
        if (!be)
            return hecl::SystemString();
        hecl::ProjectPath aPath = getWorking(&a, BRIDGETYPE::LookupExtractor(a));
        hecl::SystemString ret;
        for (int i=0 ; i<aPath.levelCount() ; ++i)
            ret += _S("../");
        hecl::ProjectPath bPath = getWorking(be, BRIDGETYPE::LookupExtractor(*be));
        ret += bPath.getRelativePath();
        return ret;
    }

    std::string getBestEntryName(const EntryType& entry) const
    {
        if (!m_pak)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getBestEntryName()");
        return m_pak->bestEntryName(entry);
    }

    std::string getBestEntryName(const IDType& entry) const
    {
        if (!m_pak)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::enterPAKBridge() must be called before PAKRouter::getBestEntryName()");
        const typename BRIDGETYPE::PAKType::Entry* e = m_pak->lookupEntry(entry);
        if (!e)
            return entry.toString();
        return m_pak->bestEntryName(*e);
    }

    bool extractResources(const BRIDGETYPE& pakBridge, bool force,
                          std::function<void(const hecl::SystemChar*, float)> progress)
    {
        enterPAKBridge(pakBridge);
        size_t count = 0;
        size_t sz = m_pak->m_idMap.size();
        float fsz = sz;
        for (unsigned w=0 ; count<sz ; ++w)
        {
            for (const auto& item : m_pak->m_firstEntries)
            {
                ResExtractor<BRIDGETYPE> extractor = BRIDGETYPE::LookupExtractor(*item);
                if (extractor.weight != w)
                    continue;
                
                std::string bestName = getBestEntryName(*item);
                hecl::SystemStringView bestNameView(bestName);
                float thisFac = ++count / fsz;
                progress(bestNameView.sys_str().c_str(), thisFac);

                /* TODO: Position after extracted item */
                hecl::ProjectPath cooked = getCooked(item);
                if (force || cooked.getPathType() == hecl::ProjectPath::Type::None)
                {
                    PAKEntryReadStream s = item->beginReadStream(*m_node);
                    FILE* fout = hecl::Fopen(cooked.getAbsolutePath().c_str(), _S("wb"));
                    fwrite(s.data(), 1, s.length(), fout);
                    fclose(fout);
                }

                hecl::ProjectPath working = getWorking(item, extractor);
                if (extractor.func_a) /* Doesn't need PAKRouter access */
                {
                    if (force || working.getPathType() == hecl::ProjectPath::Type::None)
                    {
                        PAKEntryReadStream s = item->beginReadStream(*m_node);
                        extractor.func_a(s, working);
                    }
                }
                else if (extractor.func_b) /* Needs PAKRouter access */
                {
                    if (force || working.getPathType() == hecl::ProjectPath::Type::None)
                    {
                        PAKEntryReadStream s = item->beginReadStream(*m_node);
                        extractor.func_b(m_dataSpec, s, working, *this, *item, force,
                                         [&progress, thisFac](const hecl::SystemChar* update)
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
                                                           const nod::Node** nodeOut=nullptr,
                                                           bool silenceWarnings=false,
                                                           bool currentPAK=false) const
    {
        if (!entry)
            return nullptr;

        if (!m_bridges)
            LogDNACommon.report(logvisor::Fatal,
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

        if (currentPAK)
        {
            if (!silenceWarnings)
                LogDNACommon.report(logvisor::Warning, "unable to find PAK entry %s in current PAK", entry.toString().c_str());
            return nullptr;
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
            LogDNACommon.report(logvisor::Warning, "unable to find PAK entry %s", entry.toString().c_str());
        if (nodeOut)
            *nodeOut = nullptr;
        return nullptr;
    }

    template <typename DNA>
    bool lookupAndReadDNA(const IDType& id, DNA& out, bool silenceWarnings=false)
    {
        const nod::Node* node;
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

    hecl::ProjectPath getAreaLayerWorking(const IDType& areaId, int layerIdx) const
    {
        if (!m_bridges)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::build() must be called before PAKRouter::getAreaLayerWorking()");
        auto bridgePathIt = m_bridgePaths.cbegin();
        for (const BRIDGETYPE& bridge : *m_bridges)
        {
            for (const auto& level : bridge.m_levelDeps)
                for (const auto& area : level.second.areas)
                    if (area.first == areaId)
                    {
                        hecl::ProjectPath levelPath(bridgePathIt->first, level.second.name);
                        hecl::ProjectPath areaPath(levelPath, area.second.name);
                        if (layerIdx < 0)
                            return areaPath;
                        return hecl::ProjectPath(areaPath, area.second.layers.at(layerIdx).name);
                    }
            ++bridgePathIt;
        }
        return hecl::ProjectPath();
    }

    hecl::ProjectPath getAreaLayerCooked(const IDType& areaId, int layerIdx) const
    {
        if (!m_bridges)
            LogDNACommon.report(logvisor::Fatal,
            "PAKRouter::build() must be called before PAKRouter::getAreaLayerCooked()");
        auto bridgePathIt = m_bridgePaths.cbegin();
        for (const BRIDGETYPE& bridge : *m_bridges)
        {
            for (const auto& level : bridge.m_levelDeps)
                for (const auto& area : level.second.areas)
                    if (area.first == areaId)
                    {
                        hecl::ProjectPath levelPath(bridgePathIt->second, level.second.name);
                        hecl::ProjectPath areaPath(levelPath, area.second.name);
                        if (layerIdx < 0)
                            return areaPath;
                        return hecl::ProjectPath(areaPath, area.second.layers.at(layerIdx).name);
                    }
            ++bridgePathIt;
        }
        return hecl::ProjectPath();
    }
};

}

#endif // __DNACOMMON_PAK_HPP__
