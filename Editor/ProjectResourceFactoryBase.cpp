#include "ProjectResourceFactoryBase.hpp"
#include "Runtime/IObj.hpp"

#define DUMP_CACHE_FILL 1

namespace urde
{
static logvisor::Module Log("urde::ProjectResourceFactoryBase");

static void WriteTag(athena::io::YAMLDocWriter& cacheWriter,
                     const SObjectTag& pathTag, const hecl::ProjectPath& path)
{
    char idStr[9];
    snprintf(idStr, 9, "%08X", uint32_t(pathTag.id));
    if (auto v = cacheWriter.enterSubVector(idStr))
    {
        cacheWriter.writeString(nullptr, pathTag.type.toString().c_str());
        cacheWriter.writeString(nullptr, path.getAuxInfo().size() ?
            (path.getRelativePathUTF8() + '|' + path.getAuxInfoUTF8()) :
             path.getRelativePathUTF8());
    }
}

static void WriteNameTag(athena::io::YAMLDocWriter& nameWriter,
                         const SObjectTag& pathTag,
                         const std::string& name)
{
    char idStr[9];
    snprintf(idStr, 9, "%08X", uint32_t(pathTag.id));
    nameWriter.writeString(name.c_str(), idStr);
}

void ProjectResourceFactoryBase::Clear()
{
    m_tagToPath.clear();
    m_pathToTag.clear();
    m_catalogNameToTag.clear();
}

SObjectTag ProjectResourceFactoryBase::TagFromPath(const hecl::ProjectPath& path,
                                                   hecl::BlenderToken& btok) const
{
    auto search = m_pathToTag.find(path.hash());
    if (search != m_pathToTag.cend())
        return search->second;
    return BuildTagFromPath(path, btok);
}

void ProjectResourceFactoryBase::ReadCatalog(const hecl::ProjectPath& catalogPath,
                                             athena::io::YAMLDocWriter& nameWriter)
{
    athena::io::FileReader freader(catalogPath.getAbsolutePath());
    if (!freader.isOpen())
        return;

    athena::io::YAMLDocReader reader;
    bool res = reader.parse(&freader);
    if (!res)
        return;

    const athena::io::YAMLNode* root = reader.getRootNode();
    for (const auto& p : root->m_mapChildren)
    {
        /* Hash as lowercase since lookup is case-insensitive */
        std::string pLower = p.first;
        std::transform(pLower.cbegin(), pLower.cend(), pLower.begin(), tolower);

        /* Avoid redundant filesystem access for re-caches */
        if (m_catalogNameToTag.find(pLower) != m_catalogNameToTag.cend())
            continue;

        athena::io::YAMLNode& node = *p.second;
        hecl::ProjectPath path;
        if (node.m_type == YAML_SCALAR_NODE)
        {
            path = hecl::ProjectPath(m_proj->getProjectWorkingPath(), node.m_scalarString);
        }
        else if (node.m_type == YAML_SEQUENCE_NODE)
        {
            if (node.m_seqChildren.size() >= 2)
                path = hecl::ProjectPath(m_proj->getProjectWorkingPath(), node.m_seqChildren[0]->m_scalarString).
                        ensureAuxInfo(node.m_seqChildren[1]->m_scalarString);
            else if (node.m_seqChildren.size() == 1)
                path = hecl::ProjectPath(m_proj->getProjectWorkingPath(), node.m_seqChildren[0]->m_scalarString);
        }
        if (!path.isFileOrGlob())
            continue;
        SObjectTag pathTag = TagFromPath(path, m_backgroundBlender);
        if (pathTag)
        {
            std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
            m_catalogNameToTag[pLower] = pathTag;
            WriteNameTag(nameWriter, pathTag, p.first);
#if 0
            fprintf(stderr, "%s %s %08X\n",
                    p.first.c_str(),
                    pathTag.type.toString().c_str(), uint32_t(pathTag.id));
#endif
        }
    }
}

void ProjectResourceFactoryBase::BackgroundIndexRecursiveCatalogs(const hecl::ProjectPath& dir,
                                                                  athena::io::YAMLDocWriter& nameWriter,
                                                                  int level)
{
    hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    false, false, true);

    /* Enumerate all items */
    for (const hecl::DirectoryEnumerator::Entry& ent : dEnum)
    {
        hecl::ProjectPath path(dir, ent.m_name);
        if (ent.m_isDir && level < 1)
            BackgroundIndexRecursiveCatalogs(path, nameWriter, level+1);
        else
        {
            if (!path.isFile())
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("!catalog.yaml")))
            {
                ReadCatalog(path, nameWriter);
                continue;
            }
        }

        /* bail if cancelled by client */
        if (!m_backgroundRunning)
            break;
    }
}

