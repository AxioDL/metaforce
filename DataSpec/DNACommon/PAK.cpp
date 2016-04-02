#include "PAK.hpp"
#include "../DNAMP1/DNAMP1.hpp"
#include "../DNAMP2/DNAMP2.hpp"
#include "../DNAMP3/DNAMP3.hpp"

namespace DataSpec
{

template <class PAKBRIDGE>
void UniqueResult::checkEntry(const PAKBRIDGE& pakBridge, const typename PAKBRIDGE::PAKType::Entry& entry)
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

template void UniqueResult::checkEntry(const DNAMP1::PAKBridge& pakBridge,
                                       const DNAMP1::PAKBridge::PAKType::Entry& entry);
template void UniqueResult::checkEntry(const DNAMP2::PAKBridge& pakBridge,
                                       const DNAMP2::PAKBridge::PAKType::Entry& entry);
template void UniqueResult::checkEntry(const DNAMP3::PAKBridge& pakBridge,
                                       const DNAMP3::PAKBridge::PAKType::Entry& entry);

hecl::ProjectPath UniqueResult::uniquePath(const hecl::ProjectPath& pakPath) const
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

template <class BRIDGETYPE>
void PAKRouter<BRIDGETYPE>::build(std::vector<BRIDGETYPE>& bridges, std::function<void(float)> progress)
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
            else
                m_uniqueEntries[entry.first] = std::make_pair(bridgeIdx, entry.second);
        }

        /* Add RigPairs to global map */
        bridge.addCMDLRigPairs(*this, m_cmdlRigs);

        progress(++count / bridgesSz);
        ++bridgeIdx;
    }

    /* Add named resources to catalog YAML files */
    for (BRIDGETYPE& bridge : bridges)
    {
        athena::io::YAMLDocWriter catalogWriter(nullptr);

        enterPAKBridge(bridge);
        const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
        for (const auto& namedEntry : pak.m_nameEntries)
        {
            catalogWriter.enterSubRecord(namedEntry.name.c_str());
            catalogWriter.writeString(nullptr, getWorking(namedEntry.id).getRelativePathUTF8().c_str());
            catalogWriter.leaveSubRecord();
        }

        /* Write catalog */
        intptr_t curBridgeIdx = reinterpret_cast<intptr_t>(m_curBridgeIdx.get());
        const hecl::ProjectPath& pakPath = m_bridgePaths[curBridgeIdx].first;
        hecl::SystemString catalogPath = hecl::ProjectPath(pakPath, "catalog.yaml").getAbsolutePath();
        FILE* catalog = hecl::Fopen(catalogPath.c_str(), _S("w"));
        yaml_emitter_set_output_file(catalogWriter.getEmitter(), catalog);
        catalogWriter.finish();
        fclose(catalog);
    }
}

