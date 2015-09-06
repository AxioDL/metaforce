#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <system_error>
#include <string>

#include <HECL/HECL.hpp>
#include <LogVisor/LogVisor.hpp>
#include "BlenderConnection.hpp"

#if _WIN32
#include <io.h>
#include <fcntl.h>
#endif

namespace HECL
{

LogVisor::LogModule BlenderLog("BlenderConnection");
BlenderConnection* SharedBlenderConnection = nullptr;

#ifdef __APPLE__
#define DEFAULT_BLENDER_BIN "/Applications/Blender.app/Contents/MacOS/blender"
#else
#define DEFAULT_BLENDER_BIN "blender"
#endif

extern "C" uint8_t HECL_BLENDERSHELL[];
extern "C" size_t HECL_BLENDERSHELL_SZ;

extern "C" uint8_t HECL_ADDON[];
extern "C" size_t HECL_ADDON_SZ;
    
extern "C" uint8_t HECL_STARTUP[];
extern "C" size_t HECL_STARTUP_SZ;

static void InstallBlendershell(const SystemChar* path)
{
    FILE* fp = HECL::Fopen(path, _S("w"));
    if (!fp)
        BlenderLog.report(LogVisor::FatalError, _S("unable to open %s for writing"), path);
    fwrite(HECL_BLENDERSHELL, 1, HECL_BLENDERSHELL_SZ, fp);
    fclose(fp);
}
    
static void InstallAddon(const SystemChar* path)
{
    FILE* fp = HECL::Fopen(path, _S("wb"));
    if (!fp)
        BlenderLog.report(LogVisor::FatalError, _S("Unable to install blender addon at '%s'"), path);
    fwrite(HECL_ADDON, 1, HECL_ADDON_SZ, fp);
    fclose(fp);
}

static void InstallStartup(const char* path)
{
    FILE* fp = fopen(path, "wb");
    if (!fp)
        BlenderLog.report(LogVisor::FatalError, "Unable to place hecl_startup.blend at '%s'", path);
    fwrite(HECL_STARTUP, 1, HECL_STARTUP_SZ, fp);
    fclose(fp);
}
    
size_t BlenderConnection::_readLine(char* buf, size_t bufSz)
{
    size_t readBytes = 0;
    while (true)
    {
        if (readBytes >= bufSz)
        {
            BlenderLog.report(LogVisor::FatalError, "Pipe buffer overrun");
            *(buf-1) = '\0';
            return bufSz - 1;
        }
        int ret = read(m_readpipe[0], buf, 1);
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
    BlenderLog.report(LogVisor::FatalError, strerror(errno));
    return 0;
}

size_t BlenderConnection::_writeLine(const char* buf)
{
    int ret, nlerr;
    ret = write(m_writepipe[1], buf, strlen(buf));
    if (ret < 0)
        goto err;
    nlerr = write(m_writepipe[1], "\n", 1);
    if (nlerr < 0)
        goto err;
    return (size_t)ret;
err:
    BlenderLog.report(LogVisor::FatalError, strerror(errno));
    return 0;
}

size_t BlenderConnection::_readBuf(char* buf, size_t len)
{
    int ret = read(m_readpipe[0], buf, len);
    if (ret < 0)
        goto err;
    return ret;
err:
    BlenderLog.report(LogVisor::FatalError, strerror(errno));
    return 0;
}

size_t BlenderConnection::_writeBuf(const char* buf, size_t len)
{
    int ret = write(m_writepipe[1], buf, len);
    if (ret < 0)
        goto err;
    return ret;
err:
    BlenderLog.report(LogVisor::FatalError, strerror(errno));
    return 0;
}

void BlenderConnection::_closePipe()
{
    close(m_readpipe[0]);
    close(m_writepipe[1]);
}

BlenderConnection::BlenderConnection(bool silenceBlender)
{
    BlenderLog.report(LogVisor::Info, "Establishing BlenderConnection...");
    
    /* Put hecl_blendershell.py in temp dir */
#ifdef _WIN32
    wchar_t* TMPDIR = _wgetenv(L"TEMP");
    if (!TMPDIR)
        TMPDIR = (wchar_t*)L"\\Temp";
#else
    char* TMPDIR = getenv("TMPDIR");
    if (!TMPDIR)
        TMPDIR = (char*)"/tmp";
#endif
    HECL::SystemString blenderShellPath(TMPDIR);
    blenderShellPath += _S("/hecl_blendershell.py");
    InstallBlendershell(blenderShellPath.c_str());

    HECL::SystemString blenderAddonPath(TMPDIR);
    blenderAddonPath += _S("/hecl_blenderaddon.zip");
    InstallAddon(blenderAddonPath.c_str());
    
    HECL::SystemString blenderStartupPath(TMPDIR);
    m_startupBlend = TMPDIR;
    m_startupBlend += _S("/hecl_startup.blend");
    InstallStartup(m_startupBlend.c_str());

    int installAttempt = 0;
    while (true)
    {

        /* Construct communication pipes */
#if _WIN32
        _pipe(m_readpipe, 2048, _O_BINARY);
        _pipe(m_writepipe, 2048, _O_BINARY);
        HANDLE writehandle = HANDLE(_get_osfhandle(m_writepipe[0]));
        SetHandleInformation(writehandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        HANDLE readhandle = HANDLE(_get_osfhandle(m_readpipe[1]));
        SetHandleInformation(readhandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
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
            /* Environment not set; use default */
            wchar_t progFiles[256];
            if (!GetEnvironmentVariableW(L"ProgramFiles", progFiles, 256))
                BlenderLog.report(LogVisor::FatalError, L"unable to determine 'Program Files' path");
            _snwprintf(BLENDER_BIN_BUF, 2048, L"%s\\Blender Foundation\\Blender\\blender.exe", progFiles);
            blenderBin = BLENDER_BIN_BUF;
        }

        wchar_t cmdLine[2048];
        _snwprintf(cmdLine, 2048, L" --background -P \"%s\" -- %" PRIuPTR " %" PRIuPTR " \"%s\"",
                    blenderShellPath.c_str(), uintptr_t(writehandle), uintptr_t(readhandle), blenderAddonPath.c_str());

        STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
        HANDLE nulHandle = NULL;
        if (silenceBlender)
        {
            SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
            nulHandle = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &sattrs, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            sinfo.hStdError = nulHandle;
            sinfo.hStdOutput = nulHandle;
            sinfo.dwFlags = STARTF_USESTDHANDLES;
        }

        PROCESS_INFORMATION pinfo;
        if (!CreateProcessW(blenderBin, cmdLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sinfo, &pinfo))
        {
            LPWSTR messageBuffer = nullptr;
            size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
            BlenderLog.report(LogVisor::FatalError, L"unable to launch blender from %s: %s", blenderBin, messageBuffer);
        }

        close(m_writepipe[0]);
        close(m_readpipe[1]);

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
                int devNull = open("/dev/null", O_WRONLY);
                dup2(devNull, STDOUT_FILENO);
                dup2(devNull, STDERR_FILENO);
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
                    "--background", "-P", blenderShellPath.c_str(),
                    "--", readfds, writefds, blenderAddonPath.c_str(), NULL);
                if (errno != ENOENT)
                {
                    snprintf(errbuf, 256, "NOLAUNCH %s\n", strerror(errno));
                    write(m_writepipe[1], errbuf, strlen(errbuf));
                    exit(1);
                }
            }

            /* Otherwise default blender */
            execlp(DEFAULT_BLENDER_BIN, DEFAULT_BLENDER_BIN,
                "--background", "-P", blenderShellPath.c_str(),
                "--", readfds, writefds, blenderAddonPath.c_str(), NULL);
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
            BlenderLog.report(LogVisor::FatalError, "Unable to launch blender");
        }
        else if (!strcmp(lineBuf, "NOBLENDER"))
        {
            _closePipe();
            if (blenderBin)
                BlenderLog.report(LogVisor::FatalError, _S("Unable to find blender at '%s' or '%s'"),
                           blenderBin, DEFAULT_BLENDER_BIN);
            else
                BlenderLog.report(LogVisor::FatalError, _S("Unable to find blender at '%s'"),
                           DEFAULT_BLENDER_BIN);
        }
        else if (!strcmp(lineBuf, "NOADDON"))
        {
            _closePipe();
            InstallAddon(blenderAddonPath.c_str());
            ++installAttempt;
            if (installAttempt >= 2)
                BlenderLog.report(LogVisor::FatalError, "unable to install blender addon using '%s'", blenderAddonPath.c_str());
            continue;
        }
        else if (strcmp(lineBuf, "READY"))
        {
            _closePipe();
            BlenderLog.report(LogVisor::FatalError, "read '%s' from blender; expected 'READY'", lineBuf);
        }
        _writeLine("ACK");

        break;
    }
}