#if DUMP_CACHE_FILL
static void DumpCacheAdd(const SObjectTag& pathTag, const hecl::ProjectPath& path)
{
    fprintf(stderr, "%s %08X %s\n",
            pathTag.type.toString().c_str(), uint32_t(pathTag.id),
            path.getRelativePathUTF8().c_str());
}
#endif

bool ProjectResourceFactoryBase::AddFileToIndex(const hecl::ProjectPath& path,
                                                athena::io::YAMLDocWriter& cacheWriter)
{
    /* Avoid redundant filesystem access for re-caches */
    if (m_pathToTag.find(path.hash()) != m_pathToTag.cend())
        return true;

    /* Try as glob */
    hecl::ProjectPath asGlob = path.getWithExtension(_S(".*"), true);
    if (m_pathToTag.find(asGlob.hash()) != m_pathToTag.cend())
        return true;

    /* Classify intermediate into tag */
    SObjectTag pathTag = BuildTagFromPath(path, m_backgroundBlender);
    if (pathTag)
    {
        std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
        bool useGlob = false;

        /* Special multi-resource intermediates */
        if (pathTag.type == SBIG('ANCS'))
        {
            hecl::BlenderConnection& conn = m_backgroundBlender.getBlenderConnection();
            if (!conn.openBlend(path) || conn.getBlendType() != hecl::BlenderConnection::BlendType::Actor)
                return false;

            /* Transform tag to glob */
            pathTag = {SBIG('ANCS'), asGlob.hash().val32()};
            useGlob = true;

            hecl::BlenderConnection::DataStream ds = conn.beginData();
            std::vector<std::string> armatureNames = ds.getArmatureNames();
            std::vector<std::string> subtypeNames = ds.getSubtypeNames();
            std::vector<std::string> actionNames = ds.getActionNames();

            for (const std::string& arm : armatureNames)
            {
                hecl::SystemStringView sysStr(arm);
                hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S(".CINF"));
                SObjectTag pathTag = BuildTagFromPath(subPath, m_backgroundBlender);
                m_tagToPath[pathTag] = subPath;
                m_pathToTag[subPath.hash()] = pathTag;
                WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                DumpCacheAdd(pathTag, subPath);
#endif
            }

            for (const std::string& sub : subtypeNames)
            {
                hecl::SystemStringView sysStr(sub);
                hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S(".CSKR"));
                SObjectTag pathTag = BuildTagFromPath(subPath, m_backgroundBlender);
                m_tagToPath[pathTag] = subPath;
                m_pathToTag[subPath.hash()] = pathTag;
                WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                DumpCacheAdd(pathTag, subPath);
#endif
            }

            for (const std::string& act : actionNames)
            {
                hecl::SystemStringView sysStr(act);
                hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S(".ANIM"));
                SObjectTag pathTag = BuildTagFromPath(subPath, m_backgroundBlender);
                m_tagToPath[pathTag] = subPath;
                m_pathToTag[subPath.hash()] = pathTag;
                WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                DumpCacheAdd(pathTag, subPath);
#endif
            }
        }
        else if (pathTag.type == SBIG('MLVL'))
        {
            /* Transform tag to glob */
            pathTag = {SBIG('MLVL'), asGlob.hash().val32()};
            useGlob = true;

            hecl::ProjectPath subPath = asGlob.ensureAuxInfo(_S("MAPW"));
            SObjectTag pathTag = BuildTagFromPath(subPath, m_backgroundBlender);
            m_tagToPath[pathTag] = subPath;
            m_pathToTag[subPath.hash()] = pathTag;
            WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
            DumpCacheAdd(pathTag, subPath);
#endif

            subPath = asGlob.ensureAuxInfo(_S("SAVW"));
            pathTag = BuildTagFromPath(subPath, m_backgroundBlender);
            m_tagToPath[pathTag] = subPath;
            m_pathToTag[subPath.hash()] = pathTag;
            WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
            DumpCacheAdd(pathTag, subPath);
#endif
        }
        else if (pathTag.type == SBIG('AGSC'))
        {
            /* Transform tag to glob */
            pathTag = {SBIG('AGSC'), asGlob.hash().val32()};
            useGlob = true;
        }
        else if (pathTag.type == SBIG('MREA'))
        {
            hecl::ProjectPath subPath = path.ensureAuxInfo(_S("PATH"));
            SObjectTag pathTag = BuildTagFromPath(subPath, m_backgroundBlender);
            m_tagToPath[pathTag] = subPath;
            m_pathToTag[subPath.hash()] = pathTag;
            WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
            DumpCacheAdd(pathTag, subPath);
#endif
        }

        /* Cache in-memory */
        const hecl::ProjectPath& usePath = useGlob ? asGlob : path;
        m_tagToPath[pathTag] = usePath;
        m_pathToTag[usePath.hash()] = pathTag;
        WriteTag(cacheWriter, pathTag, usePath);