template <class BRIDGETYPE>
void PAKRouter<BRIDGETYPE>::enterPAKBridge(const BRIDGETYPE& pakBridge)
{
    g_PakRouter = this;
    auto pit = m_bridgePaths.begin();
    size_t bridgeIdx = 0;
    for (const BRIDGETYPE& bridge : *m_bridges)
    {
        if (&bridge == &pakBridge)
        {
            pit->first.makeDir();
            pit->second.makeDir();
            m_pak.reset(&pakBridge.getPAK());
            m_node.reset(&pakBridge.getNode());
            m_curBridgeIdx.reset(reinterpret_cast<void*>(bridgeIdx));
            return;
        }
        ++pit;
        ++bridgeIdx;
    }
    LogDNACommon.report(logvisor::Fatal,
    "PAKBridge provided to PAKRouter::enterPAKBridge() was not part of build()");
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getWorking(const EntryType* entry,
                                                    const ResExtractor<BRIDGETYPE>& extractor) const
{
    if (!entry)
        return hecl::ProjectPath();
    const PAKType* pak = m_pak.get();
    intptr_t curBridgeIdx = reinterpret_cast<intptr_t>(m_curBridgeIdx.get());
    if (!pak)
        LogDNACommon.report(logvisor::Fatal,
        "PAKRouter::enterPAKBridge() must be called before PAKRouter::getWorkingPath()");
    if (pak->m_noShare)
    {
        const EntryType* singleSearch = pak->lookupEntry(entry->id);
        if (singleSearch)
        {
            const hecl::ProjectPath& pakPath = m_bridgePaths[curBridgeIdx].first;
            pakPath.makeDir();
#if HECL_UCS2
            hecl::SystemString entName = hecl::UTF8ToWide(getBestEntryName(*entry));
#else
            hecl::SystemString entName = getBestEntryName(*entry);
#endif
            if (extractor.fileExts[0] && !extractor.fileExts[1])
                entName += extractor.fileExts[0];
            return hecl::ProjectPath(pakPath, entName);
        }
    }

    auto uniqueSearch = m_uniqueEntries.find(entry->id);
    if (uniqueSearch != m_uniqueEntries.end())
    {
        const BRIDGETYPE& bridge = m_bridges->at(uniqueSearch->second.first);
        const hecl::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].first;
        pakPath.makeDir();
#if HECL_UCS2
        hecl::SystemString entName = hecl::UTF8ToWide(getBestEntryName(*entry));
#else
        hecl::SystemString entName = getBestEntryName(*entry);
#endif
        if (extractor.fileExts[0] && !extractor.fileExts[1])
            entName += extractor.fileExts[0];
        if (bridge.getPAK().m_noShare)
        {
            return hecl::ProjectPath(pakPath, entName);
        }
        else
        {
            hecl::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
            return hecl::ProjectPath(uniquePath, entName);
        }
    }

    auto sharedSearch = m_sharedEntries.find(entry->id);
    if (sharedSearch != m_sharedEntries.end())
    {
#if HECL_UCS2
        hecl::SystemString entBase = hecl::UTF8ToWide(getBestEntryName(*entry));
#else
        hecl::SystemString entBase = getBestEntryName(*entry);
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

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getWorking(const EntryType* entry) const
{
    if (!entry)
        return hecl::ProjectPath();
    return getWorking(entry, BRIDGETYPE::LookupExtractor(*entry));
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getWorking(const IDType& id) const
{
    return getWorking(lookupEntry(id));
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getCooked(const EntryType* entry) const
{
    if (!entry)
        return hecl::ProjectPath();
    const PAKType* pak = m_pak.get();
    intptr_t curBridgeIdx = reinterpret_cast<intptr_t>(m_curBridgeIdx.get());
    if (!pak)
        LogDNACommon.report(logvisor::Fatal,
        "PAKRouter::enterPAKBridge() must be called before PAKRouter::getCookedPath()");
    if (pak->m_noShare)
    {
        const EntryType* singleSearch = pak->lookupEntry(entry->id);
        if (singleSearch)
        {
            const hecl::ProjectPath& pakPath = m_bridgePaths[curBridgeIdx].second;
            pakPath.makeDir();
            return hecl::ProjectPath(pakPath, getBestEntryName(*entry));
        }
    }
    auto uniqueSearch = m_uniqueEntries.find(entry->id);
    if (uniqueSearch != m_uniqueEntries.end())
    {
        const BRIDGETYPE& bridge = m_bridges->at(uniqueSearch->second.first);
        const hecl::ProjectPath& pakPath = m_bridgePaths[uniqueSearch->second.first].second;
        pakPath.makeDir();
        if (bridge.getPAK().m_noShare)
        {
            return hecl::ProjectPath(pakPath, getBestEntryName(*entry));
        }
        else
        {
            hecl::ProjectPath uniquePath = entry->unique.uniquePath(pakPath);
            return hecl::ProjectPath(uniquePath, getBestEntryName(*entry));
        }
    }
    auto sharedSearch = m_sharedEntries.find(entry->id);
    if (sharedSearch != m_sharedEntries.end())
    {
        m_sharedCooked.makeDir();
        return hecl::ProjectPath(m_sharedCooked, getBestEntryName(*entry));
    }
    LogDNACommon.report(logvisor::Fatal, "Unable to find entry %s", entry->id.toString().c_str());
    return hecl::ProjectPath();
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getCooked(const IDType& id) const
{
    return getCooked(lookupEntry(id));
}

template <class BRIDGETYPE>
hecl::SystemString PAKRouter<BRIDGETYPE>::getResourceRelativePath(const EntryType& a, const IDType& b) const
{
    const PAKType* pak = m_pak.get();
    if (!pak)
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

template <class BRIDGETYPE>
std::string PAKRouter<BRIDGETYPE>::getBestEntryName(const EntryType& entry) const
{
    std::string name;
    for (const BRIDGETYPE& bridge : *m_bridges)
    {
        const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
        bool named;
        name = pak.bestEntryName(entry, named);
        if (named)
            return name;
    }
    return name;
}

template <class BRIDGETYPE>
std::string PAKRouter<BRIDGETYPE>::getBestEntryName(const IDType& entry) const
{
    std::string name;
    for (const BRIDGETYPE& bridge : *m_bridges)
    {
        const typename BRIDGETYPE::PAKType& pak = bridge.getPAK();
        const typename BRIDGETYPE::PAKType::Entry* e = pak.lookupEntry(entry);
        if (!e)
            continue;
        bool named;
        name = pak.bestEntryName(*e, named);
        if (named)
            return name;
    }
    return name;
}

template <class BRIDGETYPE>
bool PAKRouter<BRIDGETYPE>::extractResources(const BRIDGETYPE& pakBridge, bool force, hecl::BlenderToken& btok,
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

            const nod::Node* node = m_node.get();

            /* Extract first, so they start out invalid */
            hecl::ProjectPath cooked = getCooked(item);
            if (force || cooked.getPathType() == hecl::ProjectPath::Type::None)
            {
                PAKEntryReadStream s = item->beginReadStream(*node);
                FILE* fout = hecl::Fopen(cooked.getAbsolutePath().c_str(), _S("wb"));
                fwrite(s.data(), 1, s.length(), fout);
                fclose(fout);
            }

            hecl::ProjectPath working = getWorking(item, extractor);
            if (extractor.func_a) /* Doesn't need PAKRouter access */
            {
                if (force || working.getPathType() == hecl::ProjectPath::Type::None)
                {
                    PAKEntryReadStream s = item->beginReadStream(*node);
                    extractor.func_a(s, working);
                }
            }
            else if (extractor.func_b) /* Needs PAKRouter access */
            {
                if (force || working.getPathType() == hecl::ProjectPath::Type::None)
                {
                    PAKEntryReadStream s = item->beginReadStream(*node);
                    extractor.func_b(m_dataSpec, s, working, *this, *item, force, btok,
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

template <class BRIDGETYPE>
const typename BRIDGETYPE::PAKType::Entry* PAKRouter<BRIDGETYPE>::lookupEntry(const IDType& entry,
                                                                              const nod::Node** nodeOut,
                                                                              bool silenceWarnings,
                                                                              bool currentPAK) const
{
    if (!entry)
        return nullptr;

    if (!m_bridges)
        LogDNACommon.report(logvisor::Fatal,
        "PAKRouter::build() must be called before PAKRouter::lookupEntry()");

    const PAKType* pak = m_pak.get();
    const nod::Node* node = m_node.get();
    if (pak)
    {
        const EntryType* ent = pak->lookupEntry(entry);
        if (ent)
        {
            if (nodeOut)
                *nodeOut = node;
            return ent;
        }
    }

    if (currentPAK)
    {
#ifndef NDEBUG
        if (!silenceWarnings)
            LogDNACommon.report(logvisor::Warning,
            "unable to find PAK entry %s in current PAK", entry.toString().c_str());
#endif
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

#ifndef NDEBUG
    if (!silenceWarnings)
        LogDNACommon.report(logvisor::Warning,
        "unable to find PAK entry %s", entry.toString().c_str());
#endif
    if (nodeOut)
        *nodeOut = nullptr;
    return nullptr;
}

template <class BRIDGETYPE>
const typename PAKRouter<BRIDGETYPE>::RigPair* PAKRouter<BRIDGETYPE>::lookupCMDLRigPair(const IDType& id) const
{
    auto search = m_cmdlRigs.find(id);
    if (search == m_cmdlRigs.end())
        return nullptr;
    return &search->second;
}

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getAreaLayerWorking(const IDType& areaId, int layerIdx) const
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

template <class BRIDGETYPE>
hecl::ProjectPath PAKRouter<BRIDGETYPE>::getAreaLayerCooked(const IDType& areaId, int layerIdx) const
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

template class PAKRouter<DNAMP1::PAKBridge>;
template class PAKRouter<DNAMP2::PAKBridge>;
template class PAKRouter<DNAMP3::PAKBridge>;

}
