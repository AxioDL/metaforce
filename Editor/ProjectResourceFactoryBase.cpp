#include "ProjectResourceFactoryBase.hpp"
#include "Runtime/IObj.hpp"

#undef min
#undef max

namespace urde
{
static logvisor::Module Log("urde::ProjectResourceFactoryBase");

void ProjectResourceFactoryBase::BeginBackgroundIndex
    (hecl::Database::Project& proj,
     const hecl::Database::DataSpecEntry& origSpec,
     const hecl::Database::DataSpecEntry& pcSpec)
{
    CancelBackgroundIndex();
    m_proj = &proj;
    m_origSpec = &origSpec;
    m_pcSpec = &pcSpec;
    m_cookSpec.reset(pcSpec.m_factory(proj, hecl::Database::DataSpecTool::Cook));
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).beginBackgroundIndex();
}

bool ProjectResourceFactoryBase::SyncCook(const hecl::ProjectPath& working)
{
    Log.report(logvisor::Warning, _S("sync-cooking %s"), working.getRelativePath().data());
    return m_clientProc.syncCook(working, m_cookSpec.get(), hecl::blender::SharedBlenderToken,
                                 false, false);
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
                   tag.type.getChars(), u32(tag.id.Value()));
        return ret;
    }

    CFactoryFnReturn ret = m_factoryMgr.MakeObject(tag, *fr, paramXfer, selfRef);
    Log.report(logvisor::Info, "sync-built %.4s %08X",
               tag.type.getChars(), u32(tag.id.Value()));
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
                       path.getRelativePath().data());
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
            urde::SObjectTag verifyTag = m_parent.TagFromPath(path, hecl::blender::SharedBlenderToken);
            if (verifyTag.type != tag.type)
            {
                Log.report(logvisor::Error, _S("%s: expected type '%.4s', found '%.4s'"),
                           path.getRelativePath().data(),
                           tag.type.getChars(), verifyTag.type.getChars());
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
                m_cookTransaction = m_parent.m_clientProc.
                        addCookTransaction(path, m_parent.m_cookSpec.get(), false, false);
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
                   m_cookedPath.getAbsolutePath().data());
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
    else if (xc_targetDataPtr || xc_targetObjPtr)
    {
        x10_loadBuffer.reset(new u8[x14_resSize]);
        m_bufTransaction = m_parent.m_clientProc.addBufferTransaction(m_cookedPath,
                                                                      x10_loadBuffer.get(),
                                                                      x14_resSize, x14_resOffset);
    }
    else
    {
        /* Skip buffer transaction if no target pointers set */
        m_complete = true;
    }
}

bool ProjectResourceFactoryBase::AsyncTask::AsyncPump()
{
    if (m_failed || m_complete)
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

void ProjectResourceFactoryBase::AsyncTask::WaitUntilComplete()
{
    using ItType = std::unordered_map<SObjectTag,
        std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator;
    ItType search = m_parent.m_asyncLoadMap.find(x0_tag);
    if (search == m_parent.m_asyncLoadMap.end())
        return;
    for (ItType tmp = search ; !m_parent.AsyncPumpTask(tmp) ; tmp = search)
    {std::this_thread::sleep_for(std::chrono::milliseconds(2));}
}

using AsyncTask = ProjectResourceFactoryBase::AsyncTask;

std::shared_ptr<AsyncTask>
ProjectResourceFactoryBase::_AddTask(const std::shared_ptr<AsyncTask>& ptr)
{
    m_asyncLoadMap.insert({ptr->x0_tag, m_asyncLoadList.insert(m_asyncLoadList.end(), ptr)});
    return ptr;
}

std::list<std::shared_ptr<AsyncTask>>::iterator
ProjectResourceFactoryBase::_RemoveTask(std::list<std::shared_ptr<AsyncTask>>::iterator it)
{
    m_asyncLoadMap.erase((*it)->x0_tag);
    return m_asyncLoadList.erase(it);
}

std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator
ProjectResourceFactoryBase::_RemoveTask(std::unordered_map<SObjectTag,
    std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator it)
{
    if (it != m_asyncLoadMap.end())
    {
        m_asyncLoadList.erase(it->second);
        return m_asyncLoadMap.erase(it);
    }
    return it;
};

std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator
ProjectResourceFactoryBase::_RemoveTask(const SObjectTag& tag)
{
    return _RemoveTask(m_asyncLoadMap.find(tag));
};

bool
ProjectResourceFactoryBase::PrepForReadSync(const SObjectTag& tag,
                                            const hecl::ProjectPath& path,
                                            std::experimental::optional<athena::io::FileReader>& fr)
{
    /* Ensure requested resource is on the filesystem */
    if (!path.isFileOrGlob())
    {
        Log.report(logvisor::Error, _S("unable to find resource path '%s'"),
                   path.getAbsolutePath().data());
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
        urde::SObjectTag verifyTag = TagFromPath(path, hecl::blender::SharedBlenderToken);
        if (verifyTag.type != tag.type)
        {
            Log.report(logvisor::Error, _S("%s: expected type '%.4s', found '%.4s'"),
                       path.getRelativePath().data(),
                       tag.type.getChars(), verifyTag.type.getChars());
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
                           path.getAbsolutePath().data());
                return false;
            }
        }
    }

    /* Ensure cooked rep is on the filesystem */
    fr.emplace(cooked.getAbsolutePath(), 32 * 1024, false);
    if (fr->hasError())
    {
        Log.report(logvisor::Error, _S("unable to open cooked resource path '%s'"),
                   cooked.getAbsolutePath().data());
        return false;
    }

    return true;
}

