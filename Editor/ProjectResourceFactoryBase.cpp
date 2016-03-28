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

hecl::BlenderConnection& ProjectResourceFactoryBase::GetBackgroundBlender() const
{
    std::experimental::optional<hecl::BlenderConnection>& shareConn =
        ((ProjectResourceFactoryBase*)this)->m_backgroundBlender;
    if (!shareConn)
        shareConn.emplace(hecl::VerbosityLevel);
    return *shareConn;
}

void ProjectResourceFactoryBase::ReadCatalog(const hecl::ProjectPath& catalogPath)
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
        SObjectTag pathTag = TagFromPath(path);
        if (pathTag)
        {
            std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
            m_catalogNameToTag[p.first] = pathTag;
#if 0
            fprintf(stderr, "%s %s %08X\n",
                    p.first.c_str(),
                    pathTag.type.toString().c_str(), uint32_t(pathTag.id));
#endif
        }
    }
}

void ProjectResourceFactoryBase::BackgroundIndexRecursiveProc(const hecl::ProjectPath& dir, int level)
{
    hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    false, false, true);

    /* Enumerate all items */
    for (const hecl::DirectoryEnumerator::Entry& ent : dEnum)
    {
        hecl::ProjectPath path(dir, ent.m_name);
        if (ent.m_isDir)
            BackgroundIndexRecursiveProc(path, level+1);
        else
        {
            if (path.getPathType() != hecl::ProjectPath::Type::File)
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("catalog.yaml")))
            {
                ReadCatalog(path);
                continue;
            }

            /* Classify intermediate into tag */
            SObjectTag pathTag = TagFromPath(path);
            if (pathTag)
            {
                std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                m_tagToPath[pathTag] = path;
#if 0
                fprintf(stderr, "%s %08X %s\n",
                        pathTag.type.toString().c_str(), uint32_t(pathTag.id),
                        path.getRelativePathUTF8().c_str());
#endif
            }
        }

        /* bail if cancelled by client */
        if (!m_backgroundRunning)
            break;
    }
}

void ProjectResourceFactoryBase::BackgroundIndexProc()
{
    hecl::ProjectPath specRoot(m_proj->getProjectWorkingPath(), m_origSpec->m_name);
    BackgroundIndexRecursiveProc(specRoot, 0);
    m_backgroundRunning = false;
    m_backgroundBlender = std::experimental::nullopt;
}

void ProjectResourceFactoryBase::CancelBackgroundIndex()
{
    if (m_backgroundRunning && m_backgroundIndexTh.joinable())
    {
        m_backgroundRunning = false;
        m_backgroundIndexTh.join();
    }
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
        spec = m_cookSpec->overrideDataSpec(working, m_pcSpec);
    if (!spec)
        return {};
    return working.getCookedPath(*spec);
}

bool ProjectResourceFactoryBase::SyncCook(const hecl::ProjectPath& working)
{
    return m_clientProc.syncCook(working) == 0;
}

CFactoryFnReturn ProjectResourceFactoryBase::SyncMakeObject(const SObjectTag& tag,
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
    return m_factoryMgr.MakeObject(tag, fr, paramXfer);
}

void ProjectResourceFactoryBase::AsyncTask::EnsurePath(const hecl::ProjectPath& path)
{
    if (!m_workingPath)
    {
        m_workingPath = path;

        /* Ensure requested resource is on the filesystem */
        if (path.getPathType() != hecl::ProjectPath::Type::File)
        {
            Log.report(logvisor::Error, _S("unable to find resource path '%s'"),
                       path.getAbsolutePath().c_str());
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
            m_cookTransaction = m_parent.m_clientProc.addCookTransaction(path);
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
            return true;
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
        }
        else
            return {};
    }

    return SyncMakeObject(tag, search->second, paramXfer);
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
        }
        else
            return nullptr;
    }
    return &search->second;
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
        AsyncTask& data = it->second;
        auto search = m_tagToPath.find(data.x0_tag);
        if (search == m_tagToPath.end())
        {
            if (!m_backgroundRunning)
            {
                Log.report(logvisor::Error, _S("unable to find async load resource (%s, %08X)"),
                           data.x0_tag.type.toString().c_str(), data.x0_tag.id);
                it = m_asyncLoadList.erase(it);
            }
            continue;
        }
        data.EnsurePath(search->second);

        /* Pump load pipeline (cooking if needed) */
        if (data.AsyncPump())
        {
            /* Load complete, build resource */
            athena::io::MemoryReader mr(data.x10_loadBuffer.get(), data.x14_resSize);
            *data.xc_targetPtr = m_factoryMgr.MakeObject(data.x0_tag, mr, data.x18_cvXfer).release();
            it = m_asyncLoadList.erase(it);
            continue;
        }
        ++it;
    }
}

}
