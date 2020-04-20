#include "Runtime/CDvdFile.hpp"

#include "Runtime/CDvdRequest.hpp"
#include "Runtime/CStopwatch.hpp"

namespace urde {

hecl::ProjectPath CDvdFile::m_DvdRoot;
std::unordered_map<std::string, std::string> CDvdFile::m_caseInsensitiveMap;

class CFileDvdRequest : public IDvdRequest {
  std::shared_ptr<athena::io::FileReader> m_reader;
  void* m_buf;
  u32 m_len;
  ESeekOrigin m_whence;
  int m_offset;
  std::atomic_bool m_cancel = {false};
  std::atomic_bool m_complete = {false};
  std::function<void(u32)> m_callback;

public:
  ~CFileDvdRequest() override { CFileDvdRequest::PostCancelRequest(); }

  void WaitUntilComplete() override {
    while (!m_complete.load() && !m_cancel.load()) {
      std::unique_lock lk{CDvdFile::m_WaitMutex};
    }
  }
  bool IsComplete() override { return m_complete.load(); }
  void PostCancelRequest() override {
    std::unique_lock waitlk{CDvdFile::m_WaitMutex};
    m_cancel.store(true);
  }

  [[nodiscard]] EMediaType GetMediaType() const override { return EMediaType::File; }

  CFileDvdRequest(CDvdFile& file, void* buf, u32 len, ESeekOrigin whence, int off, std::function<void(u32)>&& cb)
  : m_reader(file.m_reader), m_buf(buf), m_len(len), m_whence(whence), m_offset(off), m_callback(std::move(cb)) {}

  void DoRequest() {
    if (m_cancel.load()) {
      return;
    }
    u32 readLen;
    if (m_whence == ESeekOrigin::Cur && m_offset == 0) {
      readLen = m_reader->readBytesToBuf(m_buf, m_len);
    } else {
      m_reader->seek(m_offset, athena::SeekOrigin(m_whence));
      readLen = m_reader->readBytesToBuf(m_buf, m_len);
    }
    if (m_callback) {
      m_callback(readLen);
    }
    m_complete.store(true);
  }
};

std::thread CDvdFile::m_WorkerThread;
std::mutex CDvdFile::m_WorkerMutex;
std::condition_variable CDvdFile::m_WorkerCV;
std::mutex CDvdFile::m_WaitMutex;
std::atomic_bool CDvdFile::m_WorkerRun = {false};
std::vector<std::shared_ptr<IDvdRequest>> CDvdFile::m_RequestQueue;

void CDvdFile::WorkerProc() {
  logvisor::RegisterThreadName("CDvdFile");
  while (m_WorkerRun.load()) {
    std::unique_lock lk{m_WorkerMutex};
    while (!m_RequestQueue.empty()) {
      std::vector<std::shared_ptr<IDvdRequest>> swapQueue;
      swapQueue.swap(m_RequestQueue);
      lk.unlock();
      std::unique_lock waitlk{m_WaitMutex};
      for (std::shared_ptr<IDvdRequest>& req : swapQueue) {
        auto& concreteReq = static_cast<CFileDvdRequest&>(*req);
        concreteReq.DoRequest();
      }
      waitlk.unlock();
      swapQueue.clear();
      lk.lock();
    }
    if (!m_WorkerRun.load()) {
      break;
    }
    m_WorkerCV.wait(lk);
  }
}

std::shared_ptr<IDvdRequest> CDvdFile::AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int off,
                                                     std::function<void(u32)>&& cb) {
  std::shared_ptr<IDvdRequest> ret = std::make_shared<CFileDvdRequest>(*this, buf, len, whence, off, std::move(cb));
  std::unique_lock lk{m_WorkerMutex};
  m_RequestQueue.emplace_back(ret);
  lk.unlock();
  m_WorkerCV.notify_one();
  return ret;
}

hecl::ProjectPath CDvdFile::ResolvePath(std::string_view path) {
  auto start = path.begin();
  while (*start == '/') {
    ++start;
  }
  std::string lowerChStr(start, path.end());
  std::transform(lowerChStr.begin(), lowerChStr.end(), lowerChStr.begin(), ::tolower);
  auto search = m_caseInsensitiveMap.find(lowerChStr);
  if (search == m_caseInsensitiveMap.end()) {
    return {};
  }
  return hecl::ProjectPath(m_DvdRoot, search->second);
}

void CDvdFile::RecursiveBuildCaseInsensitiveMap(const hecl::ProjectPath& path, std::string::size_type prefixLen) {
  for (const auto& p : path.enumerateDir()) {
    if (p.m_isDir) {
      RecursiveBuildCaseInsensitiveMap(hecl::ProjectPath(path, p.m_name), prefixLen);
    } else {
      hecl::ProjectPath ch(path, p.m_name);
      std::string chStr(ch.getAbsolutePathUTF8().begin() + prefixLen, ch.getAbsolutePathUTF8().end());
      std::string lowerChStr(chStr);
      std::transform(lowerChStr.begin(), lowerChStr.end(), lowerChStr.begin(), ::tolower);
      m_caseInsensitiveMap[lowerChStr] = chStr;
    }
  }
}

void CDvdFile::Initialize(const hecl::ProjectPath& path) {
  m_DvdRoot = path;
  RecursiveBuildCaseInsensitiveMap(path, path.getAbsolutePathUTF8().length() + 1);
  if (m_WorkerRun.load()) {
    return;
  }
  m_WorkerRun.store(true);
  m_WorkerThread = std::thread(WorkerProc);
}

void CDvdFile::Shutdown() {
  if (!m_WorkerRun.load()) {
    return;
  }
  m_WorkerRun.store(false);
  m_WorkerCV.notify_one();
  if (m_WorkerThread.joinable()) {
    m_WorkerThread.join();
  }
  m_RequestQueue.clear();
}

} // namespace urde
