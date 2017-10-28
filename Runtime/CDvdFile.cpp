#include "CDvdFile.hpp"
#include "CDvdRequest.hpp"

namespace urde
{

hecl::ProjectPath CDvdFile::m_DvdRoot;

class CFileDvdRequest : public IDvdRequest
{
    std::shared_ptr<athena::io::FileReader> m_reader;
    void* m_buf;
    u32 m_len;
    ESeekOrigin m_whence;
    int m_offset;
    bool m_cancel = false;
    bool m_complete = false;
    std::function<void(u32)> m_callback;
public:
    ~CFileDvdRequest()
    {
        PostCancelRequest();
    }

    void WaitUntilComplete()
    {
        while (!m_complete && !m_cancel)
        {
            std::unique_lock<std::mutex> lk(CDvdFile::m_WaitMutex);
        }
    }
    bool IsComplete() {return m_complete;}
    void PostCancelRequest()
    {
        std::unique_lock<std::mutex> waitlk(CDvdFile::m_WaitMutex);
        m_cancel = true;
    }

    EMediaType GetMediaType() const
    {
        return EMediaType::File;
    }

    CFileDvdRequest(CDvdFile& file, void* buf, u32 len, ESeekOrigin whence, int off, std::function<void(u32)>&& cb)
    : m_reader(file.m_reader), m_buf(buf), m_len(len), m_whence(whence), m_offset(off), m_callback(std::move(cb)) {}

    void DoRequest()
    {
        if (m_cancel)
            return;
        u32 readLen;
        if (m_whence == ESeekOrigin::Cur && m_offset == 0)
        {
            readLen = m_reader->readBytesToBuf(m_buf, m_len);
        }
        else
        {
            m_reader->seek(m_offset, athena::SeekOrigin(m_whence));
            readLen = m_reader->readBytesToBuf(m_buf, m_len);
        }
        if (m_callback)
            m_callback(readLen);
        m_complete = true;
    }
};

std::thread CDvdFile::m_WorkerThread;
std::mutex CDvdFile::m_WorkerMutex;
std::condition_variable CDvdFile::m_WorkerCV;
std::mutex CDvdFile::m_WaitMutex;
bool CDvdFile::m_WorkerRun = false;
std::vector<std::shared_ptr<IDvdRequest>> CDvdFile::m_RequestQueue;
void CDvdFile::WorkerProc()
{
    logvisor::RegisterThreadName("CDvdFile Thread");
    while (m_WorkerRun)
    {
        std::unique_lock<std::mutex> lk(CDvdFile::m_WorkerMutex);
        while (CDvdFile::m_RequestQueue.size())
        {
            std::vector<std::shared_ptr<IDvdRequest>> swapQueue;
            swapQueue.swap(CDvdFile::m_RequestQueue);
            lk.unlock();
            std::unique_lock<std::mutex> waitlk(CDvdFile::m_WaitMutex);
            for (std::shared_ptr<IDvdRequest>& req : swapQueue)
            {
                CFileDvdRequest& concreteReq = static_cast<CFileDvdRequest&>(*req);
                concreteReq.DoRequest();
            }
            waitlk.unlock();
            swapQueue.clear();
            lk.lock();
        }
        if (!m_WorkerRun)
            break;
        m_WorkerCV.wait(lk);
    }
}

std::shared_ptr<IDvdRequest> CDvdFile::AsyncSeekRead(void* buf, u32 len, ESeekOrigin whence,
                                                     int off, std::function<void(u32)>&& cb)
{
    std::shared_ptr<IDvdRequest> ret =
        std::make_shared<CFileDvdRequest>(*this, buf, len, whence, off, std::move(cb));
    std::unique_lock<std::mutex> lk(CDvdFile::m_WorkerMutex);
    m_RequestQueue.emplace_back(ret);
    lk.unlock();
    m_WorkerCV.notify_one();
    return ret;
}

void CDvdFile::Initialize(const hecl::ProjectPath& path)
{
    m_DvdRoot = path;
    if (m_WorkerRun)
        return;
    m_WorkerRun = true;
    m_WorkerThread = std::thread(WorkerProc);
}

void CDvdFile::Shutdown()
{
    if (!m_WorkerRun)
        return;
    m_WorkerRun = false;
    m_WorkerCV.notify_one();
    if (m_WorkerThread.joinable())
        m_WorkerThread.join();
    m_RequestQueue.clear();
}

}
