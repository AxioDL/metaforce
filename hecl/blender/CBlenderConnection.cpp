#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system_error>
#include <string>

#include "CBlenderConnection.hpp"

#ifdef __APPLE__
#define DEFAULT_BLENDER_BIN "/Applications/Blender.app/Contents/MacOS/blender"
#elif _WIN32
#define DEFAULT_BLENDER_BIN "%ProgramFiles%\\Blender Foundation\\Blender\\blender.exe"
#else
#define DEFAULT_BLENDER_BIN "blender"
#endif

#define TEMP_SHELLSCRIPT "/home/jacko/hecl/blender/blendershell.py"

size_t CBlenderConnection::_readLine(char* buf, size_t bufSz)
{
    size_t readBytes = 0;
    while (true)
    {
        if (readBytes >= bufSz)
            throw std::length_error("Pipe buffer overrun");
        ssize_t ret = read(m_readpipe[0], buf, 1);
        if (ret < 0)
            goto err;
        else if (ret == 1)
        {
            if (*buf == '\n')
            {
                *buf = '\0';
                return readBytes;
            }
            ++readBytes;
            ++buf;
        }
        else
        {
            *buf = '\0';
            return readBytes;
        }
    }
err:
    throw std::error_code(errno, std::system_category());
    return 0;
}

size_t CBlenderConnection::_writeLine(const char* buf)
{
    ssize_t ret, nlerr;
    ret = write(m_writepipe[1], buf, strlen(buf));
    if (ret < 0)
        goto err;
    nlerr = write(m_writepipe[1], "\n", 1);
    if (nlerr < 0)
        goto err;
    return (size_t)ret;
err:
    throw std::error_code(errno, std::system_category());
}

size_t CBlenderConnection::_readBuf(char* buf, size_t len)
{
    ssize_t ret = read(m_readpipe[0], buf, len);
    if (ret < 0)
        throw std::error_code(errno, std::system_category());
    return ret;
}

size_t CBlenderConnection::_writeBuf(const char* buf, size_t len)
{
    ssize_t ret = write(m_writepipe[1], buf, len);
    if (ret < 0)
        throw std::error_code(errno, std::system_category());
    return ret;
}

void CBlenderConnection::_closePipe()
{
    close(m_readpipe[0]);
    close(m_writepipe[1]);
}

CBlenderConnection::CBlenderConnection(bool silenceBlender)
{
    /* Construct communication pipes */
    pipe(m_readpipe);
    pipe(m_writepipe);

    /* User-specified blender path */
    char* blenderBin = getenv("BLENDER_BIN");

    /* Child process of blender */
    pid_t pid = fork();
    if (!pid)
    {
        close(m_writepipe[1]);
        close(m_readpipe[0]);

        if (silenceBlender)
        {
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
        }

        char errbuf[256];
        char readfds[32];
        snprintf(readfds, 32, "%d", m_writepipe[0]);
        char writefds[32];
        snprintf(writefds, 32, "%d", m_readpipe[1]);

        /* Try user-specified blender first */
        if (blenderBin)
        {
            execlp(blenderBin, blenderBin,
                   "--background", "-P", TEMP_SHELLSCRIPT,
                   "--", readfds, writefds, NULL);
            if (errno != ENOENT)
            {
                snprintf(errbuf, 256, "NOLAUNCH %s\n", strerror(errno));
                write(m_writepipe[1], errbuf, strlen(errbuf));
                exit(1);
            }
        }

        /* Otherwise default blender */
        execlp(DEFAULT_BLENDER_BIN, DEFAULT_BLENDER_BIN,
               "--background", "-P", TEMP_SHELLSCRIPT,
               "--", readfds, writefds, NULL);
        if (errno != ENOENT)
        {
            snprintf(errbuf, 256, "NOLAUNCH %s\n", strerror(errno));
            write(m_writepipe[1], errbuf, strlen(errbuf));
            exit(1);
        }

        /* Unable to find blender */
        write(m_writepipe[1], "NOBLENDER\n", 10);
        exit(1);

    }
    close(m_writepipe[0]);
    close(m_readpipe[1]);
    m_blenderProc = pid;

    /* Handle first response */
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "NOLAUNCH"))
    {
        _closePipe();
        throw std::runtime_error("Unable to launch blender");
    }
    else if (!strcmp(lineBuf, "NOBLENDER"))
    {
        _closePipe();
        if (blenderBin)
            throw std::runtime_error("Unable to find blender at '" +
                                     std::string(blenderBin) + "' or '" +
                                     std::string(DEFAULT_BLENDER_BIN) + "'");
        else
            throw std::runtime_error("Unable to find blender at '" +
                                     std::string(DEFAULT_BLENDER_BIN) + "'");
    }
    else if (!strcmp(lineBuf, "NOADDON"))
    {
        _closePipe();
        throw std::runtime_error("HECL addon not installed within blender");
    }
    else if (strcmp(lineBuf, "READY"))
    {
        _closePipe();
        throw std::runtime_error("read '" + std::string(lineBuf) +
                                 "' from blender; expected 'READY'");
    }
    _writeLine("ACK");

}

CBlenderConnection::~CBlenderConnection()
{
    _closePipe();
}

bool CBlenderConnection::openBlend(const std::string& path)
{
    _writeLine(("OPEN" + path).c_str());
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
    {
        m_loadedBlend = path;
        return true;
    }
    return false;
}

bool CBlenderConnection::cookBlend(std::function<char*(uint32_t)> bufGetter,
                                   const std::string& expectedType,
                                   const std::string& platform,
                                   bool bigEndian)
{
    char lineBuf[256];
    char reqLine[512];
    snprintf(reqLine, 512, "COOK %s %c", platform.c_str(), bigEndian?'>':'<');
    _writeLine(reqLine);
    _readLine(lineBuf, sizeof(lineBuf));
    if (strcmp(expectedType.c_str(), lineBuf))
    {
        throw std::runtime_error("expected '" + m_loadedBlend +
                                 "' to contain " + expectedType +
                                 " not " + lineBuf);
        return false;
    }
    _writeLine("ACK");

    for (_readLine(lineBuf, sizeof(lineBuf));
         !strcmp("BUF", lineBuf);
         _readLine(lineBuf, sizeof(lineBuf)))
    {
        uint32_t sz;
        _readBuf((char*)&sz, 4);
        char* buf = bufGetter(sz);
        _readBuf(buf, sz);
    }
    if (!strcmp("SUCCESS", lineBuf))
        return true;
    else if (!strcmp("EXCEPTION", lineBuf))
        throw std::runtime_error("blender script exception");

    return false;
}

void CBlenderConnection::quitBlender()
{
    _writeLine("QUIT");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
}