#if DUMP_CACHE_FILL
        DumpCacheAdd(pathTag, usePath);
#endif
    }

    return true;
}

void ProjectResourceFactoryBase::BackgroundIndexRecursiveProc(const hecl::ProjectPath& dir,
                                                              athena::io::YAMLDocWriter& cacheWriter,
                                                              athena::io::YAMLDocWriter& nameWriter,
                                                              int level)
{
    hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    false, false, true);

    /* Enumerate all items */
    for (const hecl::DirectoryEnumerator::Entry& ent : dEnum)
    {
        hecl::ProjectPath path(dir, ent.m_name);
        if (ent.m_isDir)
            BackgroundIndexRecursiveProc(path, cacheWriter, nameWriter, level+1);
        else
        {
            if (!path.isFile())
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("!catalog.yaml")))
            {
                ReadCatalog(path, nameWriter);
                continue;
            }

            /* Index the regular file */
            AddFileToIndex(path, cacheWriter);
        }

        /* bail if cancelled by client */
        if (!m_backgroundRunning)
            break;
    }
}

void ProjectResourceFactoryBase::BackgroundIndexProc()
{
    logvisor::RegisterThreadName("Resource Index Thread");

    hecl::ProjectPath tagCachePath(m_proj->getProjectCookedPath(*m_origSpec), _S("tag_cache.yaml"));
    hecl::ProjectPath nameCachePath(m_proj->getProjectCookedPath(*m_origSpec), _S("name_cache.yaml"));
    hecl::ProjectPath specRoot(m_proj->getProjectWorkingPath(), m_origSpec->m_name);

    /* Cache will be overwritten with validated entries afterwards */
    athena::io::YAMLDocWriter cacheWriter(nullptr);
    athena::io::YAMLDocWriter nameWriter(nullptr);

    /* Read in tag cache */
    if (tagCachePath.isFile())
    {
        athena::io::FileReader reader(tagCachePath.getAbsolutePath());
        if (reader.isOpen())
        {
            Log.report(logvisor::Info, _S("Cache index of '%s' loading"), m_origSpec->m_name);
            athena::io::YAMLDocReader cacheReader;
            if (cacheReader.parse(&reader))
            {
                std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                size_t tagCount = cacheReader.getRootNode()->m_mapChildren.size();
                m_tagToPath.reserve(tagCount);
                m_pathToTag.reserve(tagCount);
                size_t loadIdx = 0;
                for (const auto& child : cacheReader.getRootNode()->m_mapChildren)
                {
                    const athena::io::YAMLNode& node = *child.second;
                    unsigned long id = strtoul(child.first.c_str(), nullptr, 16);
                    hecl::FourCC type(node.m_seqChildren.at(0)->m_scalarString.c_str());
                    hecl::ProjectPath path(m_proj->getProjectWorkingPath(),
                        node.m_seqChildren.at(1)->m_scalarString);

                    if (path.isFileOrGlob())
                    {
                        SObjectTag pathTag(type, id);
                        m_tagToPath[pathTag] = path;
                        m_pathToTag[path.hash()] = pathTag;
                        WriteTag(cacheWriter, pathTag, path);
                    }
                    fprintf(stderr, "\r %" PRISize " / %" PRISize, ++loadIdx,
                            cacheReader.getRootNode()->m_mapChildren.size());
                }
                fprintf(stderr, "\n");
            }
            Log.report(logvisor::Info, _S("Cache index of '%s' loaded; %d tags"),
                       m_origSpec->m_name, m_tagToPath.size());

            if (nameCachePath.isFile())
            {
                /* Read in name cache */
                Log.report(logvisor::Info, _S("Name index of '%s' loading"), m_origSpec->m_name);
                athena::io::FileReader nreader(nameCachePath.getAbsolutePath());
                athena::io::YAMLDocReader nameReader;
                if (nameReader.parse(&nreader))
                {
                    std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                    m_catalogNameToTag.reserve(nameReader.getRootNode()->m_mapChildren.size());
                    for (const auto& child : nameReader.getRootNode()->m_mapChildren)
                    {
                        unsigned long id = strtoul(child.second->m_scalarString.c_str(), nullptr, 16);
                        auto search = m_tagToPath.find(SObjectTag(FourCC(), uint32_t(id)));
                        if (search != m_tagToPath.cend())
                        {
                            std::string chLower = child.first;
                            std::transform(chLower.cbegin(), chLower.cend(), chLower.begin(), tolower);
                            m_catalogNameToTag[chLower] = search->first;
                            WriteNameTag(nameWriter, search->first, child.first);
                        }
                    }
                }
                Log.report(logvisor::Info, _S("Name index of '%s' loaded; %d names"),
                           m_origSpec->m_name, m_catalogNameToTag.size());
            }
        }
    }

    /* Add special original IDs resource if exists (not name-cached to disk) */
    hecl::ProjectPath oidsPath(specRoot, "!original_ids.yaml");
    SObjectTag oidsTag = BuildTagFromPath(oidsPath, m_backgroundBlender);
    if (oidsTag)
        m_catalogNameToTag["mp1originalids"] = oidsTag;

    Log.report(logvisor::Info, _S("Background index of '%s' started"), m_origSpec->m_name);
    BackgroundIndexRecursiveProc(specRoot, cacheWriter, nameWriter, 0);

    tagCachePath.makeDirChain(false);
    athena::io::FileWriter twriter(tagCachePath.getAbsolutePath());
    cacheWriter.finish(&twriter);

    athena::io::FileWriter nwriter(nameCachePath.getAbsolutePath());
    nameWriter.finish(&nwriter);

    m_backgroundBlender.shutdown();
    Log.report(logvisor::Info, _S("Background index of '%s' complete; %d tags, %d names"),
               m_origSpec->m_name, m_tagToPath.size(), m_catalogNameToTag.size());
    m_backgroundRunning = false;
}

