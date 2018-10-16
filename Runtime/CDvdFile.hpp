#pragma once

#include "RetroTypes.hpp"
#include "athena/FileReader.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace urde
{

enum class ESeekOrigin
{
    Begin = 0,
    Cur = 1,
    End = 2
};

struct DVDFileInfo;
class IDvdRequest;

class CDvdFile
{
    friend class CResLoader;
    friend class CFileDvdRequest;
    static hecl::ProjectPath m_DvdRoot;
    static std::thread m_WorkerThread;
    static std::mutex m_WorkerMutex;
    static std::condition_variable m_WorkerCV;
    static std::mutex m_WaitMutex;
    static std::atomic_bool m_WorkerRun;
    static std::vector<std::shared_ptr<IDvdRequest>> m_RequestQueue;
    static void WorkerProc();

    std::string x18_path;
    std::shared_ptr<athena::io::FileReader> m_reader;

public:
    static void Initialize(const hecl::ProjectPath& path);
    static void Shutdown();

    CDvdFile(std::string_view path)
        : x18_path(path),
          m_reader(std::make_shared<athena::io::FileReader>(
                   hecl::ProjectPath(m_DvdRoot, path).getAbsolutePath())) {}
    operator bool() const { return m_reader->isOpen(); }
    void UpdateFilePos(int pos)
    {
        m_reader->seek(pos, athena::SeekOrigin::Begin);
    }
    static bool FileExists(std::string_view path)
    {
        return hecl::ProjectPath(m_DvdRoot, path).isFile();
    }
    void CloseFile()
    {
        m_reader->close();
    }
    std::shared_ptr<IDvdRequest> AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence,
                                               int off, std::function<void(u32)>&& cb = {});
    u32 SyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int offset)
    {
        m_reader->seek(offset, athena::SeekOrigin(whence));
        return m_reader->readBytesToBuf(buf, len);
    }
    std::shared_ptr<IDvdRequest> AsyncRead(void* buf, u32 len, std::function<void(u32)>&& cb = {})
    {
        return AsyncSeekRead(buf, len, ESeekOrigin::Cur, 0, std::move(cb));
    }
    u32 SyncRead(void* buf, u32 len)
    {
        return m_reader->readBytesToBuf(buf, len);
    }
    u64 Length() {return m_reader->length();}
    std::string_view GetPath() const { return x18_path; }
};
}

