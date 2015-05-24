#ifndef CBLENDERCONNECTION_HPP
#define CBLENDERCONNECTION_HPP

#if _WIN32
#define _WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <unistd.h>
#endif

class CBlenderConnection
{
#if _WIN32
    HANDLE m_blenderProc;
    HANDLE m_readpipe;
    HANDLE m_writepipe;
#else
    pid_t m_blenderProc;
    int m_readpipe[2];
    int m_writepipe[2];
#endif
    size_t readLine(char* buf, size_t bufSz);
    size_t writeLine(const char* buf);
    size_t readBuf(char* buf, size_t len);
    size_t writeBuf(const char* buf, size_t len);
    void closePipe();
public:
    CBlenderConnection(bool silenceBlender=false);
    ~CBlenderConnection();

    void quitBlender();
};

#endif // CBLENDERCONNECTION_HPP
