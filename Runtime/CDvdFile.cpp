#include "Runtime/CDvdFile.hpp"

// #include <optick.h>

#include <SDL3/SDL_iostream.h>

#include <cstring>
#include <limits>
#include <new>

#include "Runtime/CDvdRequest.hpp"
#include "Runtime/CStopwatch.hpp"

namespace metaforce {
namespace {

struct SDLDiscStreamCtx {
  SDL_IOStream* io = nullptr;
};

int64_t sdlStreamReadAt(void* userData, uint64_t offset, void* out, size_t len) {
  auto* ctx = static_cast<SDLDiscStreamCtx*>(userData);
  if (ctx == nullptr || ctx->io == nullptr || offset > uint64_t(std::numeric_limits<int64_t>::max())) {
    return -1;
  }

  if (SDL_SeekIO(ctx->io, static_cast<Sint64>(offset), SDL_IO_SEEK_SET) < 0) {
    return -1;
  }

  size_t total = 0;
  auto* dst = static_cast<Uint8*>(out);
  while (total < len) {
    const size_t read = SDL_ReadIO(ctx->io, dst + total, len - total);
    if (read == 0) {
      break;
    }
    total += read;
  }
  return static_cast<int64_t>(total);
}

int64_t sdlStreamLen(void* userData) {
  auto* ctx = static_cast<SDLDiscStreamCtx*>(userData);
  if (ctx == nullptr || ctx->io == nullptr) {
    return -1;
  }
  const Sint64 size = SDL_GetIOSize(ctx->io);
  return size < 0 ? -1 : static_cast<int64_t>(size);
}

void sdlStreamClose(void* userData) {
  auto* ctx = static_cast<SDLDiscStreamCtx*>(userData);
  if (ctx == nullptr) {
    return;
  }
  if (ctx->io != nullptr) {
    SDL_CloseIO(ctx->io);
  }
  delete ctx;
}

u32 nodReadLoop(NodHandle* reader, void* buf, u32 len) {
  if (reader == nullptr || buf == nullptr || len == 0) {
    return 0;
  }

  auto* out = static_cast<uint8_t*>(buf);
  u32 totalRead = 0;
  while (totalRead < len) {
    const u32 remaining = len - totalRead;
    const int64_t read = nod_read(reader, out + totalRead, remaining);
    if (read <= 0) {
      break;
    }
    if (read > int64_t(remaining)) {
      totalRead = len;
      break;
    }
    totalRead += u32(read);
  }

  return totalRead;
}

} // namespace

CDvdFile::NodHandleUnique CDvdFile::m_DvdRoot{nullptr, nod_free};
CDvdFile::NodHandleUnique CDvdFile::m_DataPartition{nullptr, nod_free};
std::unordered_map<std::string, CDvdFile::SFileEntry> CDvdFile::m_FileEntries;

class CFileDvdRequest : public IDvdRequest {
  std::shared_ptr<NodHandle> m_reader;
  uint64_t m_begin;
  uint64_t m_size;

  void* m_buf;
  u32 m_len;
  ESeekOrigin m_whence;
  int m_offset;

#ifdef HAS_DVD_THREAD
  std::atomic_bool m_cancel = {false};
  std::atomic_bool m_complete = {false};
#else
  bool m_cancel = false;
  bool m_complete = false;
#endif
  std::function<void(u32)> m_callback;

public:
  ~CFileDvdRequest() override { CFileDvdRequest::PostCancelRequest(); }