BlenderConnection::~BlenderConnection()
{
    _closePipe();
}

bool BlenderConnection::createBlend(const SystemString& path)
{
    if (m_lock)
    {
        BlenderLog.report(LogVisor::FatalError,
                          "BlenderConnection::createBlend() musn't be called with stream active");
        return false;
    }
    HECL::SystemUTF8View pathView(path);
    _writeLine(("CREATE \"" + pathView.str() + "\" \"" + m_startupBlend + "\"").c_str());
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
    {
        m_loadedBlend = path;
        return true;
    }
    return false;
}

bool BlenderConnection::openBlend(const SystemString& path)
{
    if (m_lock)
    {
        BlenderLog.report(LogVisor::FatalError,
                          "BlenderConnection::openBlend() musn't be called with stream active");
        return false;
    }
    HECL::SystemUTF8View pathView(path);
    _writeLine(("OPEN \"" + pathView.str() + "\"").c_str());
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
    {
        m_loadedBlend = path;
        return true;
    }
    return false;
}

bool BlenderConnection::saveBlend()
{
    if (m_lock)
    {
        BlenderLog.report(LogVisor::FatalError,
                          "BlenderConnection::saveBlend() musn't be called with stream active");
        return false;
    }
    _writeLine("SAVE");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
        return true;
    return false;
}

