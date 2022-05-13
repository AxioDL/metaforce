#include "Runtime/CDvdFile.hpp"

#include <optick.h>

#include "Runtime/CDvdRequest.hpp"
#include "Runtime/CStopwatch.hpp"

namespace metaforce {

std::unique_ptr<nod::DiscBase> CDvdFile::m_DvdRoot;
// std::unordered_map<std::string, std::string> CDvdFile::m_caseInsensitiveMap;

class CFileDvdRequest : public IDvdRequest {
  std::shared_ptr<nod::IPartReadStream> m_reader;
  uint64_t m_begin;
  uint64_t m_size;

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
    if (m_complete.load() || m_cancel.load()) {
      return;
    }
    std::unique_lock waitlk{CDvdFile::m_WaitMutex};
    m_cancel.store(true);
  }

  [[nodiscard]] EMediaType GetMediaType() const override { return EMediaType::File; }

  CFileDvdRequest(CDvdFile& file, void* buf, u32 len, ESeekOrigin whence, int off, std::function<void(u32)>&& cb)
  : m_reader(file.m_reader)
  , m_begin(file.m_begin)
  , m_size(file.m_size)
  , m_buf(buf)
  , m_len(len)
  , m_whence(whence)
  , m_offset(off)
  , m_callback(std::move(cb)) {}

  void DoRequest() {
    if (m_cancel.load()) {
      return;
    }
    u32 readLen = 0;
    if (m_whence == ESeekOrigin::Cur && m_offset == 0) {
      readLen = m_reader->read(m_buf, m_len);
    } else {
      int seek = 0;
      int64_t offset = m_offset;
      switch (m_whence) {
      case ESeekOrigin::Begin: {
        seek = SEEK_SET;
        offset += int64_t(m_begin);
        break;
      }
      case ESeekOrigin::End: {
        seek = SEEK_SET;
        offset += int64_t(m_begin) + int64_t(m_size);
        break;
      }
      case ESeekOrigin::Cur: {
        seek = SEEK_CUR;
        break;
      }
      };
      m_reader->seek(offset, seek);
      readLen = m_reader->read(m_buf, m_len);
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
std::string CDvdFile::m_rootDirectory;
std::unique_ptr<u8[]> CDvdFile::m_dolBuf;

CDvdFile::CDvdFile(std::string_view path) : x18_path(path) {
  auto* node = ResolvePath(path);
  if (node != nullptr && node->getKind() == nod::Node::Kind::File) {
    m_reader = node->beginReadStream();
    m_begin = m_reader->position();
    m_size = node->size();
  }
}

void CDvdFile::WorkerProc() {
  logvisor::RegisterThreadName("CDvdFile");
  OPTICK_THREAD("CDvdFile");

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

u32 CDvdFile::SyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int offset) {
  int seek = 0;
  switch (whence) {
  case ESeekOrigin::Begin: {
    seek = SEEK_SET;
    offset += int64_t(m_begin);
    break;
  }
  case ESeekOrigin::End: {
    seek = SEEK_SET;
    offset += int64_t(m_begin) + int64_t(m_size);
    break;
  }
  case ESeekOrigin::Cur: {
    seek = SEEK_CUR;
    break;
  }
  };
  m_reader->seek(offset, seek);
  return m_reader->read(buf, len);
}

nod::Node* CDvdFile::ResolvePath(std::string_view path) {
  if (!m_DvdRoot) {
    return nullptr;
  }
  if (path.starts_with('/')) {
    path.remove_prefix(1);
  }
  std::string prefixedPath;
  if (!m_rootDirectory.empty()) {
    prefixedPath = m_rootDirectory;
    prefixedPath += '/';
    prefixedPath += path;
    path = prefixedPath;
  }
  auto* node = &m_DvdRoot->getDataPartition()->getFSTRoot();
  while (node != nullptr && !path.empty()) {
    std::string component;
    auto end = path.find('/');
    if (end != std::string_view::npos) {
      component = path.substr(0, end);
      path.remove_prefix(component.size() + 1);
    } else {
      component = path;
      path.remove_prefix(component.size());
    }
    std::transform(component.begin(), component.end(), component.begin(), ::tolower);
    auto* tmpNode = node;
    node = nullptr;
    for (auto& item : *tmpNode) {
      const auto name = item.getName();
      if (std::equal(component.begin(), component.end(), name.begin(), name.end(),
                     [](char a, char b) { return a == tolower(b); })) {
        node = &item;
        break;
      }
    }
  }
  return node;
}

bool CDvdFile::Initialize(const std::string_view& path) {
  if (m_WorkerRun.load()) {
    return true;
  }
  m_DvdRoot = nod::OpenDiscFromImage(path);
  if (!m_DvdRoot) {
    return false;
  }
  m_dolBuf = m_DvdRoot->getDataPartition()->getDOLBuf();
  m_WorkerRun.store(true);
  m_WorkerThread = std::thread(WorkerProc);
  return true;
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

SDiscInfo CDvdFile::DiscInfo() {
  SDiscInfo out{};
  if (!m_DvdRoot) {
    return out;
  }
  const auto& header = m_DvdRoot->getHeader();
  std::memcpy(out.gameId.data(), header.m_gameID, sizeof(header.m_gameID));
  out.version = header.m_discVersion;
  out.gameTitle = header.m_gameTitle;
  return out;
}

void CDvdFile::SetRootDirectory(const std::string_view& rootDir) { m_rootDirectory = rootDir; }

} // namespace metaforce