void ProjectResourceFactoryBase::CancelBackgroundIndex()
{
    m_backgroundRunning = false;
    if (m_backgroundIndexTh.joinable())
        m_backgroundIndexTh.join();
}

void ProjectResourceFactoryBase::BeginBackgroundIndex
    (hecl::Database::Project& proj,
     const hecl::Database::DataSpecEntry& origSpec,
     const hecl::Database::DataSpecEntry& pcSpec)
{
    CancelBackgroundIndex();
    Clear();
    m_proj = &proj;
    m_origSpec = &origSpec;
    m_pcSpec = &pcSpec;
    m_cookSpec.reset(pcSpec.m_factory(proj, hecl::Database::DataSpecTool::Cook));
    m_backgroundRunning = true;
    m_backgroundIndexTh =
        std::thread(std::bind(&ProjectResourceFactoryBase::BackgroundIndexProc, this));
}

hecl::ProjectPath ProjectResourceFactoryBase::GetCookedPath(const hecl::ProjectPath& working,
                                                            bool pcTarget) const
{
    const hecl::Database::DataSpecEntry* spec = m_origSpec;
    if (pcTarget)
        spec = m_cookSpec->overrideDataSpec(working, m_pcSpec, hecl::SharedBlenderToken);
    if (!spec)
        return {};
    return working.getCookedPath(*spec);
}

bool ProjectResourceFactoryBase::SyncCook(const hecl::ProjectPath& working)
{
    Log.report(logvisor::Warning, _S("sync-cooking %s"), working.getRelativePath().c_str());
    return m_clientProc.syncCook(working, m_cookSpec.get(), hecl::SharedBlenderToken);
}

CFactoryFnReturn ProjectResourceFactoryBase::BuildSync(const SObjectTag& tag,
                                                       const hecl::ProjectPath& path,
                                                       const CVParamTransfer& paramXfer,
                                                       CObjectReference* selfRef)
{
    /* Ensure cooked rep is on the filesystem */
    std::experimental::optional<athena::io::FileReader> fr;
    if (!PrepForReadSync(tag, path, fr))
        return {};

    /* All good, build resource */
    if (m_factoryMgr.CanMakeMemory(tag))
    {
        u32 length = fr->length();
        std::unique_ptr<u8[]> memBuf = fr->readUBytes(length);
        CFactoryFnReturn ret =
            m_factoryMgr.MakeObjectFromMemory(tag, std::move(memBuf), length, false, paramXfer, selfRef);
        Log.report(logvisor::Info, "sync-built %.4s %08X",
                   tag.type.toString().c_str(), u32(tag.id));
        return ret;
    }

    CFactoryFnReturn ret = m_factoryMgr.MakeObject(tag, *fr, paramXfer, selfRef);
    Log.report(logvisor::Info, "sync-built %.4s %08X",
               tag.type.toString().c_str(), u32(tag.id));
    return ret;
}