  void WaitUntilComplete() override {
#ifdef HAS_DVD_THREAD
    while (!m_complete.load() && !m_cancel.load()) {
      std::unique_lock lk{CDvdFile::m_WaitMutex};
    }
#else
    if (!m_complete && !m_cancel) {
      CDvdFile::DoWork();
    }
#endif
  }
  bool IsComplete() override {
#ifdef HAS_DVD_THREAD
    return m_complete.load();
#else
    if (!m_complete) {
      CDvdFile::DoWork();
    }
    return m_complete;
#endif
  }
  void PostCancelRequest() override {
#ifdef HAS_DVD_THREAD
    if (m_complete.load() || m_cancel.load()) {
      return;
    }
    std::unique_lock waitlk{CDvdFile::m_WaitMutex};
    m_cancel.store(true);
#else
    m_cancel = true;
#endif
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
#ifdef HAS_DVD_THREAD
    if (m_cancel.load()) {
      return;
    }
#else
    if (m_cancel) {
      return;
    }
#endif

    if (!m_reader) {
#ifdef HAS_DVD_THREAD
      m_complete.store(true);
#else
      m_complete = true;
#endif
      if (m_callback) {
        m_callback(0);
      }
      return;
    }

    u32 readLen = 0;
    if (m_whence == ESeekOrigin::Cur && m_offset == 0) {
      readLen = nodReadLoop(m_reader.get(), m_buf, m_len);
    } else {
      int seek = 0;
      int64_t offset = m_offset;
      switch (m_whence) {
      case ESeekOrigin::Begin: {
        seek = 0;
        offset += int64_t(m_begin);
        break;
      }
      case ESeekOrigin::End: {
        seek = 0;
        offset += int64_t(m_begin) + int64_t(m_size);
        break;
      }
      case ESeekOrigin::Cur: {
        seek = 1;
        break;
      }
      };
      if (nod_seek(m_reader.get(), offset, seek) >= 0) {
        readLen = nodReadLoop(m_reader.get(), m_buf, m_len);
      }
    }

    if (m_callback) {
      m_callback(readLen);
    }
#ifdef HAS_DVD_THREAD
    m_complete.store(true);
#else
    m_complete = true;
#endif
  }
};

#ifdef HAS_DVD_THREAD
std::thread CDvdFile::m_WorkerThread;
std::mutex CDvdFile::m_WorkerMutex;
std::condition_variable CDvdFile::m_WorkerCV;
std::mutex CDvdFile::m_WaitMutex;
std::atomic_bool CDvdFile::m_WorkerRun = {false};
#endif
std::vector<std::shared_ptr<IDvdRequest>> CDvdFile::m_RequestQueue;
std::string CDvdFile::m_rootDirectory;
std::unique_ptr<u8[]> CDvdFile::m_dolBuf;
size_t CDvdFile::m_dolBufLen = 0;

CDvdFile::CDvdFile(std::string_view path) : x18_path(path) {
  const SFileEntry* entry = ResolvePath(path);
  if (entry == nullptr) {
    return;
  }

  NodHandle* fileRaw = nullptr;
  if (nod_partition_open_file(m_DataPartition.get(), entry->fstIndex, &fileRaw) == NOD_RESULT_OK &&
      fileRaw != nullptr) {
    m_reader = std::shared_ptr<NodHandle>(fileRaw, nod_free);
    m_size = entry->size;
  }
}

// single-threaded hack
void CDvdFile::DoWork() {
  for (std::shared_ptr<IDvdRequest>& req : m_RequestQueue) {
    auto& concreteReq = static_cast<CFileDvdRequest&>(*req);
    concreteReq.DoRequest();
  }
  m_RequestQueue.clear();
}

void CDvdFile::WorkerProc() {
#ifdef HAS_DVD_THREAD
  // OPTICK_THREAD("CDvdFile");

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
#endif
}

std::shared_ptr<IDvdRequest> CDvdFile::AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int off,
                                                     std::function<void(u32)>&& cb) {
  std::shared_ptr<IDvdRequest> ret = std::make_shared<CFileDvdRequest>(*this, buf, len, whence, off, std::move(cb));
#ifdef HAS_DVD_THREAD
  std::unique_lock lk{m_WorkerMutex};
#endif
  m_RequestQueue.emplace_back(ret);
#ifdef HAS_DVD_THREAD
  lk.unlock();
  m_WorkerCV.notify_one();
#endif
  return ret;
}

u32 CDvdFile::SyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int offset) {
  if (!m_reader) {
    return 0;
  }

  int seek = 0;
  int64_t seekOffset = offset;
  switch (whence) {
  case ESeekOrigin::Begin: {
    seek = 0;
    seekOffset += int64_t(m_begin);
    break;
  }
  case ESeekOrigin::End: {
    seek = 0;
    seekOffset += int64_t(m_begin) + int64_t(m_size);
    break;
  }
  case ESeekOrigin::Cur: {
    seek = 1;
    break;
  }
  };

  if (nod_seek(m_reader.get(), seekOffset, seek) < 0) {
    return 0;
  }
  return nodReadLoop(m_reader.get(), buf, len);
}

u32 CDvdFile::SyncRead(void* buf, u32 len) {
  if (!m_reader) {
    return 0;
  }
  return nodReadLoop(m_reader.get(), buf, len);
}

std::string CDvdFile::NormalizePath(std::string_view path) {
  std::string out;
  out.reserve(path.size());

  bool prevSlash = false;
  for (char c : path) {
    if (c == '/' || c == '\\') {
      if (!out.empty() && !prevSlash) {
        out.push_back('/');
      }
      prevSlash = true;
      continue;
    }

    out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    prevSlash = false;
  }

  if (!out.empty() && out.back() == '/') {
    out.pop_back();
  }
  return out;
}

const CDvdFile::SFileEntry* CDvdFile::ResolvePath(std::string_view path) {
  if (!m_DataPartition) {
    return nullptr;
  }

  std::string normalizedPath = NormalizePath(path);
  if (normalizedPath.empty()) {
    return nullptr;
  }

  if (!m_rootDirectory.empty()) {
    normalizedPath = m_rootDirectory + "/" + normalizedPath;
  }

  const auto search = m_FileEntries.find(normalizedPath);
  return search != m_FileEntries.end() ? &search->second : nullptr;
}