std::unique_ptr<urde::IObj> ProjectResourceFactoryBase::Build(const urde::SObjectTag& tag,
                                                              const urde::CVParamTransfer& paramXfer,
                                                              CObjectReference* selfRef)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id on type '%.4s'", tag.type.getChars());

    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return {};
    auto asyncSearch = m_asyncLoadMap.find(tag);
    if (asyncSearch != m_asyncLoadMap.end())
    {
        /* Async spinloop */
        AsyncTask& task = **asyncSearch->second;
        task.EnsurePath(task.x0_tag, *resPath);

        /* Pump load pipeline (cooking if needed) */
        while (!task.AsyncPump()) {std::this_thread::sleep_for(std::chrono::milliseconds(2));}

        if (task.m_complete && task.x10_loadBuffer)
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

            //*task.xc_targetObjPtr = newObj.get();
            Log.report(logvisor::Warning, "spin-built %.4s %08X",
                       task.x0_tag.type.getChars(), u32(task.x0_tag.id.Value()));

            _RemoveTask(asyncSearch);
            return newObj;
        }
        else if (task.m_complete)
        {
            Log.report(logvisor::Error, "unable to spin-build %.4s %08X; Resource requested as cook-only",
                       task.x0_tag.type.getChars(), u32(task.x0_tag.id.Value()));
        }
        else
        {
            Log.report(logvisor::Error, "unable to spin-build %.4s %08X",
                       task.x0_tag.type.getChars(), u32(task.x0_tag.id.Value()));
        }

        _RemoveTask(asyncSearch);
        return {};
    }

    /* Fall-back to sync build */
    return BuildSync(tag, *resPath, paramXfer, selfRef);
}

std::shared_ptr<AsyncTask>
ProjectResourceFactoryBase::BuildAsyncInternal(const urde::SObjectTag& tag,
                                               const urde::CVParamTransfer& paramXfer,
                                               std::unique_ptr<urde::IObj>* objOut,
                                               CObjectReference* selfRef)
{
    if (m_asyncLoadMap.find(tag) != m_asyncLoadMap.end())
        return {};
    return _AddTask(std::make_unique<AsyncTask>(*this, tag, objOut, paramXfer, selfRef));
}

void ProjectResourceFactoryBase::BuildAsync(const urde::SObjectTag& tag,
                                            const urde::CVParamTransfer& paramXfer,
                                            std::unique_ptr<urde::IObj>* objOut,
                                            CObjectReference* selfRef)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id on type '%.4s'", tag.type.getChars());

    BuildAsyncInternal(tag, paramXfer, objOut, selfRef);
}

u32 ProjectResourceFactoryBase::ResourceSize(const SObjectTag& tag)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id on type '%.4s'", tag.type.getChars());

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

std::shared_ptr<urde::IDvdRequest>
ProjectResourceFactoryBase::LoadResourceAsync(const urde::SObjectTag& tag, void* target)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id");
    if (m_asyncLoadMap.find(tag) != m_asyncLoadMap.end())
        return {};
    return std::static_pointer_cast<urde::IDvdRequest>(_AddTask(std::make_shared<AsyncTask>(*this, tag, reinterpret_cast<u8*>(target))));
}

