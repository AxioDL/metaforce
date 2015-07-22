#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system_error>
#include <string>

#include <HECL/HECL.hpp>
#include <LogVisor/LogVisor.hpp>
#include "BlenderConnection.hpp"

static LogVisor::LogModule Log("BlenderConnection");

#ifdef __APPLE__
#define DEFAULT_BLENDER_BIN "/Applications/Blender.app/Contents/MacOS/blender"
#elif _WIN32
#define DEFAULT_BLENDER_BIN _S("%ProgramFiles%\\Blender Foundation\\Blender\\blender.exe")
#else
#define DEFAULT_BLENDER_BIN "blender"
#endif

#define TEMP_SHELLSCRIPT "/home/jacko/hecl/blender/blendershell.py"

size_t BlenderConnection::_readLine(char* buf, size_t bufSz)
{
    size_t readBytes = 0;
    while (true)
    {
        if (readBytes >= bufSz)
        {
            throw std::length_error("Pipe buffer overrun");
            *(buf-1) = '\0';
            return bufSz - 1;
        }
#if _WIN32
        DWORD ret = 0;
        if (!ReadFile(m_readpipe[0], buf, 1, &ret, NULL))
            goto err;
#else
        ssize_t ret = read(m_readpipe[0], buf, 1);
        if (ret < 0)
            goto err;
#endif
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

size_t BlenderConnection::_writeLine(const char* buf)
{
#if _WIN32
    DWORD ret = 0;
    if (!WriteFile(m_writepipe[1], buf, strlen(buf), &ret, NULL))
        goto err;
    if (!WriteFile(m_writepipe[1], "\n", 1, NULL, NULL))
        goto err;
#else
    ssize_t ret, nlerr;
    ret = write(m_writepipe[1], buf, strlen(buf));
    if (ret < 0)
        goto err;
    nlerr = write(m_writepipe[1], "\n", 1);
    if (nlerr < 0)
        goto err;
#endif
    return (size_t)ret;
err:
    throw std::error_code(errno, std::system_category());
}

size_t BlenderConnection::_readBuf(char* buf, size_t len)
{
#if _WIN32
    DWORD ret = 0;
    if (!ReadFile(m_readpipe[0], buf, len, &ret, NULL))
        goto err;
#else
    ssize_t ret = read(m_readpipe[0], buf, len);
    if (ret < 0)
        goto err;
#endif
    return ret;
err:
    throw std::error_code(errno, std::system_category());
    return 0;
}

size_t BlenderConnection::_writeBuf(const char* buf, size_t len)
{
#if _WIN32
    DWORD ret = 0;
    if (!WriteFile(m_writepipe[1], buf, len, &ret, NULL))
        goto err;
#else
    ssize_t ret = write(m_writepipe[1], buf, len);
    if (ret < 0)
        goto err;
#endif
    return ret;
err:
    throw std::error_code(errno, std::system_category());
    return 0;
}

void BlenderConnection::_closePipe()
{
#if _WIN32
    CloseHandle(m_readpipe[0]);
    CloseHandle(m_writepipe[1]);
#else
    close(m_readpipe[0]);
    close(m_writepipe[1]);
#endif
}

BlenderConnection::BlenderConnection(bool silenceBlender)
{
    /* Construct communication pipes */
#if _WIN32
    SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    CreatePipe(&m_readpipe[0], &m_readpipe[1], &sattrs, 0);
    CreatePipe(&m_writepipe[0], &m_writepipe[1], &sattrs, 0);
#else
    pipe(m_readpipe);
    pipe(m_writepipe);
#endif

    /* User-specified blender path */
#if _WIN32
    wchar_t BLENDER_BIN_BUF[2048];
    wchar_t* blenderBin = _wgetenv(L"BLENDER_BIN");
#else
    char* blenderBin = getenv("BLENDER_BIN");
#endif

    /* Child process of blender */
#if _WIN32
    if (!blenderBin)
    {
        /* Environment not set; use registry */
        HKEY blenderKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\BlenderFoundation", 0, KEY_READ, &blenderKey) == ERROR_SUCCESS)
        {
            DWORD bufSz = sizeof(BLENDER_BIN_BUF);
            if (RegGetValueW(blenderKey, NULL, L"Install_Dir", REG_SZ, NULL, BLENDER_BIN_BUF, &bufSz) == ERROR_SUCCESS)
            {
                wcscat_s(BLENDER_BIN_BUF, 2048, L"\\blender.exe");
                blenderBin = BLENDER_BIN_BUF;
            }
            RegCloseKey(blenderKey);
        }
    }
    if (!blenderBin)
    {
        Log.report(LogVisor::FatalError, "unable to find blender");
        return;
    }

    wchar_t cmdLine[2048];
    _snwprintf(cmdLine, 2048, L" --background -P shellscript.py -- %08X %08X", 
               (uint32_t)m_writepipe[0], (uint32_t)m_readpipe[1]);

    STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
    HANDLE nulHandle = NULL;
    if (silenceBlender)
    {
        nulHandle = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &sattrs, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        sinfo.hStdError = nulHandle;
        sinfo.hStdOutput = nulHandle;
        sinfo.dwFlags = STARTF_USESTDHANDLES;
    }

    PROCESS_INFORMATION pinfo;
    if (!CreateProcessW(blenderBin, cmdLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sinfo, &pinfo))
        Log.report(LogVisor::FatalError, "unable to launch blender");

    CloseHandle(m_writepipe[1]);
    CloseHandle(m_readpipe[0]);

    if (nulHandle)
        CloseHandle(nulHandle);

#else
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
#endif

    /* Handle first response */
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "NOLAUNCH"))
    {
        _closePipe();
        Log.report(LogVisor::FatalError, "Unable to launch blender");
    }
    else if (!strcmp(lineBuf, "NOBLENDER"))
    {
        _closePipe();
        if (blenderBin)
            Log.report(LogVisor::FatalError, _S("Unable to find blender at '%s' or '%s'"),
                       blenderBin, DEFAULT_BLENDER_BIN);
        else
            Log.report(LogVisor::FatalError, _S("Unable to find blender at '%s'"),
                       DEFAULT_BLENDER_BIN);
    }
    else if (!strcmp(lineBuf, "NOADDON"))
    {
        _closePipe();
        Log.report(LogVisor::FatalError, "HECL addon not installed within blender");
    }
    else if (strcmp(lineBuf, "READY"))
    {
        _closePipe();
        Log.report(LogVisor::FatalError, "read '%s' from blender; expected 'READY'", lineBuf);
    }
    _writeLine("ACK");

}

BlenderConnection::~BlenderConnection()
{
    _closePipe();
}

bool BlenderConnection::openBlend(const std::string& path)
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

bool BlenderConnection::cookBlend(std::function<char*(uint32_t)> bufGetter,
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

void BlenderConnection::quitBlender()
{
    _writeLine("QUIT");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
}