void ProjectResourceFactoryBase::AsyncTask::EnsurePath(const urde::SObjectTag& tag,
                                                       const hecl::ProjectPath& path)
{
    if (!m_workingPath)
    {
        m_workingPath = path;

        /* Ensure requested resource is on the filesystem */
        if (!path.isFileOrGlob())
        {
            Log.report(logvisor::Error, _S("unable to find resource path '%s'"),
                       path.getRelativePath().c_str());
            m_failed = true;
            return;
        }

        /* Cached resolve (try PC first, then original) */
        m_cookedPath = path.getCookedPath(*m_parent.m_pcSpec);
        if (!m_cookedPath.isFile())
            m_cookedPath = path.getCookedPath(*m_parent.m_origSpec);
        if (!m_cookedPath.isFile() ||
            m_cookedPath.getModtime() < path.getModtime())
        {
            /* Last chance type validation */
            urde::SObjectTag verifyTag = m_parent.TagFromPath(path, hecl::SharedBlenderToken);
            if (verifyTag.type != tag.type)
            {
                Log.report(logvisor::Error, _S("%s: expected type '%.4s', found '%.4s'"),
                           path.getRelativePath().c_str(),
                           tag.type.toString().c_str(), verifyTag.type.toString().c_str());
                m_failed = true;
                return;
            }

            /* Get cooked representation path */
            m_cookedPath = m_parent.GetCookedPath(path, true);

            /* Perform mod-time comparison */
            if (!m_cookedPath.isFile() ||
                m_cookedPath.getModtime() < path.getModtime())
            {
                /* Start a background cook here */
                m_cookTransaction = m_parent.m_clientProc.addCookTransaction(path, m_parent.m_cookSpec.get());
                return;
            }
        }

        CookComplete();
    }
}

void ProjectResourceFactoryBase::AsyncTask::CookComplete()
{
    /* Ensure cooked rep is on the filesystem */
    athena::io::FileReader fr(m_cookedPath.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
    {
        Log.report(logvisor::Error, _S("unable to open cooked resource path '%s'"),
                   m_cookedPath.getAbsolutePath().c_str());
        m_failed = true;
        return;
    }

    /* Ready for buffer transaction at this point */
    u32 availSz = std::max(0, s32(fr.length()) - s32(x14_resOffset));
    x14_resSize = std::min(x14_resSize, availSz);
    if (xc_targetDataRawPtr)
    {
        m_bufTransaction = m_parent.m_clientProc.addBufferTransaction(m_cookedPath,
                                                                      xc_targetDataRawPtr,
                                                                      x14_resSize, x14_resOffset);
    }
    else
    {
        x10_loadBuffer.reset(new u8[x14_resSize]);
        m_bufTransaction = m_parent.m_clientProc.addBufferTransaction(m_cookedPath,
                                                                      x10_loadBuffer.get(),
                                                                      x14_resSize, x14_resOffset);
    }
}

bool ProjectResourceFactoryBase::AsyncTask::AsyncPump()
{
    if (m_failed)
        return true;

    if (m_bufTransaction)
    {
        if (m_bufTransaction->m_complete)
        {
            m_complete = true;
            return true;
        }
    }
    else if (m_cookTransaction)
    {
        if (m_cookTransaction->m_complete)
            CookComplete();
    }

    return m_failed;
}

void ProjectResourceFactoryBase::AsyncTask::WaitForComplete()
{
    using ItType = std::unordered_map<SObjectTag, std::shared_ptr<AsyncTask>>::iterator;
    ItType search = m_parent.m_asyncLoadList.find(x0_tag);
    if (search == m_parent.m_asyncLoadList.end())
        return;
    for (ItType tmp = search ; !m_parent.AsyncPumpTask(tmp) ; tmp = search)
    {std::this_thread::sleep_for(std::chrono::milliseconds(2));}
}

bool ProjectResourceFactoryBase::WaitForTagReady(const urde::SObjectTag& tag,
                                                 const hecl::ProjectPath*& pathOut)
{
    std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
    auto search = m_tagToPath.find(tag);
    if (search == m_tagToPath.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                lk.lock();
                search = m_tagToPath.find(tag);
                if (search != m_tagToPath.end())
                    break;
            }
            if (search == m_tagToPath.end())
                return false;
        }
        else
            return false;
    }
    lk.unlock();
    pathOut = &search->second;
    return true;
}