std::shared_ptr<urde::IDvdRequest>
ProjectResourceFactoryBase::LoadResourcePartAsync(const urde::SObjectTag& tag,
                                                  u32 off, u32 size, void* target)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id");
    if (m_asyncLoadMap.find(tag) != m_asyncLoadMap.end())
        return {};
    return std::static_pointer_cast<urde::IDvdRequest>(_AddTask(std::make_shared<AsyncTask>(*this, tag, reinterpret_cast<u8*>(target), size, off)));
}

std::unique_ptr<u8[]> ProjectResourceFactoryBase::LoadResourceSync(const urde::SObjectTag& tag)
{
    if (!tag.id.IsValid())
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

std::unique_ptr<u8[]> ProjectResourceFactoryBase::LoadNewResourcePartSync(const urde::SObjectTag& tag,
                                                                       u32 off, u32 size)
{
    if (!tag.id.IsValid())
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

std::shared_ptr<AsyncTask>
ProjectResourceFactoryBase::CookResourceAsync(const urde::SObjectTag& tag)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id");
    if (m_asyncLoadMap.find(tag) != m_asyncLoadMap.end())
        return {};
    return _AddTask(std::make_shared<AsyncTask>(*this, tag));
}

void ProjectResourceFactoryBase::CancelBuild(const urde::SObjectTag& tag)
{
    _RemoveTask(tag);
}

bool ProjectResourceFactoryBase::CanBuild(const urde::SObjectTag& tag)
{
    if (!tag.id.IsValid())
        Log.report(logvisor::Fatal, "attempted to access null id");

    const hecl::ProjectPath* resPath = nullptr;
    if (!WaitForTagReady(tag, resPath))
        return false;

    if (resPath->isFile())
        return true;

    return false;
}

const urde::SObjectTag* ProjectResourceFactoryBase::GetResourceIdByName(std::string_view name) const
{
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).getResourceIdByName(name);
}

FourCC ProjectResourceFactoryBase::GetResourceTypeById(CAssetId id) const
{
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).getResourceTypeById(id);
}

void ProjectResourceFactoryBase::EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const
{
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).enumerateResources(lambda);
}

void ProjectResourceFactoryBase::EnumerateNamedResources(
        const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const
{
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).enumerateNamedResources(lambda);
}

template <typename ItType>
bool ProjectResourceFactoryBase::AsyncPumpTask(ItType& it)
{
    /* Ensure requested resource is in the index */
    AsyncTask& task = _GetAsyncTask(it);
    hecl::ProjectPath path = static_cast<DataSpec::SpecBase&>(*m_cookSpec).pathFromTag(task.x0_tag);
    if (!path)
    {
        if (!static_cast<DataSpec::SpecBase&>(*m_cookSpec).backgroundIndexRunning())
        {
            Log.report(logvisor::Error, _S("unable to find async load resource (%.4s, %08X)"),
                       task.x0_tag.type.getChars(), task.x0_tag.id);
            it = _RemoveTask(it);
        }
        return true;
    }
    task.EnsurePath(task.x0_tag, path);

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

                *task.xc_targetObjPtr = std::move(newObj);
                Log.report(logvisor::Info, "async-built %.4s %08X",
                           task.x0_tag.type.getChars(),
                           u32(task.x0_tag.id.Value()));
            }
            else if (task.xc_targetDataPtr)
            {
                /* Buffer only */
                *task.xc_targetDataPtr = std::move(task.x10_loadBuffer);
                Log.report(logvisor::Info, "async-loaded %.4s %08X",
                           task.x0_tag.type.getChars(),
                           u32(task.x0_tag.id.Value()));
            }
            else if (task.xc_targetDataRawPtr)
            {
                /* Buffer only raw */
                Log.report(logvisor::Info, "async-loaded %.4s %08X",
                           task.x0_tag.type.getChars(),
                           u32(task.x0_tag.id.Value()));
            }
        }

        it = _RemoveTask(it);
        return true;
    }
    ++it;
    return false;
}

template bool ProjectResourceFactoryBase::AsyncPumpTask<std::list<std::shared_ptr<AsyncTask>>::iterator>(
    std::list<std::shared_ptr<AsyncTask>>::iterator& it);
template bool ProjectResourceFactoryBase::AsyncPumpTask<std::unordered_map<SObjectTag,
    std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator>(
    std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator& it);

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