void BlenderConnection::deleteBlend()
{
    if (m_loadedBlend.size())
    {
        HECL::Unlink(m_loadedBlend.c_str());
        BlenderLog.report(LogVisor::Info, _S("Deleted '%s'"), m_loadedBlend.c_str());
        m_loadedBlend.clear();
    }
}

void BlenderConnection::PyOutStream::linkBlend(const std::string& target, const std::string& objName,
                                               bool link)
{
    format("if '%s' not in bpy.data.scenes:\n"
           "    with bpy.data.libraries.load('''%s''', link=%s, relative=True) as (data_from, data_to):\n"
           "        data_to.scenes = data_from.scenes\n"
           "    obj_scene = None\n"
           "    for scene in data_to.scenes:\n"
           "        if scene.name == '%s':\n"
           "            obj_scene = scene\n"
           "            break\n"
           "    if not obj_scene:\n"
           "        raise RuntimeError('''unable to find %s in %s. try deleting it and restart the extract.''')\n"
           "    obj = None\n"
           "    for object in obj_scene.objects:\n"
           "        if object.name == obj_scene.name:\n"
           "            obj = object\n"
           "else:\n"
           "    obj = bpy.data.objects['%s']\n"
           "\n",
           objName.c_str(), target.c_str(), link?"True":"False",
           objName.c_str(), objName.c_str(), target.c_str(), objName.c_str());
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
        BlenderLog.report(LogVisor::Error, "expected '%s' to contain '%s' not '%s'",
                   m_loadedBlend.c_str(), expectedType.c_str(), lineBuf);
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
        BlenderLog.report(LogVisor::FatalError, "blender script exception");

    return false;
}

void BlenderConnection::quitBlender()
{
    _writeLine("QUIT");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
}

}