bool
ProjectResourceFactoryBase::PrepForReadSync(const SObjectTag& tag,
                                            const hecl::ProjectPath& path,
                                            std::experimental::optional<athena::io::FileReader>& fr)
{
    /* Ensure requested resource is on the filesystem */
    if (!path.isFileOrGlob())
    {
        Log.report(logvisor::Error, _S("unable to find resource path '%s'"),
                   path.getAbsolutePath().c_str());
        return false;
    }

    /* Cached resolve (try PC first, then original) */
    hecl::ProjectPath cooked = path.getCookedPath(*m_pcSpec);
    if (!cooked.isFile())
        cooked = path.getCookedPath(*m_origSpec);
    if (!cooked.isFile() ||
        cooked.getModtime() < path.getModtime())
    {
        /* Last chance type validation */
        urde::SObjectTag verifyTag = TagFromPath(path, hecl::SharedBlenderToken);
        if (verifyTag.type != tag.type)
        {
            Log.report(logvisor::Error, _S("%s: expected type '%.4s', found '%.4s'"),
                       path.getRelativePath().c_str(),
                       tag.type.toString().c_str(), verifyTag.type.toString().c_str());
            return false;
        }

        /* Get cooked representation path */
        cooked = GetCookedPath(path, true);

        /* Perform mod-time comparison */
        if (!cooked.isFile() ||
            cooked.getModtime() < path.getModtime())
        {
            /* Do a blocking cook here */
            if (!SyncCook(path))
            {
                Log.report(logvisor::Error, _S("unable to cook resource path '%s'"),
                           path.getAbsolutePath().c_str());
                return false;
            }
        }
    }

    /* Ensure cooked rep is on the filesystem */
    fr.emplace(cooked.getAbsolutePath(), 32 * 1024, false);
    if (fr->hasError())
    {
        Log.report(logvisor::Error, _S("unable to open cooked resource path '%s'"),
                   cooked.getAbsolutePath().c_str());
        return false;
    }

    return true;
}

std::unique_ptr<urde::IObj> ProjectResourceFactoryBase::Build(const urde::SObjectTag& tag,
                                                              const urde::CVParamTransfer& paramXfer,
                                                              CObjectReference* selfRef)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");

    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return {};
    auto asyncSearch = m_asyncLoadList.find(tag);
    if (asyncSearch != m_asyncLoadList.end())
    {
        /* Async spinloop */
        AsyncTask& task = *asyncSearch->second;
        task.EnsurePath(task.x0_tag, *resPath);

        /* Pump load pipeline (cooking if needed) */
        while (!task.AsyncPump()) {std::this_thread::sleep_for(std::chrono::milliseconds(2));}

        if (task.m_complete)
        {
            /* Load complete, build resource */
            std::unique_ptr<IObj> newObj;
            if (m_factoryMgr.CanMakeMemory(task.x0_tag))
            {
                newObj = m_factoryMgr.MakeObjectFromMemory(tag, std::move(task.x10_loadBuffer),
                                                           task.x14_resSize, false, task.x18_cvXfer, selfRef);
            }
            else
            {
                athena::io::MemoryReader mr(task.x10_loadBuffer.get(), task.x14_resSize);
                newObj = m_factoryMgr.MakeObject(task.x0_tag, mr, task.x18_cvXfer, selfRef);
            }

            *task.xc_targetObjPtr = newObj.get();
            Log.report(logvisor::Warning, "spin-built %.4s %08X",
                       task.x0_tag.type.toString().c_str(),
                       u32(task.x0_tag.id));

            m_asyncLoadList.erase(asyncSearch);
            return newObj;
        }
        Log.report(logvisor::Error, "unable to spin-build %.4s %08X",
                   task.x0_tag.type.toString().c_str(),
                   u32(task.x0_tag.id));
        m_asyncLoadList.erase(asyncSearch);
        return {};
    }

    /* Fall-back to sync build */
    return BuildSync(tag, *resPath, paramXfer, selfRef);
}

std::shared_ptr<ProjectResourceFactoryBase::AsyncTask>
ProjectResourceFactoryBase::BuildAsyncInternal(const urde::SObjectTag& tag,
                                               const urde::CVParamTransfer& paramXfer,
                                               urde::IObj** objOut,
                                               CObjectReference* selfRef)
{
    if (m_asyncLoadList.find(tag) != m_asyncLoadList.end())
        return {};
    return m_asyncLoadList.emplace(std::make_pair(tag,
        std::make_unique<AsyncTask>(*this, tag, objOut, paramXfer, selfRef))).first->second;
}

