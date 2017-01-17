#ifndef HECL_CLIENT_PROCESS_HPP
#define HECL_CLIENT_PROCESS_HPP

#include "hecl.hpp"
#include "Database.hpp"
#include "hecl/Blender/BlenderConnection.hpp"
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
    std::condition_variable m_initCv;
    std::condition_variable m_waitCv;
    int m_verbosity;

public:
    struct Transaction
    {
        ClientProcess& m_parent;
        enum class Type
        {
            Buffer,
            Cook,
            Lambda
        } m_type;
        bool m_complete = false;
        virtual void run(BlenderToken& btok)=0;
        Transaction(ClientProcess& parent, Type tp) : m_parent(parent), m_type(tp) {}
    };
    struct BufferTransaction : Transaction
    {
        ProjectPath m_path;
        void* m_targetBuf;
        size_t m_maxLen;
        size_t m_offset;
        void run(BlenderToken& btok);
        BufferTransaction(ClientProcess& parent, const ProjectPath& path,
                          void* target, size_t maxLen, size_t offset)
        : Transaction(parent, Type::Buffer),
          m_path(path), m_targetBuf(target),
          m_maxLen(maxLen), m_offset(offset) {}
    };
    struct CookTransaction : Transaction
    {
        ProjectPath m_path;
        Database::IDataSpec* m_dataSpec;
        bool m_returnResult = false;
        void run(BlenderToken& btok);
        CookTransaction(ClientProcess& parent, const ProjectPath& path, Database::IDataSpec* spec)
        : Transaction(parent, Type::Cook), m_path(path), m_dataSpec(spec) {}
    };
    struct LambdaTransaction : Transaction
    {
        std::function<void(BlenderToken&)> m_func;
        void run(BlenderToken& btok);
        LambdaTransaction(ClientProcess& parent, std::function<void(BlenderToken&)>&& func)
        : Transaction(parent, Type::Lambda), m_func(std::move(func)) {}
    };
private:
    std::list<std::shared_ptr<Transaction>> m_pendingQueue;
    std::list<std::shared_ptr<Transaction>> m_completedQueue;
    int m_inProgress = 0;
    bool m_running = true;

    struct Worker
    {
        ClientProcess& m_proc;
        int m_idx;
        std::thread m_thr;
        BlenderToken m_blendTok;
        bool m_didInit = false;
        Worker(ClientProcess& proc, int idx);
        void proc();
    };
    std::vector<Worker> m_workers;

public:
    ClientProcess(int verbosityLevel=1);
    ~ClientProcess() {shutdown();}
    std::shared_ptr<const BufferTransaction>
    addBufferTransaction(const hecl::ProjectPath& path, void* target,
                         size_t maxLen, size_t offset);
    std::shared_ptr<const CookTransaction>
    addCookTransaction(const hecl::ProjectPath& path, Database::IDataSpec* spec);
    std::shared_ptr<const LambdaTransaction>
    addLambdaTransaction(std::function<void(BlenderToken&)>&& func);
    bool syncCook(const hecl::ProjectPath& path, Database::IDataSpec* spec, BlenderToken& btok);
    void swapCompletedQueue(std::list<std::shared_ptr<Transaction>>& queue);
    void waitUntilComplete();
    void shutdown();
};

}

#endif // HECL_CLIENT_PROCESS_HPP
