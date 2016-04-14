#include "ProjectResourceFactoryBase.hpp"
#include "Runtime/IObj.hpp"

namespace urde
{
static logvisor::Module Log("urde::ProjectResourceFactoryBase");

void ProjectResourceFactoryBase::Clear()
{
    m_tagToPath.clear();
    m_catalogNameToTag.clear();
}

void ProjectResourceFactoryBase::ReadCatalog(const hecl::ProjectPath& catalogPath,
                                             athena::io::YAMLDocWriter& nameWriter)
{
    FILE* fp = hecl::Fopen(catalogPath.getAbsolutePath().c_str(), _S("r"));
    if (!fp)
        return;

    athena::io::YAMLDocReader reader;
    yaml_parser_set_input_file(reader.getParser(), fp);
    bool res = reader.parse();
    fclose(fp);
    if (!res)
        return;

    const athena::io::YAMLNode* root = reader.getRootNode();
    for (const auto& p : root->m_mapChildren)
    {
        hecl::ProjectPath path(m_proj->getProjectWorkingPath(), p.second->m_scalarString);
        if (path.getPathType() != hecl::ProjectPath::Type::File)
            continue;
        SObjectTag pathTag = TagFromPath(path, m_backgroundBlender);
        if (pathTag)
        {
            std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
            m_catalogNameToTag[p.first] = pathTag;

            char idStr[9];
            snprintf(idStr, 9, "%08X", uint32_t(pathTag.id));
            nameWriter.writeString(p.first.c_str(), idStr);
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
            if (path.getPathType() != hecl::ProjectPath::Type::File)
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("catalog.yaml")))
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

static void WriteTag(athena::io::YAMLDocWriter& cacheWriter,
                     const SObjectTag& pathTag, const hecl::ProjectPath& path)
{
    char idStr[9];
    snprintf(idStr, 9, "%08X", uint32_t(pathTag.id));
    cacheWriter.enterSubVector(idStr);
    cacheWriter.writeString(nullptr, pathTag.type.toString().c_str());
    cacheWriter.writeString(nullptr, path.getRelativePathUTF8().c_str());
    cacheWriter.leaveSubVector();
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
            if (path.getPathType() != hecl::ProjectPath::Type::File)
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("catalog.yaml")))
            {
                ReadCatalog(path, nameWriter);
                continue;
            }

