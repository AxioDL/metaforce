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

size_t BlenderConnection::_readBuf(void* buf, size_t len)
{
    int ret = read(m_readpipe[0], buf, len);
    if (ret < 0)
        goto err;
    return ret;
err:
    BlenderLog.report(LogVisor::FatalError, strerror(errno));
    return 0;
}

size_t BlenderConnection::_writeBuf(const void* buf, size_t len)
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

BlenderConnection::BlenderConnection(int verbosityLevel)
{
    BlenderLog.report(LogVisor::Info, "Establishing BlenderConnection...");
    
    /* Put hecl_blendershell.py in temp dir */
#ifdef _WIN32
    wchar_t* TMPDIR = _wgetenv(L"TEMP");
    if (!TMPDIR)
        TMPDIR = (wchar_t*)L"\\Temp";
    m_startupBlend = HECL::WideToUTF8(TMPDIR);
#else
    char* TMPDIR = getenv("TMPDIR");
    if (!TMPDIR)
        TMPDIR = (char*)"/tmp";
    m_startupBlend = TMPDIR;
#endif
    HECL::SystemString blenderShellPath(TMPDIR);
    blenderShellPath += _S("/hecl_blendershell.py");
    InstallBlendershell(blenderShellPath.c_str());

    HECL::SystemString blenderAddonPath(TMPDIR);
    blenderAddonPath += _S("/hecl_blenderaddon.zip");
    InstallAddon(blenderAddonPath.c_str());
    
    m_startupBlend += "/hecl_startup.blend";
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
        _snwprintf(cmdLine, 2048, L" --background -P \"%s\" -- %" PRIuPTR " %" PRIuPTR " %d \"%s\"",
                   blenderShellPath.c_str(), uintptr_t(writehandle), uintptr_t(readhandle),
                   verbosityLevel > 1 ? 1 : 0, blenderAddonPath.c_str());

        STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
        HANDLE nulHandle = NULL;
        if (verbosityLevel == 0)
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

            if (verbosityLevel == 0)
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
            char dverbose[32];
            snprintf(dverbose, 32, "%d", verbosityLevel > 1 ? 1 : 0);

            /* Try user-specified blender first */
            if (blenderBin)
            {
                execlp(blenderBin, blenderBin,
                    "--background", "-P", blenderShellPath.c_str(),
                    "--", readfds, writefds, dverbose, blenderAddonPath.c_str(), NULL);
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
                "--", readfds, writefds, dverbose, blenderAddonPath.c_str(), NULL);
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
        else if (!strcmp(lineBuf, "ADDONINSTALLED"))
        {
            _closePipe();
            blenderAddonPath = _S("SKIPINSTALL");
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

static const char* BlendTypeStrs[] =
{
    "NONE",
    "MESH",
    "ACTOR",
    "AREA",
    nullptr
};

bool BlenderConnection::createBlend(const SystemString& path, BlendType type)
{
    if (m_lock)
    {
        BlenderLog.report(LogVisor::FatalError,
                          "BlenderConnection::createBlend() musn't be called with stream active");
        return false;
    }
    HECL::SystemUTF8View pathView(path);
    _writeLine(("CREATE \"" + pathView.str() + "\" " + BlendTypeStrs[type] + " \"" + m_startupBlend + "\"").c_str());
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
    {
        m_loadedBlend = path;
        return true;
    }
    return false;
}

BlenderConnection::BlendType BlenderConnection::getBlendType()
{
    _writeLine("GETTYPE");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    unsigned idx = 0;
    while (BlendTypeStrs[idx])
    {
        if (!strcmp(BlendTypeStrs[idx], lineBuf))
            return BlendType(idx);
        ++idx;
    }
    return TypeNone;
}

bool BlenderConnection::openBlend(const SystemString& path)
{
    if (m_lock)
    {
        BlenderLog.report(LogVisor::FatalError,
                          "BlenderConnection::openBlend() musn't be called with stream active");
        return false;
    }
    if (path == m_loadedBlend)
        return true;
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

void BlenderConnection::PyOutStream::linkBlend(const std::string& target,
                                               const std::string& objName,
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

BlenderConnection::DataStream::Mesh::Mesh(BlenderConnection& conn, int skinSlotCount)
{
    uint32_t matSetCount;
    conn._readBuf(&matSetCount, 4);
    materialSets.reserve(matSetCount);
    for (int i=0 ; i<matSetCount ; ++i)
    {
        materialSets.emplace_back();
        std::vector<Material>& materials = materialSets.back();
        uint32_t matCount;
        conn._readBuf(&matCount, 4);
        materials.reserve(matCount);
        for (int i=0 ; i<matCount ; ++i)
            materials.emplace_back(conn);
    }

    uint32_t count;
    conn._readBuf(&count, 4);
    pos.reserve(count);
    for (int i=0 ; i<count ; ++i)
        pos.emplace_back(conn);

    conn._readBuf(&count, 4);
    norm.reserve(count);
    for (int i=0 ; i<count ; ++i)
        norm.emplace_back(conn);

    conn._readBuf(&colorLayerCount, 4);
    conn._readBuf(&count, 4);
    color.reserve(count);
    for (int i=0 ; i<count ; ++i)
        color.emplace_back(conn);

    conn._readBuf(&uvLayerCount, 4);
    conn._readBuf(&count, 4);
    uv.reserve(count);
    for (int i=0 ; i<count ; ++i)
        uv.emplace_back(conn);

    conn._readBuf(&count, 4);
    boneNames.reserve(count);
    for (int i=0 ; i<count ; ++i)
    {
        char name[128];
        conn._readLine(name, 128);
        boneNames.emplace_back(name);
    }

    conn._readBuf(&count, 4);
    skins.reserve(count);
    for (int i=0 ; i<count ; ++i)
    {
        skins.emplace_back();
        std::vector<SkinBind>& binds = skins.back();
        uint32_t bindCount;
        conn._readBuf(&bindCount, 4);
        binds.reserve(bindCount);
        for (int j=0 ; j<bindCount ; ++j)
            binds.emplace_back(conn);
    }

    /* Assume 16 islands per material for reserve */
    if (materialSets.size())
        surfaces.reserve(materialSets.front().size() * 16);
    uint8_t isSurf;
    conn._readBuf(&isSurf, 1);
    while (isSurf)
    {
        surfaces.emplace_back(conn, *this, skinSlotCount);
        conn._readBuf(&isSurf, 1);
    }

    /* Connect skinned verts to bank slots */
    if (boneNames.size())
    {
        for (Surface& surf : surfaces)
        {
            std::vector<uint32_t>& bank = skinBanks.banks[surf.skinBankIdx];
            for (Surface::Vert& vert : surf.verts)
            {
                for (uint32_t i=0 ; i<bank.size() ; ++i)
                {
                    if (bank[i] == vert.iSkin)
                    {
                        vert.iBankSkin = i;
                        break;
                    }
                }
            }
        }
    }
}

BlenderConnection::DataStream::Mesh::Material::Material
(BlenderConnection& conn)
{
    char buf[4096];
    conn._readLine(buf, 4096);
    source.assign(buf);

    uint32_t texCount;
    conn._readBuf(&texCount, 4);
    texs.reserve(texCount);
    for (int i=0 ; i<texCount ; ++i)
    {
        conn._readLine(buf, 4096);
        texs.emplace_back(buf);
    }
}

BlenderConnection::DataStream::Mesh::Surface::Surface
(BlenderConnection& conn, Mesh& parent, int skinSlotCount)
: centroid(conn), materialIdx(conn), aabbMin(conn), aabbMax(conn),
  reflectionNormal(conn)
{
    uint32_t countEstimate;
    conn._readBuf(&countEstimate, 4);
    verts.reserve(countEstimate);

    uint8_t isVert;
    conn._readBuf(&isVert, 1);
    while (isVert)
    {
        verts.emplace_back(conn, parent);
        conn._readBuf(&isVert, 1);
    }

    if (parent.boneNames.size())
        skinBankIdx = parent.skinBanks.addSurface(*this, skinSlotCount);
}

BlenderConnection::DataStream::Mesh::Surface::Vert::Vert
(BlenderConnection& conn, const Mesh& parent)
{
    conn._readBuf(&iPos, 4);
    conn._readBuf(&iNorm, 4);
    if (parent.colorLayerCount)
        conn._readBuf(iColor, 4 * parent.colorLayerCount);
    if (parent.uvLayerCount)
        conn._readBuf(iUv, 4 * parent.uvLayerCount);
    conn._readBuf(&iSkin, 4);
    if (parent.pos.size() == 1250)
        printf("");
}

static bool VertInBank(const std::vector<uint32_t>& bank, uint32_t sIdx)
{
    for (uint32_t idx : bank)
        if (sIdx == idx)
            return true;
    return false;
}

uint32_t BlenderConnection::DataStream::Mesh::SkinBanks::addSurface
(const Surface& surf, int skinSlotCount)
{
    if (banks.empty())
        addSkinBank(skinSlotCount);
    std::vector<uint32_t> toAdd;
    toAdd.reserve(skinSlotCount);
    std::vector<std::vector<uint32_t>>::iterator bankIt = banks.begin();
    for (;;)
    {
        bool done = true;
        for (; bankIt != banks.end() ; ++bankIt)
        {
            std::vector<uint32_t>& bank = *bankIt;
            done = true;
            for (const Surface::Vert& v : surf.verts)
            {
                if (!VertInBank(bank, v.iSkin) && !VertInBank(toAdd, v.iSkin))
                {
                    toAdd.push_back(v.iSkin);
                    if (bank.size() + toAdd.size() > skinSlotCount)
                    {
                        toAdd.clear();
                        done = false;
                        break;
                    }
                }
            }
            if (toAdd.size())
            {
                for (uint32_t a : toAdd)
                    bank.push_back(a);
                toAdd.clear();
            }
            if (done)
                return uint32_t(bankIt - banks.begin());
        }
        if (!done)
        {
            bankIt = addSkinBank(skinSlotCount);
            continue;
        }
        break;
    }
    return uint32_t(-1);
}

void BlenderConnection::quitBlender()
{
    _writeLine("QUIT");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
}

}