void ProjectResourceFactoryBase::BuildAsync(const urde::SObjectTag& tag,
                                            const urde::CVParamTransfer& paramXfer,
                                            urde::IObj** objOut,
                                            CObjectReference* selfRef)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");

    BuildAsyncInternal(tag, paramXfer, objOut, selfRef);
}

u32 ProjectResourceFactoryBase::ResourceSize(const SObjectTag& tag)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");

    /* Ensure resource at requested path is indexed and not cooking */
    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return {};

    /* Ensure cooked rep is on the filesystem */
    std::experimental::optional<athena::io::FileReader> fr;
    if (!PrepForReadSync(tag, *resPath, fr))
        return {};

    return fr->length();
}

std::shared_ptr<ProjectResourceFactoryBase::AsyncTask>
ProjectResourceFactoryBase::LoadResourceAsync(const urde::SObjectTag& tag,
                                              std::unique_ptr<u8[]>& target)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");
    if (m_asyncLoadList.find(tag) != m_asyncLoadList.end())
        return {};
    return m_asyncLoadList.emplace(std::make_pair(tag,
        std::make_shared<AsyncTask>(*this, tag, target))).first->second;
}

std::shared_ptr<ProjectResourceFactoryBase::AsyncTask>
ProjectResourceFactoryBase::LoadResourcePartAsync(const urde::SObjectTag& tag,
                                                  u32 size, u32 off,
                                                  std::unique_ptr<u8[]>& target)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");
    if (m_asyncLoadList.find(tag) != m_asyncLoadList.end())
        return {};
    return m_asyncLoadList.emplace(std::make_pair(tag,
        std::make_shared<AsyncTask>(*this, tag, target, size, off))).first->second;
}

std::shared_ptr<ProjectResourceFactoryBase::AsyncTask>
ProjectResourceFactoryBase::LoadResourcePartAsync(const urde::SObjectTag& tag, u32 size, u32 off, u8* target)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");
    if (m_asyncLoadList.find(tag) != m_asyncLoadList.end())
        return {};
    return m_asyncLoadList.emplace(std::make_pair(tag,
        std::make_shared<AsyncTask>(*this, tag, target, size, off))).first->second;
}

std::unique_ptr<u8[]> ProjectResourceFactoryBase::LoadResourceSync(const urde::SObjectTag& tag)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");

    /* Ensure resource at requested path is indexed and not cooking */
    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return {};

    /* Ensure cooked rep is on the filesystem */
    std::experimental::optional<athena::io::FileReader> fr;
    if (!PrepForReadSync(tag, *resPath, fr))
        return {};

    return fr->readUBytes(fr->length());
}

std::unique_ptr<u8[]> ProjectResourceFactoryBase::LoadResourcePartSync(const urde::SObjectTag& tag,
                                                                       u32 size, u32 off)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");

    /* Ensure resource at requested path is indexed and not cooking */
    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return {};

    /* Ensure cooked rep is on the filesystem */
    std::experimental::optional<athena::io::FileReader> fr;
    if (!PrepForReadSync(tag, *resPath, fr))
        return {};

    s32 sz = std::min(s32(size), std::max(0, s32(fr->length()) - s32(off)));
    fr->seek(off, athena::SeekOrigin::Begin);
    return fr->readUBytes(sz);
}

void ProjectResourceFactoryBase::CancelBuild(const urde::SObjectTag& tag)
{
    m_asyncLoadList.erase(tag);
}

bool ProjectResourceFactoryBase::CanBuild(const urde::SObjectTag& tag)
{
    if ((tag.id & 0xffffffff) == 0xffffffff || !tag.id)
        Log.report(logvisor::Fatal, "attempted to access null id");

    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return false;

    if (resPath->isFile())
        return true;

    return false;
}

const urde::SObjectTag* ProjectResourceFactoryBase::GetResourceIdByName(const char* name) const
{
    std::string lower = name;
    std::transform(lower.cbegin(), lower.cend(), lower.begin(), tolower);

    std::unique_lock<std::mutex> lk(const_cast<ProjectResourceFactoryBase*>(this)->m_backgroundIndexMutex);
    auto search = m_catalogNameToTag.find(lower);
    if (search == m_catalogNameToTag.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                lk.lock();
                search = m_catalogNameToTag.find(lower);
                if (search != m_catalogNameToTag.end())
                    break;
            }
            if (search == m_catalogNameToTag.end())
                return nullptr;
        }
        else
            return nullptr;
    }
    return &search->second;
}

