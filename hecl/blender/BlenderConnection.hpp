#ifndef CBLENDERCONNECTION_HPP
#define CBLENDERCONNECTION_HPP

#if _WIN32
#define _WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <string>
#include <functional>

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
    std::string m_loadedBlend;
    size_t _readLine(char* buf, size_t bufSz);
    size_t _writeLine(const char* buf);
    size_t _readBuf(char* buf, size_t len);
    size_t _writeBuf(const char* buf, size_t len);
    void _closePipe();
public:
    CBlenderConnection(bool silenceBlender=false);
    ~CBlenderConnection();

    bool openBlend(const std::string& path);
    enum CookPlatform
    {
        CP_MODERN = 0,
        CP_GX     = 1,
    };
    bool cookBlend(std::function<char*(uint32_t)> bufGetter,
                   const std::string& expectedType,
                   const std::string& platform,
                   bool bigEndian=false);
    void quitBlender();
};

#endif // CBLENDERCONNECTION_HPP
