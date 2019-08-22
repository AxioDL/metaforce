#pragma once

#include "hecl.hpp"
#include "Database.hpp"
#include "boo/ThreadLocalPtr.hpp"
#include "hecl/Blender/Token.hpp"
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace hecl {

extern int CpuCountOverride;
void SetCpuCountOverride(int argc, const SystemChar** argv);

class ClientProcess {
  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::condition_variable m_initCv;
  std::condition_variable m_waitCv;
  const MultiProgressPrinter* m_progPrinter;
  int m_completedCooks = 0;
  int m_addedCooks = 0;

public:
  struct Transaction {
    ClientProcess& m_parent;
    enum class Type { Buffer, Cook, Lambda } m_type;
    bool m_complete = false;
    virtual void run(blender::Token& btok) = 0;
    Transaction(ClientProcess& parent, Type tp) : m_parent(parent), m_type(tp) {}
  };
  struct BufferTransaction final : Transaction {
    ProjectPath m_path;
    void* m_targetBuf;
    size_t m_maxLen;
    size_t m_offset;
    void run(blender::Token& btok) override;
    BufferTransaction(ClientProcess& parent, const ProjectPath& path, void* target, size_t maxLen, size_t offset)
    : Transaction(parent, Type::Buffer), m_path(path), m_targetBuf(target), m_maxLen(maxLen), m_offset(offset) {}
  };
  struct CookTransaction final : Transaction {
    ProjectPath m_path;
    Database::IDataSpec* m_dataSpec;
    bool m_returnResult = false;
    bool m_force;
    bool m_fast;
    void run(blender::Token& btok) override;
    CookTransaction(ClientProcess& parent, const ProjectPath& path, bool force, bool fast, Database::IDataSpec* spec)
    : Transaction(parent, Type::Cook), m_path(path), m_dataSpec(spec), m_force(force), m_fast(fast) {}
  };
  struct LambdaTransaction final : Transaction {
    std::function<void(blender::Token&)> m_func;
    void run(blender::Token& btok) override;
    LambdaTransaction(ClientProcess& parent, std::function<void(blender::Token&)>&& func)
    : Transaction(parent, Type::Lambda), m_func(std::move(func)) {}
  };

private:
  std::list<std::shared_ptr<Transaction>> m_pendingQueue;
  std::list<std::shared_ptr<Transaction>> m_completedQueue;
  int m_inProgress = 0;
  bool m_running = true;

  struct Worker {
    ClientProcess& m_proc;
    int m_idx;
    std::thread m_thr;
    blender::Token m_blendTok;
    bool m_didInit = false;
    Worker(ClientProcess& proc, int idx);
    void proc();
  };
  std::vector<Worker> m_workers;
  static ThreadLocalPtr<ClientProcess::Worker> ThreadWorker;

public:
  ClientProcess(const MultiProgressPrinter* progPrinter = nullptr);
  ~ClientProcess() { shutdown(); }
  std::shared_ptr<const BufferTransaction> addBufferTransaction(const hecl::ProjectPath& path, void* target,
                                                                size_t maxLen, size_t offset);
  std::shared_ptr<const CookTransaction> addCookTransaction(const hecl::ProjectPath& path, bool force, bool fast,
                                                            Database::IDataSpec* spec);
  std::shared_ptr<const LambdaTransaction> addLambdaTransaction(std::function<void(blender::Token&)>&& func);
  bool syncCook(const hecl::ProjectPath& path, Database::IDataSpec* spec, blender::Token& btok, bool force, bool fast);
  void swapCompletedQueue(std::list<std::shared_ptr<Transaction>>& queue);
  void waitUntilComplete();
  void shutdown();
  bool isBusy() const { return m_pendingQueue.size() || m_inProgress; }

  static int GetThreadWorkerIdx() {
    Worker* w = ThreadWorker.get();
    if (w)
      return w->m_idx;
    return -1;
  }
};

} // namespace hecl
