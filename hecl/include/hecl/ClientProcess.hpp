#ifndef HECL_CLIENT_PROCESS_HPP
#define HECL_CLIENT_PROCESS_HPP

#include "hecl.hpp"
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace hecl
{

class ClientProcess
{
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_verbosity;

public:
    struct Transaction
    {
        enum class Type
        {
            Buffer,
            Cook
        } m_type;
        bool m_complete = false;
        virtual void run()=0;
        Transaction(Type tp) : m_type(tp) {}
    };
    struct BufferTransaction : Transaction
    {
        ProjectPath m_path;
        void* m_targetBuf;
        size_t m_maxLen;
        size_t m_offset;
        void run();
        BufferTransaction(const ProjectPath& path, void* target, size_t maxLen, size_t offset)
        : Transaction(Type::Buffer), m_path(path), m_targetBuf(target), m_maxLen(maxLen), m_offset(offset) {}
    };
    struct CookTransaction : Transaction
    {
        ProjectPath m_path;
        bool m_verbose;
        int m_returnVal = 0;
        void run();
        CookTransaction(const ProjectPath& path, bool verbose)
        : Transaction(Type::Cook), m_path(path), m_verbose(verbose) {}
    };
private:
    std::list<std::unique_ptr<Transaction>> m_pendingQueue;
    std::list<std::unique_ptr<Transaction>> m_completedQueue;
    bool m_running = true;

    struct Worker
    {
        ClientProcess& m_proc;
        std::thread m_thr;
        Worker(ClientProcess& proc);
        void proc();
    };
    std::vector<Worker> m_workers;

public:
    ClientProcess(int verbosityLevel=1);
    ~ClientProcess() {shutdown();}
    void addBufferTransaction(const hecl::ProjectPath& path, void* target,
                              size_t maxLen, size_t offset);
    void addCookTransaction(const hecl::ProjectPath& path);
    void swapCompletedQueue(std::list<std::unique_ptr<Transaction>>& queue);
    void shutdown();
};

}

#endif // HECL_CLIENT_PROCESS_HPP
