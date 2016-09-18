#ifndef __URDE_CDVDFILE_HPP__
#define __URDE_CDVDFILE_HPP__

#include "RetroTypes.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace urde
{

static const char* DecodeARAMFile(const char* name)
{
    return (strncmp(name, "aram:", 5) == 0 ? name + 5 : name);
}

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
    static bool m_WorkerRun;
    static std::vector<std::shared_ptr<IDvdRequest>> m_RequestQueue;
    static void WorkerProc();

    std::string x18_path;
    athena::io::FileReader m_reader;

public:
    static void Initialize(const hecl::ProjectPath& path);
    static void Shutdown();

    CDvdFile(const char* path)
    : x18_path(path), m_reader(hecl::ProjectPath(m_DvdRoot, path).getAbsolutePath()) {}
    void UpdateFilePos(int pos)
    {
        m_reader.seek(pos, athena::SeekOrigin::Begin);
    }
    static bool FileExists(const char* path)
    {
        return hecl::ProjectPath(m_DvdRoot, path).isFile();
    }
    void CloseFile()
    {
        m_reader.close();
    }
    std::shared_ptr<IDvdRequest> AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int off);
    void SyncSeekRead(void* buf, u32 len, ESeekOrigin whence, int offset)
    {
        m_reader.seek(offset, athena::SeekOrigin(whence));
        m_reader.readBytesToBuf(buf, len);
    }
    std::shared_ptr<IDvdRequest> AsyncRead(void* buf, u32 len)
    {
        return AsyncSeekRead(buf, len, ESeekOrigin::Cur, 0);
    }
    void SyncRead(void* buf, u32 len)
    {
        m_reader.readBytesToBuf(buf, len);
    }
    u64 Length() {return m_reader.length();}
};
}

#endif // __URDE_CDVDFILE_HPP__