bool CDvdFile::BuildFileEntries() {
  if (!m_DataPartition) {
    return false;
  }

  m_FileEntries.clear();

  struct SDirFrame {
    u32 endIndex = 0;
    std::string path;
  };

  struct SFstBuildContext {
    std::unordered_map<std::string, CDvdFile::SFileEntry>* fileEntries = nullptr;
    std::vector<SDirFrame> dirStack;
  } ctx{&m_FileEntries, {}};

  nod_partition_iterate_fst(
      m_DataPartition.get(),
      [](u32 index, NodNodeKind kind, const char* name, u32 size, void* userData) -> u32 {
        auto* ctx = static_cast<SFstBuildContext*>(userData);
        while (!ctx->dirStack.empty() && index >= ctx->dirStack.back().endIndex) {
          ctx->dirStack.pop_back();
        }

        const std::string nodeName =
            CDvdFile::NormalizePath(name != nullptr ? std::string_view{name} : std::string_view{});
        if (nodeName.empty()) {
          return index + 1;
        }

        std::string fullPath;
        if (!ctx->dirStack.empty()) {
          fullPath = ctx->dirStack.back().path;
          fullPath += '/';
          fullPath += nodeName;
        } else {
          fullPath = nodeName;
        }

        if (kind == NOD_NODE_KIND_FILE) {
          ctx->fileEntries->insert_or_assign(fullPath, CDvdFile::SFileEntry{index, size});
        } else {
          ctx->dirStack.push_back({size, std::move(fullPath)});
        }
        return index + 1;
      },
      &ctx);

  return !m_FileEntries.empty();
}

bool CDvdFile::LoadDolBuf() {
  if (!m_DataPartition) {
    return false;
  }

  NodPartitionMeta meta{};
  if (nod_partition_meta(m_DataPartition.get(), &meta) != NOD_RESULT_OK || meta.raw_dol.data == nullptr ||
      meta.raw_dol.size == 0) {
    return false;
  }

  auto dolBuf = std::make_unique<u8[]>(meta.raw_dol.size);
  std::memcpy(dolBuf.get(), meta.raw_dol.data, meta.raw_dol.size);
  m_dolBuf = std::move(dolBuf);
  m_dolBufLen = meta.raw_dol.size;
  return true;
}

bool CDvdFile::Initialize(const std::string_view& path) {
#ifdef HAS_DVD_THREAD
  if (m_WorkerRun.load()) {
    return true;
  }
#endif

  Shutdown();

  std::string pathStr(path);
  SDL_IOStream* io = SDL_IOFromFile(pathStr.c_str(), "rb");
  if (io == nullptr) {
    return false;
  }

  auto* streamCtx = new (std::nothrow) SDLDiscStreamCtx{io};
  if (streamCtx == nullptr) {
    SDL_CloseIO(io);
    return false;
  }

  NodHandle* discRaw = nullptr;
  const NodDiscOptions discOpts{
      .preloader_threads = 1,
  };
  const NodDiscStream stream{
      .user_data = streamCtx,
      .read_at = sdlStreamReadAt,
      .stream_len = sdlStreamLen,
      .close = sdlStreamClose,
  };
  const NodResult discResult = nod_disc_open_stream(&stream, &discOpts, &discRaw);
  if (discResult != NOD_RESULT_OK || discRaw == nullptr) {
    sdlStreamClose(streamCtx);
    return false;
  }
  m_DvdRoot = NodHandleUnique(discRaw, nod_free);

  NodHandle* partitionRaw = nullptr;
  const NodResult partitionResult =
      nod_disc_open_partition_kind(m_DvdRoot.get(), NOD_PARTITION_KIND_DATA, nullptr, &partitionRaw);
  if (partitionResult != NOD_RESULT_OK || partitionRaw == nullptr) {
    Shutdown();
    return false;
  }
  m_DataPartition = NodHandleUnique(partitionRaw, nod_free);

  if (!BuildFileEntries() || !LoadDolBuf()) {
    Shutdown();
    return false;
  }

#ifdef HAS_DVD_THREAD
  m_WorkerRun.store(true);
  m_WorkerThread = std::thread(WorkerProc);
#endif
  return true;
}

void CDvdFile::Shutdown() {
#ifdef HAS_DVD_THREAD
  if (m_WorkerRun.load()) {
    m_WorkerRun.store(false);
    m_WorkerCV.notify_one();
    if (m_WorkerThread.joinable()) {
      m_WorkerThread.join();
    }
  }
#endif
  m_RequestQueue.clear();
  m_FileEntries.clear();
  m_dolBuf.reset();
  m_dolBufLen = 0;
  m_DataPartition.reset();
  m_DvdRoot.reset();
}

SDiscInfo CDvdFile::DiscInfo() {
  SDiscInfo out{};
  if (!m_DvdRoot) {
    return out;
  }

  NodDiscHeader header{};
  if (nod_disc_header(m_DvdRoot.get(), &header) != NOD_RESULT_OK) {
    return out;
  }

  std::memcpy(out.gameId.data(), header.game_id, sizeof(header.game_id));
  out.version = header.disc_version;
  const char* titleBegin = header.game_title;
  const char* titleEnd = std::find(titleBegin, titleBegin + sizeof(header.game_title), '\0');
  out.gameTitle.assign(titleBegin, titleEnd);
  return out;
}

void CDvdFile::SetRootDirectory(const std::string_view& rootDir) { m_rootDirectory = NormalizePath(rootDir); }

} // namespace metaforce
