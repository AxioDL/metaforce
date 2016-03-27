#include "hecl/ClientProcess.hpp"
#include "athena/FileReader.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/wait.h>
#endif

namespace hecl
{
static logvisor::Module Log("hecl::ClientProcess");

static bool ExecProcessAndWait(bool verbose,
                               const SystemChar* exePath,
                               const SystemChar* args[])
{
#if _WIN32
    std::wstring cmdLine;
    for (const SystemChar** it=args ; *it ; ++it)
    {
        if (it != args)
            cmdLine.append(L' ');
        cmdLine.append(*it);
    }

    STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
    HANDLE nulHandle = NULL;
    if (!verbose)
    {
        SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        nulHandle = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &sattrs,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        sinfo.hStdError = nulHandle;
        sinfo.hStdOutput = nulHandle;
        sinfo.dwFlags = STARTF_USESTDHANDLES;
    }

    PROCESS_INFORMATION pinfo;
    if (!CreateProcessW(exePath, cmdLine.c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sinfo, &pinfo))
    {
        LPWSTR messageBuffer = nullptr;
        size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
        Log.report(logvisor::Fatal, L"error launching '%s': %s", exePath, messageBuffer);
    }

    if (nulHandle)
        CloseHandle(nulHandle);

    CloseHandle(pinfo.hThread);
    WaitForSingleObject(pinfo.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pinfo.hProcess, &exitCode);
    CloseHandle(pinfo.hProcess);

    if (exitCode == 0)
        return true;

    return false;

#else
    /* Assemble command args */
    std::vector<const SystemChar*> assembleArgs;
    size_t argCount = 0;
    for (const SystemChar** it=args ; *it ; ++it) ++argCount;
    assembleArgs.reserve(argCount+1);
    assembleArgs.push_back(exePath);
    for (const SystemChar** it=args ; *it ; ++it) assembleArgs.push_back(*it);

    pid_t pid = fork();
    if (!pid)
    {
        if (!verbose)
        {
            int devNull = open("/dev/null", O_WRONLY);
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
        }

        if (execvp(exePath, (char*const*)assembleArgs.data()) < 0)
        {
            if (errno == ENOENT)
                return false;
            else
                Log.report(logvisor::Fatal, _S("error execing '%s': %s"), exePath, strerror(errno));
        }

        exit(1);
    }

    int exitStatus;
    if (waitpid(pid, &exitStatus, 0) < 0)
        Log.report(logvisor::Fatal, "unable to wait for hecl process to complete: %s", strerror(errno));

    if (WIFEXITED(exitStatus))
        if (WEXITSTATUS(exitStatus) == 0)
            return true;

    return false;

#endif
}

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

void ClientProcess::BufferTransaction::run()
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
}

void ClientProcess::CookTransaction::run()
{
    const SystemChar* args[] = {_S("cook"), m_path.getAbsolutePath().c_str()};
    if (!ExecProcessAndWait(m_verbose, _S("hecl"), args))
        Log.report(logvisor::Fatal, _S("unable to background-cook '%s'"),
                   m_path.getAbsolutePath().c_str());
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
        if (m_proc.m_pendingQueue.size())
        {
            std::unique_ptr<Transaction> trans = std::move(m_proc.m_pendingQueue.front());
            m_proc.m_pendingQueue.pop_front();
            lk.unlock();
            trans->run();
            lk.lock();
            m_proc.m_completedQueue.push_back(std::move(trans));
        }
        m_proc.m_cv.wait(lk);
    }
}

ClientProcess::ClientProcess(int verbosityLevel)
: m_verbosity(verbosityLevel)
{
    int cpuCount = GetCPUCount();
    m_workers.reserve(cpuCount);
    for (int i=0 ; i<cpuCount ; ++i)
        m_workers.emplace_back(*this);
}

void ClientProcess::addBufferTransaction(const ProjectPath& path, void* target,
                                         size_t maxLen, size_t offset)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_pendingQueue.emplace_back(new BufferTransaction(path, target, maxLen, offset));
    m_cv.notify_one();
}

void ClientProcess::addCookTransaction(const hecl::ProjectPath& path)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_pendingQueue.emplace_back(new CookTransaction(path, m_verbosity != 0));
    m_cv.notify_one();
}

void ClientProcess::swapCompletedQueue(std::list<std::unique_ptr<Transaction>>& queue)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    queue.swap(m_completedQueue);
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
