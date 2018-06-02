#include "hecl/ClientProcess.hpp"
#include "hecl/Database.hpp"
#include "athena/FileReader.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/MultiProgressPrinter.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/wait.h>
#endif

#define HECL_MULTIPROCESSOR 1

namespace hecl
{
static logvisor::Module CP_Log("hecl::ClientProcess");

ThreadLocalPtr<ClientProcess::Worker> ClientProcess::ThreadWorker;

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

void ClientProcess::BufferTransaction::run(blender::Token& btok)
{
    athena::io::FileReader r(m_path.getAbsolutePath(), 32 * 1024, false);
    if (r.hasError())
    {
        CP_Log.report(logvisor::Fatal, _S("unable to background-buffer '%s'"),
                      m_path.getAbsolutePath().data());
        return;
    }
    if (m_offset)
        r.seek(m_offset, athena::Begin);
    r.readBytesToBuf(m_targetBuf, m_maxLen);
    m_complete = true;
}

void ClientProcess::CookTransaction::run(blender::Token& btok)
{
    m_dataSpec->setThreadProject();
    m_returnResult = m_parent.syncCook(m_path, m_dataSpec, btok, m_force, m_fast);
    std::unique_lock<std::mutex> lk(m_parent.m_mutex);
    ++m_parent.m_completedCooks;
    m_parent.m_progPrinter->setMainFactor(m_parent.m_completedCooks / float(m_parent.m_addedCooks));
    m_complete = true;
}

void ClientProcess::LambdaTransaction::run(blender::Token& btok)
{
    m_func(btok);
    m_complete = true;
}

ClientProcess::Worker::Worker(ClientProcess& proc, int idx)
: m_proc(proc), m_idx(idx)
{
    m_thr = std::thread(std::bind(&Worker::proc, this));
}

void ClientProcess::Worker::proc()
{
    ClientProcess::ThreadWorker.reset(this);

    char thrName[64];
    snprintf(thrName, 64, "HECL Worker%d", m_idx);
    logvisor::RegisterThreadName(thrName);

    std::unique_lock<std::mutex> lk(m_proc.m_mutex);
    while (m_proc.m_running)
    {
        if (!m_didInit)
        {
            m_proc.m_initCv.notify_one();
            m_didInit = true;
        }
        while (m_proc.m_running && m_proc.m_pendingQueue.size())
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
    lk.unlock();
    m_blendTok.shutdown();
}

ClientProcess::ClientProcess(const MultiProgressPrinter* progPrinter)
: m_progPrinter(progPrinter)
{
#if HECL_MULTIPROCESSOR
    const int cpuCount = GetCPUCount();
#else
    constexpr int cpuCount = 1;
#endif
    m_workers.reserve(cpuCount);
    for (int i=0 ; i<cpuCount ; ++i)
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_workers.emplace_back(*this, m_workers.size());
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
ClientProcess::addCookTransaction(const hecl::ProjectPath& path, bool force,
                                  bool fast, Database::IDataSpec* spec)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto ret = std::make_shared<CookTransaction>(*this, path, force, fast, spec);
    m_pendingQueue.emplace_back(ret);
    m_cv.notify_one();
    ++m_addedCooks;
    m_progPrinter->setMainFactor(m_completedCooks / float(m_addedCooks));
    return ret;
}

std::shared_ptr<const ClientProcess::LambdaTransaction>
ClientProcess::addLambdaTransaction(std::function<void(blender::Token&)>&& func)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto ret = std::make_shared<LambdaTransaction>(*this, std::move(func));
    m_pendingQueue.emplace_back(ret);
    m_cv.notify_one();
    return ret;
}

bool ClientProcess::syncCook(const hecl::ProjectPath& path, Database::IDataSpec* spec, blender::Token& btok,
                             bool force, bool fast)
{
    if (spec->canCook(path, btok))
    {
        const Database::DataSpecEntry* specEnt = spec->overrideDataSpec(path, spec->getDataSpecEntry(), btok);
        if (specEnt)
        {
            hecl::ProjectPath cooked = path.getCookedPath(*specEnt);
            if (fast)
                cooked = cooked.getWithExtension(_S(".fast"));
            cooked.makeDirChain(false);
            if (force || cooked.getPathType() == ProjectPath::Type::None ||
                path.getModtime() > cooked.getModtime())
            {
                if (m_progPrinter)
                {
                    hecl::SystemString str;
                    if (path.getAuxInfo().empty())
                        str = hecl::SysFormat(_S("Cooking %s"), path.getRelativePath().data());
                    else
                        str = hecl::SysFormat(_S("Cooking %s|%s"), path.getRelativePath().data(), path.getAuxInfo().data());
                    m_progPrinter->print(str.c_str(), nullptr, -1.f, hecl::ClientProcess::GetThreadWorkerIdx());
                    m_progPrinter->flush();
                }
                else
                {
                    if (path.getAuxInfo().empty())
                        LogModule.report(logvisor::Info, _S("Cooking %s"),
                                         path.getRelativePath().data());
                    else
                        LogModule.report(logvisor::Info, _S("Cooking %s|%s"),
                                         path.getRelativePath().data(),
                                         path.getAuxInfo().data());
                }
                spec->doCook(path, cooked, false, btok, [](const SystemChar*) {});
                if (m_progPrinter)
                {
                    hecl::SystemString str;
                    if (path.getAuxInfo().empty())
                        str = hecl::SysFormat(_S("Cooked  %s"), path.getRelativePath().data());
                    else
                        str = hecl::SysFormat(_S("Cooked  %s|%s"), path.getRelativePath().data(), path.getAuxInfo().data());
                    m_progPrinter->print(str.c_str(), nullptr, -1.f, hecl::ClientProcess::GetThreadWorkerIdx());
                    m_progPrinter->flush();
                }
            }
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
    while (isBusy())
        m_waitCv.wait(lk);
}

void ClientProcess::shutdown()
{
    if (!m_running)
        return;
    std::unique_lock<std::mutex> lk(m_mutex);
    m_pendingQueue.clear();
    m_running = false;
    m_cv.notify_all();
    lk.unlock();
    for (Worker& worker : m_workers)
        if (worker.m_thr.joinable())
            worker.m_thr.join();
}

}
