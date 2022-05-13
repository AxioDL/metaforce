#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/RetroTypes.hpp"

//#include <athena/FileReader.hpp>
#include <nod/nod.hpp>
#include <nod/DiscBase.hpp>

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
  static std::unique_ptr<nod::DiscBase> m_DvdRoot;
  //  static std::unordered_map<std::string, std::string> m_caseInsensitiveMap;
  static std::thread m_WorkerThread;
  static std::mutex m_WorkerMutex;
  static std::condition_variable m_WorkerCV;
  static std::mutex m_WaitMutex;
  static std::atomic_bool m_WorkerRun;
  static std::vector<std::shared_ptr<IDvdRequest>> m_RequestQueue;
  static std::string m_rootDirectory;
  static std::unique_ptr<u8[]> m_dolBuf;
  static void WorkerProc();

  std::string x18_path;
  std::shared_ptr<nod::IPartReadStream> m_reader;
  uint64_t m_begin;
  uint64_t m_size;

  static nod::Node* ResolvePath(std::string_view path);
  //  static void RecursiveBuildCaseInsensitiveMap(const hecl::ProjectPath& path, std::string::size_type prefixLen);

public:
  static bool Initialize(const std::string_view& path);
  static SDiscInfo DiscInfo();
  static void SetRootDirectory(const std::string_view& rootDir);
  static void Shutdown();
  static u8* GetDolBuf() { return m_dolBuf.get(); }

  CDvdFile(std::string_view path);
  operator bool() const { return m_reader.operator bool(); }
  void UpdateFilePos(int pos) { m_reader->seek(pos, SEEK_SET); }
  static bool FileExists(std::string_view path) {
    nod::Node* node = ResolvePath(path);
    return node != nullptr && node->getKind() == nod::Node::Kind::File;
  }
  void CloseFile() { m_reader.reset(); }
  std::shared_ptr<IDvdRequest> AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int off,
                                             std::function<void(u32)>&& cb = {});
  u32 SyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int offset);
  std::shared_ptr<IDvdRequest> AsyncRead(void* buf, u32 len, std::function<void(u32)>&& cb = {}) {
    return AsyncSeekRead(buf, len, ESeekOrigin::Cur, 0, std::move(cb));
  }
  u32 SyncRead(void* buf, u32 len) { return m_reader->read(buf, len); }
  u64 Length() const { return m_size; }
  std::string_view GetPath() const { return x18_path; }
};
} // namespace metaforce