FourCC ProjectResourceFactoryBase::GetResourceTypeById(ResId id) const
{
    if ((id & 0xffffffff) == 0xffffffff || !id)
        return {};

    std::unique_lock<std::mutex> lk(const_cast<ProjectResourceFactoryBase*>(this)->m_backgroundIndexMutex);
    SObjectTag searchTag = {FourCC(), id};
    auto search = m_tagToPath.find(searchTag);
    if (search == m_tagToPath.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                lk.lock();
                search = m_tagToPath.find(searchTag);
                if (search != m_tagToPath.end())
                    break;
            }
            if (search == m_tagToPath.end())
                return {};
        }
        else
            return {};
    }

    return search->first.type;
}

void ProjectResourceFactoryBase::EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const
{
    std::unique_lock<std::mutex> lk(const_cast<ProjectResourceFactoryBase*>(this)->m_backgroundIndexMutex);
    while (m_backgroundRunning)
    {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        lk.lock();
    }
    for (const auto& pair : m_tagToPath)
    {
        if (!lambda(pair.first))
            break;
    }
}

void ProjectResourceFactoryBase::EnumerateNamedResources(
        const std::function<bool(const std::string&, const SObjectTag&)>& lambda) const
{
    std::unique_lock<std::mutex> lk(const_cast<ProjectResourceFactoryBase*>(this)->m_backgroundIndexMutex);
    while (m_backgroundRunning)
    {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        lk.lock();
    }
    lk.unlock();
    for (const auto& pair : m_catalogNameToTag)
    {
        if (!lambda(pair.first, pair.second))
            break;
    }
}

bool ProjectResourceFactoryBase::AsyncPumpTask(
    std::unordered_map<SObjectTag, std::shared_ptr<AsyncTask>>::iterator& it)
{
    /* Ensure requested resource is in the index */
    std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
    AsyncTask& task = *it->second;
    auto search = m_tagToPath.find(task.x0_tag);
    if (search == m_tagToPath.end())
    {
        if (!m_backgroundRunning)
        {
            Log.report(logvisor::Error, _S("unable to find async load resource (%s, %08X)"),
                       task.x0_tag.type.toString().c_str(), task.x0_tag.id);
            it = m_asyncLoadList.erase(it);
        }
        return true;
    }
    lk.unlock();
    task.EnsurePath(task.x0_tag, search->second);

    /* Pump load pipeline (cooking if needed) */
    if (task.AsyncPump())
    {
        if (task.m_complete)
        {
            /* Load complete, build resource */
            if (task.xc_targetObjPtr)
            {
                /* Factory build */
                std::unique_ptr<IObj> newObj;
                if (m_factoryMgr.CanMakeMemory(task.x0_tag))
                {
                    newObj = m_factoryMgr.MakeObjectFromMemory(task.x0_tag, std::move(task.x10_loadBuffer),
                                                               task.x14_resSize, false, task.x18_cvXfer,
                                                               task.m_selfRef);
                }
                else
                {
                    athena::io::MemoryReader mr(task.x10_loadBuffer.get(), task.x14_resSize);
                    newObj = m_factoryMgr.MakeObject(task.x0_tag, mr, task.x18_cvXfer, task.m_selfRef);
                }

                *task.xc_targetObjPtr = newObj.release();
                Log.report(logvisor::Info, "async-built %.4s %08X",
                           task.x0_tag.type.toString().c_str(),
                           u32(task.x0_tag.id));
            }
            else if (task.xc_targetDataPtr)
            {
                /* Buffer only */
                *task.xc_targetDataPtr = std::move(task.x10_loadBuffer);
                Log.report(logvisor::Info, "async-loaded %.4s %08X",
                           task.x0_tag.type.toString().c_str(),
                           u32(task.x0_tag.id));
            }
            else if (task.xc_targetDataRawPtr)
            {
                /* Buffer only raw */
                Log.report(logvisor::Info, "async-loaded %.4s %08X",
                           task.x0_tag.type.toString().c_str(),
                           u32(task.x0_tag.id));
            }
        }

        it = m_asyncLoadList.erase(it);
        return true;
    }
    ++it;
    return false;
}

void ProjectResourceFactoryBase::AsyncIdle()
{
    /* Consume completed transactions, they will be processed this cycle at the latest */
    std::list<std::shared_ptr<hecl::ClientProcess::Transaction>> completed;
    m_clientProc.swapCompletedQueue(completed);

    /* Begin self-profiling loop */
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    for (auto it=m_asyncLoadList.begin() ; it != m_asyncLoadList.end() ;)
    {
        /* Allow 8 milliseconds (roughly 1/2 frame-time) for each async build cycle */
        std::chrono::steady_clock::time_point resStart = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(resStart - start).count() > 8)
            break;

        AsyncPumpTask(it);
    }
}

}
