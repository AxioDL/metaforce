#pragma once

#include <atomic>
#include <array>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <cstddef>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/RetroTypes.hpp"

#include <nod.h>

#ifndef EMSCRIPTEN
#define HAS_DVD_THREAD
#endif

namespace metaforce {

enum class ESeekOrigin { Begin = 0, Cur = 1, End = 2 };

struct DVDFileInfo;
class IDvdRequest;

struct SDiscInfo {
  std::array<char, 6> gameId;
  uint8_t version;
  std::string gameTitle;
};

class CDvdFile {
  friend class CResLoader;
  friend class CFileDvdRequest;
  using NodHandleUnique = std::unique_ptr<NodHandle, decltype(&nod_free)>;
  struct SFileEntry {
    u32 fstIndex = NOD_FST_STOP;
    u32 size = 0;
  };

  static NodHandleUnique m_DvdRoot;
  static NodHandleUnique m_DataPartition;
  static std::unordered_map<std::string, SFileEntry> m_FileEntries;
#ifdef HAS_DVD_THREAD
  static std::thread m_WorkerThread;
  static std::mutex m_WorkerMutex;
  static std::condition_variable m_WorkerCV;
  static std::mutex m_WaitMutex;
  static std::atomic_bool m_WorkerRun;
#endif
  static std::vector<std::shared_ptr<IDvdRequest>> m_RequestQueue;
  static std::string m_rootDirectory;
  static std::unique_ptr<u8[]> m_dolBuf;
  static size_t m_dolBufLen;
  static void WorkerProc();

  std::string x18_path;
  std::shared_ptr<NodHandle> m_reader;
  uint64_t m_begin = 0;
  uint64_t m_size = 0;

  static std::string NormalizePath(std::string_view path);
  static const SFileEntry* ResolvePath(std::string_view path);
  static bool BuildFileEntries();
  static bool LoadDolBuf();

public:
  static bool Initialize(const std::string_view& path);
  static SDiscInfo DiscInfo();
  static void SetRootDirectory(const std::string_view& rootDir);
  static void Shutdown();
  static u8* GetDolBuf() { return m_dolBuf.get(); }
  static size_t GetDolBufLen() { return m_dolBufLen; }
  static void DoWork();

  CDvdFile(std::string_view path);
  operator bool() const { return m_reader.operator bool(); }
  void UpdateFilePos(int pos) {
    if (m_reader) {
      nod_seek(m_reader.get(), pos, 0);
    }
  }
  static bool FileExists(std::string_view path) { return ResolvePath(path) != nullptr; }
  void CloseFile() { m_reader.reset(); }
  std::shared_ptr<IDvdRequest> AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int off,
                                             std::function<void(u32)>&& cb = {});
  u32 SyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int offset);
  std::shared_ptr<IDvdRequest> AsyncRead(void* buf, u32 len, std::function<void(u32)>&& cb = {}) {
    return AsyncSeekRead(buf, len, ESeekOrigin::Cur, 0, std::move(cb));
  }
  u32 SyncRead(void* buf, u32 len);
  u64 Length() const { return m_size; }
  std::string_view GetPath() const { return x18_path; }
};
} // namespace metaforce