            /* Classify intermediate into tag */
            SObjectTag pathTag = TagFromPath(path, m_backgroundBlender);
            if (pathTag)
            {
                std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                m_tagToPath[pathTag] = path;
                WriteTag(cacheWriter, pathTag, path);
#if 1
                fprintf(stderr, "%s %08X %s\n",
                        pathTag.type.toString().c_str(), uint32_t(pathTag.id),
                        path.getRelativePathUTF8().c_str());
#endif

                /* Special multi-resource intermediates */
                if (pathTag.type == SBIG('ANCS'))
                {
                    hecl::BlenderConnection& conn = m_backgroundBlender.getBlenderConnection();
                    if (!conn.openBlend(path) || conn.getBlendType() != hecl::BlenderConnection::BlendType::Actor)
                        continue;

                    hecl::BlenderConnection::DataStream ds = conn.beginData();
                    std::vector<std::string> armatureNames = ds.getArmatureNames();
                    std::vector<std::string> subtypeNames = ds.getSubtypeNames();
                    std::vector<std::string> actionNames = ds.getActionNames();

                    for (const std::string& arm : armatureNames)
                    {
                        hecl::SystemStringView sysStr(arm);
                        hecl::ProjectPath subPath = path.getWithExtension((_S('.') + sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("CINF"));
                        SObjectTag pathTag = TagFromPath(subPath, m_backgroundBlender);
                        if (pathTag)
                        {
                            m_tagToPath[pathTag] = path;
                            WriteTag(cacheWriter, pathTag, path);
                        }
                    }

                    for (const std::string& sub : subtypeNames)
                    {
                        hecl::SystemStringView sysStr(sub);
                        hecl::ProjectPath subPath = path.getWithExtension((_S('.') + sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("CSKR"));
                        SObjectTag pathTag = TagFromPath(subPath, m_backgroundBlender);
                        if (pathTag)
                        {
                            m_tagToPath[pathTag] = path;
                            WriteTag(cacheWriter, pathTag, path);
                        }
                    }

                    for (const std::string& act : actionNames)
                    {
                        hecl::SystemStringView sysStr(act);
                        hecl::ProjectPath subPath = path.getWithExtension((_S('.') + sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("ANIM"));
                        SObjectTag pathTag = TagFromPath(subPath, m_backgroundBlender);
                        if (pathTag)
                        {
                            m_tagToPath[pathTag] = path;
                            WriteTag(cacheWriter, pathTag, path);
                        }
                    }
                }
            }
        }

        /* bail if cancelled by client */
        if (!m_backgroundRunning)
            break;
    }
}

void ProjectResourceFactoryBase::BackgroundIndexProc()
{
    hecl::ProjectPath tagCachePath(m_proj->getProjectCookedPath(*m_origSpec), _S("tag_cache.yaml"));
    hecl::ProjectPath nameCachePath(m_proj->getProjectCookedPath(*m_origSpec), _S("name_cache.yaml"));
    hecl::ProjectPath specRoot(m_proj->getProjectWorkingPath(), m_origSpec->m_name);

    /* Read in tag cache */
    if (tagCachePath.getPathType() == hecl::ProjectPath::Type::File)
    {
        FILE* cacheFile = hecl::Fopen(tagCachePath.getAbsolutePath().c_str(), _S("r"));
        if (cacheFile)
        {
            Log.report(logvisor::Info, _S("Cache index of '%s' loading"), m_origSpec->m_name);
            athena::io::YAMLDocReader cacheReader;
            yaml_parser_set_input_file(cacheReader.getParser(), cacheFile);
            if (cacheReader.parse())
            {
                std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                m_tagToPath.reserve(cacheReader.getRootNode()->m_mapChildren.size());
                size_t loadIdx = 0;
                for (const auto& child : cacheReader.getRootNode()->m_mapChildren)
                {
                    unsigned long id = strtoul(child.first.c_str(), nullptr, 16);
                    hecl::FourCC type(child.second->m_seqChildren.at(0)->m_scalarString.c_str());
                    hecl::ProjectPath path(m_proj->getProjectWorkingPath(),
                        child.second->m_seqChildren.at(1)->m_scalarString);
                    m_tagToPath[SObjectTag(type, id)] = path;
                    fprintf(stderr, "\r %" PRISize " / %" PRISize, loadIdx++,
                            cacheReader.getRootNode()->m_mapChildren.size());
                }
                fprintf(stderr, "\n");
            }
            fclose(cacheFile);
            Log.report(logvisor::Info, _S("Cache index of '%s' loaded; %d tags"),
                       m_origSpec->m_name, m_tagToPath.size());

            if (nameCachePath.getPathType() == hecl::ProjectPath::Type::File)
            {
                /* Read in name cache */
                Log.report(logvisor::Info, _S("Name index of '%s' loading"), m_origSpec->m_name);
                FILE* nameFile = hecl::Fopen(nameCachePath.getAbsolutePath().c_str(), _S("r"));
                athena::io::YAMLDocReader nameReader;
                yaml_parser_set_input_file(nameReader.getParser(), nameFile);
                if (nameReader.parse())
                {
                    std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                    m_catalogNameToTag.reserve(nameReader.getRootNode()->m_mapChildren.size());
                    for (const auto& child : nameReader.getRootNode()->m_mapChildren)
                    {
                        unsigned long id = strtoul(child.second->m_scalarString.c_str(), nullptr, 16);
                        auto search = m_tagToPath.find(SObjectTag(FourCC(), uint32_t(id)));
                        if (search != m_tagToPath.cend())
                            m_catalogNameToTag[child.first] = search->first;
                    }
                }
                fclose(nameFile);
                Log.report(logvisor::Info, _S("Name index of '%s' loaded; %d names"),
                           m_origSpec->m_name, m_catalogNameToTag.size());
            }
            else
            {
                /* Build name cache */
                Log.report(logvisor::Info, _S("Name index of '%s' started"), m_origSpec->m_name);
                athena::io::YAMLDocWriter nameWriter(nullptr);
                BackgroundIndexRecursiveCatalogs(specRoot, nameWriter, 0);
                FILE* nameFile = hecl::Fopen(nameCachePath.getAbsolutePath().c_str(), _S("w"));
                yaml_emitter_set_output_file(nameWriter.getEmitter(), nameFile);
                nameWriter.finish();
                fclose(nameFile);
                Log.report(logvisor::Info, _S("Name index of '%s' complete; %d names"),
                           m_origSpec->m_name, m_catalogNameToTag.size());
            }
            m_backgroundRunning = false;
            return;
        }
    }

    Log.report(logvisor::Info, _S("Background index of '%s' started"), m_origSpec->m_name);
    athena::io::YAMLDocWriter cacheWriter(nullptr);
    athena::io::YAMLDocWriter nameWriter(nullptr);
    BackgroundIndexRecursiveProc(specRoot, cacheWriter, nameWriter, 0);

    FILE* cacheFile = hecl::Fopen(tagCachePath.getAbsolutePath().c_str(), _S("w"));
    yaml_emitter_set_output_file(cacheWriter.getEmitter(), cacheFile);
    cacheWriter.finish();
    fclose(cacheFile);

    FILE* nameFile = hecl::Fopen(nameCachePath.getAbsolutePath().c_str(), _S("w"));
    yaml_emitter_set_output_file(nameWriter.getEmitter(), nameFile);
    nameWriter.finish();
    fclose(nameFile);

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
    if (working.getAuxInfo().size())
        return working.getCookedPath(*spec).getWithExtension((_S('.') + working.getAuxInfo()).c_str());
    else
        return working.getCookedPath(*spec);
}

bool ProjectResourceFactoryBase::SyncCook(const hecl::ProjectPath& working)
{
    return m_clientProc.syncCook(working, m_cookSpec.get(), hecl::SharedBlenderToken);
}

CFactoryFnReturn ProjectResourceFactoryBase::BuildSync(const SObjectTag& tag,
                                                       const hecl::ProjectPath& path,
                                                       const CVParamTransfer& paramXfer)
{
    /* Ensure requested resource is on the filesystem */
    if (path.getPathType() != hecl::ProjectPath::Type::File)
    {
        Log.report(logvisor::Error, _S("unable to find resource path '%s'"),
                   path.getAbsolutePath().c_str());
        return {};
    }

    /* Last chance type validation */
    urde::SObjectTag verifyTag = TagFromPath(path, hecl::SharedBlenderToken);
    if (verifyTag.type != tag.type)
    {
        Log.report(logvisor::Error, _S("%s: expected type '%.4s', found '%.4s'"),
                   path.getRelativePath().c_str(),
                   tag.type.toString().c_str(), verifyTag.type.toString().c_str());
        return {};
    }

    /* Get cooked representation path */
    hecl::ProjectPath cooked = GetCookedPath(path, true);

    /* Perform mod-time comparison */
    if (cooked.getPathType() != hecl::ProjectPath::Type::File ||
        cooked.getModtime() < path.getModtime())
    {
        /* Do a blocking cook here */
        if (!SyncCook(path))
        {
            Log.report(logvisor::Error, _S("unable to cook resource path '%s'"),
                       path.getAbsolutePath().c_str());
            return {};
        }
    }

    /* Ensure cooked rep is on the filesystem */
    athena::io::FileReader fr(cooked.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
    {
        Log.report(logvisor::Error, _S("unable to open cooked resource path '%s'"),
                   cooked.getAbsolutePath().c_str());
        return {};
    }

    /* All good, build resource */
    if (m_factoryMgr.CanMakeMemory(tag))
    {
        u32 length = fr.length();
        std::unique_ptr<u8[]> memBuf = fr.readUBytes(length);
        return m_factoryMgr.MakeObjectFromMemory(tag, std::move(memBuf), length, false, paramXfer);
    }

    return m_factoryMgr.MakeObject(tag, fr, paramXfer);
}

void ProjectResourceFactoryBase::AsyncTask::EnsurePath(const urde::SObjectTag& tag,
                                                       const hecl::ProjectPath& path)
{
    if (!m_workingPath)
    {
        m_workingPath = path;

        /* Ensure requested resource is on the filesystem */
        if (path.getPathType() != hecl::ProjectPath::Type::File)
        {
            Log.report(logvisor::Error, _S("unable to find resource path '%s'"),
                       path.getRelativePath().c_str());
            m_failed = true;
            return;
        }

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
        if (m_cookedPath.getPathType() != hecl::ProjectPath::Type::File ||
            m_cookedPath.getModtime() < path.getModtime())
        {
            /* Start a background cook here */
            m_cookTransaction = m_parent.m_clientProc.addCookTransaction(path, m_parent.m_cookSpec.get());
            return;
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
    x14_resSize = fr.length();
    x10_loadBuffer.reset(new u8[x14_resSize]);
    m_bufTransaction = m_parent.m_clientProc.addBufferTransaction(m_cookedPath,
                                                                  x10_loadBuffer.get(),
                                                                  x14_resSize, 0);
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

std::unique_ptr<urde::IObj> ProjectResourceFactoryBase::Build(const urde::SObjectTag& tag,
                                                              const urde::CVParamTransfer& paramXfer)
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
                lk.lock();
                search = m_tagToPath.find(tag);
                if (search != m_tagToPath.end())
                    break;
            }
            if (search == m_tagToPath.end())
                return {};
        }
        else
            return {};
    }
    lk.unlock();

    auto asyncSearch = m_asyncLoadList.find(tag);
    if (asyncSearch != m_asyncLoadList.end())
    {
        /* Async spinloop */
        AsyncTask& task = asyncSearch->second;
        task.EnsurePath(task.x0_tag, search->second);

        /* Pump load pipeline (cooking if needed) */
        while (!task.AsyncPump()) {std::this_thread::sleep_for(std::chrono::milliseconds(2));}

        if (task.m_complete)
        {
            /* Load complete, build resource */
            std::unique_ptr<IObj> newObj;
            if (m_factoryMgr.CanMakeMemory(task.x0_tag))
            {
                newObj = m_factoryMgr.MakeObjectFromMemory(tag, std::move(task.x10_loadBuffer),
                                                           task.x14_resSize, false, task.x18_cvXfer);
            }
            else
            {
                athena::io::MemoryReader mr(task.x10_loadBuffer.get(), task.x14_resSize);
                newObj = m_factoryMgr.MakeObject(task.x0_tag, mr, task.x18_cvXfer);
            }

            *task.xc_targetPtr = newObj.get();
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
    return BuildSync(tag, search->second, paramXfer);
}

void ProjectResourceFactoryBase::BuildAsync(const urde::SObjectTag& tag,
                                            const urde::CVParamTransfer& paramXfer,
                                            urde::IObj** objOut)
{
    if (m_asyncLoadList.find(tag) != m_asyncLoadList.end())
        return;
    m_asyncLoadList.emplace(std::make_pair(tag, AsyncTask(*this, tag, objOut, paramXfer)));
}

void ProjectResourceFactoryBase::CancelBuild(const urde::SObjectTag& tag)
{
    m_asyncLoadList.erase(tag);
}

bool ProjectResourceFactoryBase::CanBuild(const urde::SObjectTag& tag)
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

    if (search->second.getPathType() == hecl::ProjectPath::Type::File)
        return true;

    return false;
}

const urde::SObjectTag* ProjectResourceFactoryBase::GetResourceIdByName(const char* name) const
{
    std::unique_lock<std::mutex> lk(((ProjectResourceFactoryBase*)this)->m_backgroundIndexMutex);
    auto search = m_catalogNameToTag.find(name);
    if (search == m_catalogNameToTag.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                lk.lock();
                search = m_catalogNameToTag.find(name);
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
    std::unique_lock<std::mutex> lk(((ProjectResourceFactoryBase*)this)->m_backgroundIndexMutex);
    SObjectTag searchTag = {FourCC(), id};
    auto search = m_tagToPath.find(searchTag);
    if (search == m_tagToPath.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
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

void ProjectResourceFactoryBase::AsyncIdle()
{
    /* Consume completed transactions, they will be processed this cycle at the latest */
    std::list<std::unique_ptr<hecl::ClientProcess::Transaction>> completed;
    m_clientProc.swapCompletedQueue(completed);

    /* Begin self-profiling loop */
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    for (auto it=m_asyncLoadList.begin() ; it != m_asyncLoadList.end() ;)
    {
        /* Allow 8 milliseconds (roughly 1/2 frame-time) for each async build cycle */
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count() > 8)
            break;

        /* Ensure requested resource is in the index */
        std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
        AsyncTask& task = it->second;
        auto search = m_tagToPath.find(task.x0_tag);
        if (search == m_tagToPath.end())
        {
            if (!m_backgroundRunning)
            {
                Log.report(logvisor::Error, _S("unable to find async load resource (%s, %08X)"),
                           task.x0_tag.type.toString().c_str(), task.x0_tag.id);
                it = m_asyncLoadList.erase(it);
            }
            continue;
        }
        lk.unlock();
        task.EnsurePath(task.x0_tag, search->second);

        /* Pump load pipeline (cooking if needed) */
        if (task.AsyncPump())
        {
            if (task.m_complete)
            {
                /* Load complete, build resource */
                std::unique_ptr<IObj> newObj;
                if (m_factoryMgr.CanMakeMemory(task.x0_tag))
                {
                    newObj = m_factoryMgr.MakeObjectFromMemory(task.x0_tag, std::move(task.x10_loadBuffer),
                                                               task.x14_resSize, false, task.x18_cvXfer);
                }
                else
                {
                    athena::io::MemoryReader mr(task.x10_loadBuffer.get(), task.x14_resSize);
                    newObj = m_factoryMgr.MakeObject(task.x0_tag, mr, task.x18_cvXfer);
                }

                *task.xc_targetPtr = newObj.release();
                Log.report(logvisor::Info, "async-built %.4s %08X",
                           task.x0_tag.type.toString().c_str(),
                           u32(task.x0_tag.id));
            }
            it = m_asyncLoadList.erase(it);
            continue;
        }
        ++it;
    }
}

}
