#include "hecl/ClientProcess.hpp"
#include "hecl/Database.hpp"
#include "athena/FileReader.hpp"
#include "BlenderConnection.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/wait.h>
#endif

namespace hecl
{
static logvisor::Module Log("hecl::ClientProcess");

static int GetCPUCount()
{
#if _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void ClientProcess::BufferTransaction::run(BlenderToken& btok)
{
    athena::io::FileReader r(m_path.getAbsolutePath(), 32 * 1024, false);
    if (r.hasError())
    {
        Log.report(logvisor::Fatal, _S("unable to background-buffer '%s'"),
                   m_path.getAbsolutePath().c_str());
        return;
    }
    if (m_offset)
        r.seek(m_offset, athena::Begin);
    r.readBytesToBuf(m_targetBuf, m_maxLen);
    m_complete = true;
}

void ClientProcess::CookTransaction::run(BlenderToken& btok)
{
    m_returnResult = m_parent.syncCook(m_path, m_dataSpec, btok);
    m_complete = true;
}

void ClientProcess::LambdaTransaction::run(BlenderToken& btok)
{
    m_func(btok);
    m_complete = true;
}

ClientProcess::Worker::Worker(ClientProcess& proc)
: m_proc(proc)
{
    m_thr = std::thread(std::bind(&Worker::proc, this));
}

void ClientProcess::Worker::proc()
{
    while (m_proc.m_running)
    {
        std::unique_lock<std::mutex> lk(m_proc.m_mutex);
        if (!m_didInit)
        {
            m_proc.m_initCv.notify_one();
            m_didInit = true;
        }
        while (m_proc.m_pendingQueue.size())
        {
            std::shared_ptr<Transaction> trans = std::move(m_proc.m_pendingQueue.front());
            ++m_proc.m_inProgress;
            m_proc.m_pendingQueue.pop_front();
            lk.unlock();
            trans->run(m_blendTok);
            lk.lock();
            m_proc.m_completedQueue.push_back(std::move(trans));
            --m_proc.m_inProgress;
        }
        m_proc.m_waitCv.notify_one();
        if (!m_proc.m_running)
            break;
        m_proc.m_cv.wait(lk);
    }
    m_blendTok.shutdown();
}

ClientProcess::ClientProcess(int verbosityLevel)
: m_verbosity(verbosityLevel)
{
#ifdef NDEBUG
    int cpuCount = GetCPUCount();
#else
    constexpr int cpuCount = 1;
#endif
    m_workers.reserve(cpuCount);
    for (int i=0 ; i<cpuCount ; ++i)
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_workers.emplace_back(*this);
        m_initCv.wait(lk);
    }
}

std::shared_ptr<const ClientProcess::BufferTransaction>
ClientProcess::addBufferTransaction(const ProjectPath& path, void* target,
                                    size_t maxLen, size_t offset)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto ret = std::make_shared<BufferTransaction>(*this, path, target, maxLen, offset);
    m_pendingQueue.emplace_back(ret);
    m_cv.notify_one();
    return ret;
}

std::shared_ptr<const ClientProcess::CookTransaction>
ClientProcess::addCookTransaction(const hecl::ProjectPath& path, Database::IDataSpec* spec)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto ret = std::make_shared<CookTransaction>(*this, path, spec);
    m_pendingQueue.emplace_back(ret);
    m_cv.notify_one();
    return ret;
}

std::shared_ptr<const ClientProcess::LambdaTransaction>
ClientProcess::addLambdaTransaction(std::function<void(BlenderToken&)>&& func)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto ret = std::make_shared<LambdaTransaction>(*this, std::move(func));
    m_pendingQueue.emplace_back(ret);
    m_cv.notify_one();
    return ret;
}

bool ClientProcess::syncCook(const hecl::ProjectPath& path, Database::IDataSpec* spec, BlenderToken& btok)
{
    if (spec->canCook(path, btok))
    {
        const Database::DataSpecEntry* specEnt = spec->overrideDataSpec(path, spec->getDataSpecEntry(), btok);
        if (specEnt)
        {
            hecl::ProjectPath cooked = path.getCookedPath(*specEnt);
            cooked.makeDirChain(false);
            spec->doCook(path, cooked, false, btok, [](const SystemChar*) {});
            return true;
        }
    }
    return false;
}

void ClientProcess::swapCompletedQueue(std::list<std::shared_ptr<Transaction>>& queue)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    queue.swap(m_completedQueue);
}

void ClientProcess::waitUntilComplete()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    while (m_pendingQueue.size() || m_inProgress)
        m_waitCv.wait(lk);
}

void ClientProcess::shutdown()
{
    if (!m_running)
        return;
    m_running = false;
    m_cv.notify_all();
    for (Worker& worker : m_workers)
        if (worker.m_thr.joinable())
            worker.m_thr.join();
}

}
