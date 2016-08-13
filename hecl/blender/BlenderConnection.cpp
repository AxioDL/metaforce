#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <system_error>
#include <string>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>

#include <hecl/hecl.hpp>
#include <hecl/Database.hpp>
#include "logvisor/logvisor.hpp"
#include "BlenderConnection.hpp"

#if _WIN32
#include <io.h>
#include <fcntl.h>
#endif

namespace std
{
template <> struct hash<std::pair<uint32_t,uint32_t>>
{
    size_t operator()(const std::pair<uint32_t,uint32_t>& val) const NOEXCEPT
    {
        /* this will potentially truncate the second value if 32-bit size_t,
         * however, its application here is intended to operate in 16-bit indices */
        return val.first | (val.second << 16);
    }
};
}

namespace hecl
{

logvisor::Module BlenderLog("hecl::BlenderConnection");
BlenderToken SharedBlenderToken;

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
    FILE* fp = hecl::Fopen(path, _S("w"));
    if (!fp)
        BlenderLog.report(logvisor::Fatal, _S("unable to open %s for writing"), path);
    fwrite(HECL_BLENDERSHELL, 1, HECL_BLENDERSHELL_SZ, fp);
    fclose(fp);
}

static void InstallAddon(const SystemChar* path)
{
    FILE* fp = hecl::Fopen(path, _S("wb"));
    if (!fp)
        BlenderLog.report(logvisor::Fatal, _S("Unable to install blender addon at '%s'"), path);
    fwrite(HECL_ADDON, 1, HECL_ADDON_SZ, fp);
    fclose(fp);
}

static void InstallStartup(const char* path)
{
    FILE* fp = fopen(path, "wb");
    if (!fp)
        BlenderLog.report(logvisor::Fatal, "Unable to place hecl_startup.blend at '%s'", path);
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
            BlenderLog.report(logvisor::Fatal, "Pipe buffer overrun");
            *(buf-1) = '\0';
            return bufSz - 1;
        }
        int ret;
        while ((ret = read(m_readpipe[0], buf, 1)) < 0 && errno == EINTR) {}
        if (ret < 0)
        {
            BlenderLog.report(logvisor::Fatal, strerror(errno));
            return 0;
        }
        else if (ret == 1)
        {
            if (*buf == '\n')
            {
                *buf = '\0';
                if (readBytes >= 4)
                    if (!memcmp(buf, "EXCEPTION", std::min(readBytes, size_t(9))))
                        _blenderDied();
                return readBytes;
            }
            ++readBytes;
            ++buf;
        }
        else
        {
            *buf = '\0';
            if (readBytes >= 4)
                if (!memcmp(buf, "EXCEPTION", std::min(readBytes, size_t(9))))
                    _blenderDied();
            return readBytes;
        }
    }
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
    _blenderDied();
    return 0;
}

size_t BlenderConnection::_readBuf(void* buf, size_t len)
{
    int ret = read(m_readpipe[0], buf, len);
    if (ret < 0)
        goto err;
    if (len >= 4)
        if (!memcmp((char*)buf, "EXCEPTION", std::min(len, size_t(9))))
            _blenderDied();
    return ret;
err:
    _blenderDied();
    return 0;
}

size_t BlenderConnection::_writeBuf(const void* buf, size_t len)
{
    int ret = write(m_writepipe[1], buf, len);
    if (ret < 0)
        goto err;
    return ret;
err:
    _blenderDied();
    return 0;
}

void BlenderConnection::_closePipe()
{
    close(m_readpipe[0]);
    close(m_writepipe[1]);
}

void BlenderConnection::_blenderDied()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    FILE* errFp = hecl::Fopen(m_errPath.c_str(), _S("r"));
    if (errFp)
    {
        fseek(errFp, 0, SEEK_END);
        int64_t len = hecl::FTell(errFp);
        if (len)
        {
            fseek(errFp, 0, SEEK_SET);
            std::unique_ptr<char[]> buf(new char[len+1]);
            memset(buf.get(), 0, len+1);
            fread(buf.get(), 1, len, errFp);
            BlenderLog.report(logvisor::Fatal, "\n%s", buf.get());
        }
    }
    BlenderLog.report(logvisor::Fatal, "Blender Exception");
}

BlenderConnection::BlenderConnection(int verbosityLevel)
{
    BlenderLog.report(logvisor::Info, "Establishing BlenderConnection...");

    /* Put hecl_blendershell.py in temp dir */
#ifdef _WIN32
    wchar_t* TMPDIR = _wgetenv(L"TEMP");
    if (!TMPDIR)
        TMPDIR = (wchar_t*)L"\\Temp";
    m_startupBlend = hecl::WideToUTF8(TMPDIR);
#else
    signal(SIGPIPE, SIG_IGN);
    char* TMPDIR = getenv("TMPDIR");
    if (!TMPDIR)
        TMPDIR = (char*)"/tmp";
    m_startupBlend = TMPDIR;
#endif
    hecl::SystemString blenderShellPath(TMPDIR);
    blenderShellPath += _S("/hecl_blendershell.py");
    InstallBlendershell(blenderShellPath.c_str());

    hecl::SystemString blenderAddonPath(TMPDIR);
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
                BlenderLog.report(logvisor::Fatal, L"unable to determine 'Program Files' path");
            _snwprintf(BLENDER_BIN_BUF, 2048, L"%s\\Blender Foundation\\Blender\\blender.exe", progFiles);
            blenderBin = BLENDER_BIN_BUF;
        }

        wchar_t cmdLine[2048];
        _snwprintf(cmdLine, 2048, L" --background -P \"%s\" -- %" PRIuPTR " %" PRIuPTR " %d \"%s\"",
                   blenderShellPath.c_str(), uintptr_t(writehandle), uintptr_t(readhandle),
                   verbosityLevel, blenderAddonPath.c_str());

        STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
        HANDLE nulHandle = NULL;
        if (verbosityLevel == 0)
        {
            SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
            nulHandle = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                                    &sattrs, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            sinfo.hStdInput = nulHandle;
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
            BlenderLog.report(logvisor::Fatal, L"unable to launch blender from %s: %s", blenderBin, messageBuffer);
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
            char vLevel[32];
            snprintf(vLevel, 32, "%d", verbosityLevel);

            /* Try user-specified blender first */
            if (blenderBin)
            {
                execlp(blenderBin, blenderBin,
                    "--background", "-P", blenderShellPath.c_str(),
                    "--", readfds, writefds, vLevel, blenderAddonPath.c_str(), NULL);
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
                "--", readfds, writefds, vLevel, blenderAddonPath.c_str(), NULL);
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

        /* Stash error path an unlink existing file */
        m_errPath = hecl::SystemString(TMPDIR) + hecl::SysFormat(_S("/hecl_%016llX.derp"), (unsigned long long)m_blenderProc);
        hecl::Unlink(m_errPath.c_str());

        /* Handle first response */
        char lineBuf[256];
        _readLine(lineBuf, sizeof(lineBuf));
        if (!strcmp(lineBuf, "NOLAUNCH"))
        {
            _closePipe();
            BlenderLog.report(logvisor::Fatal, "Unable to launch blender");
        }
        else if (!strcmp(lineBuf, "NOBLENDER"))
        {
            _closePipe();
            if (blenderBin)
                BlenderLog.report(logvisor::Fatal, _S("Unable to find blender at '%s' or '%s'"),
                           blenderBin, DEFAULT_BLENDER_BIN);
            else
                BlenderLog.report(logvisor::Fatal, _S("Unable to find blender at '%s'"),
                           DEFAULT_BLENDER_BIN);
        }
        else if (!strcmp(lineBuf, "NOADDON"))
        {
            _closePipe();
            InstallAddon(blenderAddonPath.c_str());
            ++installAttempt;
            if (installAttempt >= 2)
                BlenderLog.report(logvisor::Fatal, _S("unable to install blender addon using '%s'"), blenderAddonPath.c_str());
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
            BlenderLog.report(logvisor::Fatal, "read '%s' from blender; expected 'READY'", lineBuf);
        }
        _writeLine("ACK");

        _readLine(lineBuf, 7);
        if (!strcmp(lineBuf, "SLERP0"))
            m_hasSlerp = false;
        else if (!strcmp(lineBuf, "SLERP1"))
            m_hasSlerp = true;
        else
        {
            _closePipe();
            BlenderLog.report(logvisor::Fatal, "read '%s' from blender; expected 'SLERP(0|1)'", lineBuf);
        }

        break;
    }
}

BlenderConnection::~BlenderConnection()
{
    _closePipe();
}

std::streambuf::int_type
BlenderConnection::PyOutStream::StreamBuf::overflow(int_type ch)
{
    if (!m_parent.m_parent || !m_parent.m_parent->m_lock)
        BlenderLog.report(logvisor::Fatal, "lock not held for PyOutStream writing");
    if (ch != traits_type::eof() && ch != '\n' && ch != '\0')
    {
        m_lineBuf += char_type(ch);
        return ch;
    }
    //printf("FLUSHING %s\n", m_lineBuf.c_str());
    m_parent.m_parent->_writeLine(m_lineBuf.c_str());
    char readBuf[16];
    m_parent.m_parent->_readLine(readBuf, 16);
    if (strcmp(readBuf, "OK"))
    {
        if (m_deleteOnError)
            m_parent.m_parent->deleteBlend();
        m_parent.m_parent->_blenderDied();
    }
    m_lineBuf.clear();
    return ch;
}

static const char* BlendTypeStrs[] =
{
    "NONE",
    "MESH",
    "ACTOR",
    "AREA",
    "WORLD",
    "MAPAREA",
    "MAPUNIVERSE",
    "FRAME",
    nullptr
};

bool BlenderConnection::createBlend(const ProjectPath& path, BlendType type)
{
    if (m_lock)
    {
        BlenderLog.report(logvisor::Fatal,
                          "BlenderConnection::createBlend() musn't be called with stream active");
        return false;
    }
    _writeLine(("CREATE \"" + path.getAbsolutePathUTF8() + "\" " + BlendTypeStrs[int(type)] + " \"" + m_startupBlend + "\"").c_str());
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
    {
        /* Delete immediately in case save doesn't occur */
        hecl::Unlink(path.getAbsolutePath().c_str());
        m_loadedBlend = path;
        m_loadedType = type;
        return true;
    }
    return false;
}

bool BlenderConnection::openBlend(const ProjectPath& path, bool force)
{
    if (m_lock)
    {
        BlenderLog.report(logvisor::Fatal,
                          "BlenderConnection::openBlend() musn't be called with stream active");
        return false;
    }
    if (!force && path == m_loadedBlend)
        return true;
    _writeLine(("OPEN \"" + path.getAbsolutePathUTF8() + "\"").c_str());
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
    if (!strcmp(lineBuf, "FINISHED"))
    {
        m_loadedBlend = path;
        _writeLine("GETTYPE");
        _readLine(lineBuf, sizeof(lineBuf));
        m_loadedType = BlendType::None;
        unsigned idx = 0;
        while (BlendTypeStrs[idx])
        {
            if (!strcmp(BlendTypeStrs[idx], lineBuf))
            {
                m_loadedType = BlendType(idx);
                break;
            }
            ++idx;
        }
        m_loadedRigged = false;
        if (m_loadedType == BlendType::Mesh)
        {
            _writeLine("GETMESHRIGGED");
            _readLine(lineBuf, sizeof(lineBuf));
            if (!strcmp("TRUE", lineBuf))
                m_loadedRigged = true;
        }
        return true;
    }
    return false;
}

bool BlenderConnection::saveBlend()
{
    if (m_lock)
    {
        BlenderLog.report(logvisor::Fatal,
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
    if (m_loadedBlend)
    {
        hecl::Unlink(m_loadedBlend.getAbsolutePath().c_str());
        BlenderLog.report(logvisor::Info, _S("Deleted '%s'"), m_loadedBlend.getAbsolutePath().c_str());
        m_loadedBlend = ProjectPath();
    }
}

void BlenderConnection::PyOutStream::linkBlend(const char* target,
                                               const char* objName,
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
           objName, target, link?"True":"False",
           objName, objName, target, objName);
}

void BlenderConnection::PyOutStream::linkBackground(const char* target,
                                                    const char* sceneName)
{
    format("if '%s' not in bpy.data.scenes:\n"
           "    with bpy.data.libraries.load('''%s''', link=True, relative=True) as (data_from, data_to):\n"
           "        data_to.scenes = data_from.scenes\n"
           "    obj_scene = None\n"
           "    for scene in data_to.scenes:\n"
           "        if scene.name == '%s':\n"
           "            obj_scene = scene\n"
           "            break\n"
           "    if not obj_scene:\n"
           "        raise RuntimeError('''unable to find %s in %s. try deleting it and restart the extract.''')\n"
           "\n"
           "bpy.context.scene.background_set = bpy.data.scenes['%s']\n",
           sceneName, target,
           sceneName, sceneName, target, sceneName);
}

BlenderConnection::DataStream::Mesh::Mesh
(BlenderConnection& conn, HMDLTopology topologyIn, int skinSlotCount, SurfProgFunc& surfProg)
: topology(topologyIn), sceneXf(conn), aabbMin(conn), aabbMax(conn)
{
    uint32_t matSetCount;
    conn._readBuf(&matSetCount, 4);
    materialSets.reserve(matSetCount);
    for (uint32_t i=0 ; i<matSetCount ; ++i)
    {
        materialSets.emplace_back();
        std::vector<Material>& materials = materialSets.back();
        uint32_t matCount;
        conn._readBuf(&matCount, 4);
        materials.reserve(matCount);
        for (uint32_t i=0 ; i<matCount ; ++i)
            materials.emplace_back(conn);
    }

    uint32_t count;
    conn._readBuf(&count, 4);
    pos.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        pos.emplace_back(conn);

    conn._readBuf(&count, 4);
    norm.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        norm.emplace_back(conn);

    conn._readBuf(&colorLayerCount, 4);
    if (colorLayerCount > 4)
        LogModule.report(logvisor::Fatal, "mesh has %u color-layers; max 4", colorLayerCount);
    conn._readBuf(&count, 4);
    color.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        color.emplace_back(conn);

    conn._readBuf(&uvLayerCount, 4);
    if (uvLayerCount > 8)
        LogModule.report(logvisor::Fatal, "mesh has %u UV-layers; max 8", uvLayerCount);
    conn._readBuf(&count, 4);
    uv.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        uv.emplace_back(conn);

    conn._readBuf(&count, 4);
    boneNames.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
    {
        char name[128];
        conn._readLine(name, 128);
        boneNames.emplace_back(name);
    }

    conn._readBuf(&count, 4);
    skins.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
    {
        skins.emplace_back();
        std::vector<SkinBind>& binds = skins.back();
        uint32_t bindCount;
        conn._readBuf(&bindCount, 4);
        binds.reserve(bindCount);
        for (uint32_t j=0 ; j<bindCount ; ++j)
            binds.emplace_back(conn);
    }

    /* Assume 16 islands per material for reserve */
    if (materialSets.size())
        surfaces.reserve(materialSets.front().size() * 16);
    uint8_t isSurf;
    conn._readBuf(&isSurf, 1);
    int prog = 0;
    while (isSurf)
    {
        surfaces.emplace_back(conn, *this, skinSlotCount);
        surfProg(++prog);
        conn._readBuf(&isSurf, 1);
    }

    /* Custom properties */
    uint32_t propCount;
    conn._readBuf(&propCount, 4);
    std::string keyBuf;
    std::string valBuf;
    for (uint32_t i=0 ; i<propCount ; ++i)
    {
        uint32_t kLen;
        conn._readBuf(&kLen, 4);
        keyBuf.assign(kLen, '\0');
        conn._readBuf(&keyBuf[0], kLen);

        uint32_t vLen;
        conn._readBuf(&vLen, 4);
        valBuf.assign(vLen, '\0');
        conn._readBuf(&valBuf[0], vLen);

        customProps[keyBuf] = valBuf;
    }

    /* Connect skinned verts to bank slots */
    if (boneNames.size())
    {
        for (Surface& surf : surfaces)
        {
            SkinBanks::Bank& bank = skinBanks.banks[surf.skinBankIdx];
            for (Surface::Vert& vert : surf.verts)
            {
                for (uint32_t i=0 ; i<bank.m_skinIdxs.size() ; ++i)
                {
                    if (bank.m_skinIdxs[i] == vert.iSkin)
                    {
                        vert.iBankSkin = i;
                        break;
                    }
                }
            }
        }
    }
}

BlenderConnection::DataStream::Mesh
BlenderConnection::DataStream::Mesh::getContiguousSkinningVersion() const
{
    Mesh newMesh = *this;
    newMesh.pos.clear();
    newMesh.norm.clear();
    newMesh.contiguousSkinVertCounts.clear();
    newMesh.contiguousSkinVertCounts.reserve(skins.size());
    for (size_t i=0 ; i<skins.size() ; ++i)
    {
        std::unordered_map<std::pair<uint32_t,uint32_t>, uint32_t> contigMap;
        size_t vertCount = 0;
        for (Surface& surf : newMesh.surfaces)
        {
            for (Surface::Vert& vert : surf.verts)
            {
                if (vert.iSkin == i)
                {
                    auto key = std::make_pair(vert.iPos, vert.iNorm);
                    auto search = contigMap.find(key);
                    if (search != contigMap.end())
                    {
                        vert.iPos = search->second;
                        vert.iNorm = search->second;
                    }
                    else
                    {
                        uint32_t newIdx = newMesh.pos.size();
                        contigMap[key] = newIdx;
                        newMesh.pos.push_back(pos.at(vert.iPos));
                        newMesh.norm.push_back(norm.at(vert.iNorm));
                        vert.iPos = newIdx;
                        vert.iNorm = newIdx;
                        ++vertCount;
                    }
                }
            }
        }
        newMesh.contiguousSkinVertCounts.push_back(vertCount);
    }
    return newMesh;
}

BlenderConnection::DataStream::Mesh::Material::Material
(BlenderConnection& conn)
{
    uint32_t bufSz;
    conn._readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    conn._readBuf(&name[0], bufSz);

    conn._readBuf(&bufSz, 4);
    source.assign(bufSz, ' ');
    conn._readBuf(&source[0], bufSz);

    uint32_t texCount;
    conn._readBuf(&texCount, 4);
    texs.reserve(texCount);
    for (uint32_t i=0 ; i<texCount ; ++i)
    {
        conn._readBuf(&bufSz, 4);
        std::string readStr(bufSz, ' ');
        conn._readBuf(&readStr[0], bufSz);
        SystemStringView absolute(readStr);

        SystemString relative =
        conn.m_loadedBlend.getProject().getProjectRootPath().getProjectRelativeFromAbsolute(absolute);
        texs.emplace_back(conn.m_loadedBlend.getProject().getProjectWorkingPath(), relative);
    }

    uint32_t iPropCount;
    conn._readBuf(&iPropCount, 4);
    iprops.reserve(iPropCount);
    for (uint32_t i=0 ; i<iPropCount ; ++i)
    {
        conn._readBuf(&bufSz, 4);
        std::string readStr(bufSz, ' ');
        conn._readBuf(&readStr[0], bufSz);

        int32_t val;
        conn._readBuf(&val, 4);
        iprops[readStr] = val;
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
        skinBankIdx = parent.skinBanks.addSurface(parent, *this, skinSlotCount);
}

BlenderConnection::DataStream::Mesh::Surface::Vert::Vert
(BlenderConnection& conn, const Mesh& parent)
{
    conn._readBuf(&iPos, 4);
    conn._readBuf(&iNorm, 4);
    for (uint32_t i=0 ; i<parent.colorLayerCount ; ++i)
        conn._readBuf(&iColor[i], 4);
    for (uint32_t i=0 ; i<parent.uvLayerCount ; ++i)
        conn._readBuf(&iUv[i], 4);
    conn._readBuf(&iSkin, 4);
}

static bool VertInBank(const std::vector<uint32_t>& bank, uint32_t sIdx)
{
    for (uint32_t idx : bank)
        if (sIdx == idx)
            return true;
    return false;
}

uint32_t BlenderConnection::DataStream::Mesh::SkinBanks::addSurface
(const Mesh& mesh, const Surface& surf, int skinSlotCount)
{
    if (banks.empty())
        addSkinBank(skinSlotCount);
    std::vector<uint32_t> toAdd;
    if (skinSlotCount > 0)
        toAdd.reserve(skinSlotCount);
    std::vector<Bank>::iterator bankIt = banks.begin();
    for (;;)
    {
        bool done = true;
        for (; bankIt != banks.end() ; ++bankIt)
        {
            Bank& bank = *bankIt;
            done = true;
            for (const Surface::Vert& v : surf.verts)
            {
                if (!VertInBank(bank.m_skinIdxs, v.iSkin) && !VertInBank(toAdd, v.iSkin))
                {
                    toAdd.push_back(v.iSkin);
                    if (skinSlotCount > 0 && bank.m_skinIdxs.size() + toAdd.size() > skinSlotCount)
                    {
                        toAdd.clear();
                        done = false;
                        break;
                    }
                }
            }
            if (toAdd.size())
            {
                bank.addSkins(mesh, toAdd);
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

BlenderConnection::DataStream::ColMesh::ColMesh(BlenderConnection& conn)
: aabbMin(conn), aabbMax(conn)
{
    uint32_t matCount;
    conn._readBuf(&matCount, 4);
    materials.reserve(matCount);
    for (uint32_t i=0 ; i<matCount ; ++i)
        materials.emplace_back(conn);

    uint32_t count;
    conn._readBuf(&count, 4);
    verts.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        verts.emplace_back(conn);

    conn._readBuf(&count, 4);
    edges.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        edges.emplace_back(conn);

    conn._readBuf(&count, 4);
    trianges.reserve(count);
    for (uint32_t i=0 ; i<count ; ++i)
        trianges.emplace_back(conn);
}

BlenderConnection::DataStream::ColMesh::Material::Material(BlenderConnection& conn)
{
    uint32_t nameLen;
    conn._readBuf(&nameLen, 4);
    if (nameLen)
    {
        name.assign(nameLen, '\0');
        conn._readBuf(&name[0], nameLen);
    }
    conn._readBuf(&type, 5);
}

BlenderConnection::DataStream::ColMesh::Edge::Edge(BlenderConnection& conn)
{
    conn._readBuf(this, 9);
}

BlenderConnection::DataStream::ColMesh::Triangle::Triangle(BlenderConnection& conn)
{
    conn._readBuf(this, 16);
}

BlenderConnection::DataStream::Light::Light(BlenderConnection& conn)
: sceneXf(conn), color(conn)
{
    conn._readBuf(&layer, 29);
}

BlenderConnection::DataStream::Actor::Actor(BlenderConnection& conn)
{
    uint32_t armCount;
    conn._readBuf(&armCount, 4);
    armatures.reserve(armCount);
    for (uint32_t i=0 ; i<armCount ; ++i)
        armatures.emplace_back(conn);

    uint32_t subtypeCount;
    conn._readBuf(&subtypeCount, 4);
    subtypes.reserve(subtypeCount);
    for (uint32_t i=0 ; i<subtypeCount ; ++i)
        subtypes.emplace_back(conn);

    uint32_t actionCount;
    conn._readBuf(&actionCount, 4);
    actions.reserve(actionCount);
    for (uint32_t i=0 ; i<actionCount ; ++i)
        actions.emplace_back(conn);
}

BlenderConnection::DataStream::Actor::Armature::Armature(BlenderConnection& conn)
{
    uint32_t bufSz;
    conn._readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    conn._readBuf(&name[0], bufSz);

    uint32_t boneCount;
    conn._readBuf(&boneCount, 4);
    bones.reserve(boneCount);
    for (uint32_t i=0 ; i<boneCount ; ++i)
        bones.emplace_back(conn);
}

BlenderConnection::DataStream::Actor::Armature::Bone::Bone(BlenderConnection& conn)
{
    uint32_t bufSz;
    conn._readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    conn._readBuf(&name[0], bufSz);

    origin.read(conn);

    conn._readBuf(&parent, 4);

    uint32_t childCount;
    conn._readBuf(&childCount, 4);
    children.reserve(childCount);
    for (uint32_t i=0 ; i<childCount ; ++i)
    {
        children.emplace_back(0);
        conn._readBuf(&children.back(), 4);
    }
}

BlenderConnection::DataStream::Actor::Subtype::Subtype(BlenderConnection& conn)
{
    uint32_t bufSz;
    conn._readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    conn._readBuf(&name[0], bufSz);

    std::string meshPath;
    conn._readBuf(&bufSz, 4);
    if (bufSz)
    {
        meshPath.assign(bufSz, ' ');
        conn._readBuf(&meshPath[0], bufSz);
        SystemStringView meshPathAbs(meshPath);

        SystemString meshPathRel =
        conn.m_loadedBlend.getProject().getProjectRootPath().getProjectRelativeFromAbsolute(meshPathAbs);
        mesh.assign(conn.m_loadedBlend.getProject().getProjectWorkingPath(), meshPathRel);
    }

    conn._readBuf(&armature, 4);

    uint32_t overlayCount;
    conn._readBuf(&overlayCount, 4);
    overlayMeshes.reserve(overlayCount);
    for (uint32_t i=0 ; i<overlayCount ; ++i)
    {
        std::string overlayName;
        conn._readBuf(&bufSz, 4);
        overlayName.assign(bufSz, ' ');
        conn._readBuf(&overlayName[0], bufSz);

        std::string meshPath;
        conn._readBuf(&bufSz, 4);
        if (bufSz)
        {
            meshPath.assign(bufSz, ' ');
            conn._readBuf(&meshPath[0], bufSz);
            SystemStringView meshPathAbs(meshPath);

            SystemString meshPathRel =
            conn.m_loadedBlend.getProject().getProjectRootPath().getProjectRelativeFromAbsolute(meshPathAbs);
            overlayMeshes.emplace_back(std::move(overlayName),
            ProjectPath(conn.m_loadedBlend.getProject().getProjectWorkingPath(), meshPathRel));
        }
    }
}

BlenderConnection::DataStream::Actor::Action::Action(BlenderConnection& conn)
{
    uint32_t bufSz;
    conn._readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    conn._readBuf(&name[0], bufSz);

    conn._readBuf(&interval, 4);
    conn._readBuf(&additive, 1);

    uint32_t frameCount;
    conn._readBuf(&frameCount, 4);
    frames.reserve(frameCount);
    for (uint32_t i=0 ; i<frameCount ; ++i)
    {
        frames.emplace_back();
        conn._readBuf(&frames.back(), 4);
    }

    uint32_t chanCount;
    conn._readBuf(&chanCount, 4);
    channels.reserve(chanCount);
    for (uint32_t i=0 ; i<chanCount ; ++i)
        channels.emplace_back(conn);

    uint32_t aabbCount;
    conn._readBuf(&aabbCount, 4);
    subtypeAABBs.reserve(aabbCount);
    for (uint32_t i=0 ; i<aabbCount ; ++i)
    {
        subtypeAABBs.emplace_back();
        subtypeAABBs.back().first.read(conn);
        subtypeAABBs.back().second.read(conn);
    }
}

BlenderConnection::DataStream::Actor::Action::Channel::Channel(BlenderConnection& conn)
{
    uint32_t bufSz;
    conn._readBuf(&bufSz, 4);
    boneName.assign(bufSz, ' ');
    conn._readBuf(&boneName[0], bufSz);

    conn._readBuf(&attrMask, 4);

    uint32_t keyCount;
    conn._readBuf(&keyCount, 4);
    keys.reserve(keyCount);
    for (uint32_t i=0 ; i<keyCount ; ++i)
        keys.emplace_back(conn, attrMask);
}

BlenderConnection::DataStream::Actor::Action::Channel::Key::Key(BlenderConnection& conn, uint32_t attrMask)
{
    if (attrMask & 1)
        rotation.read(conn);

    if (attrMask & 2)
        position.read(conn);

    if (attrMask & 4)
        scale.read(conn);
}

BlenderConnection::DataStream::Mesh
BlenderConnection::DataStream::compileMesh(HMDLTopology topology,
                                           int skinSlotCount,
                                           Mesh::SurfProgFunc surfProg)
{
    if (m_parent->m_loadedType != BlendType::Mesh)
        BlenderLog.report(logvisor::Fatal, _S("%s is not a MESH blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    char req[128];
    snprintf(req, 128, "MESHCOMPILE %s %d",
             MeshOutputModeString(topology), skinSlotCount);
    m_parent->_writeLine(req);

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to cook mesh: %s", readBuf);

    return Mesh(*m_parent, topology, skinSlotCount, surfProg);
}

BlenderConnection::DataStream::Mesh
BlenderConnection::DataStream::compileMesh(const std::string& name,
                                           HMDLTopology topology,
                                           int skinSlotCount,
                                           Mesh::SurfProgFunc surfProg)
{
    if (m_parent->m_loadedType != BlendType::Area)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an AREA blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    char req[128];
    snprintf(req, 128, "MESHCOMPILENAME %s %s %d", name.c_str(),
             MeshOutputModeString(topology), skinSlotCount);
    m_parent->_writeLine(req);

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to cook mesh '%s': %s", name.c_str(), readBuf);

    return Mesh(*m_parent, topology, skinSlotCount, surfProg);
}

BlenderConnection::DataStream::ColMesh
BlenderConnection::DataStream::compileColMesh(const std::string& name)
{
    if (m_parent->m_loadedType != BlendType::Area)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an AREA blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    char req[128];
    snprintf(req, 128, "MESHCOMPILENAMECOLLISION %s", name.c_str());
    m_parent->_writeLine(req);

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to cook collision mesh '%s': %s", name.c_str(), readBuf);

    return ColMesh(*m_parent);
}

BlenderConnection::DataStream::Mesh
BlenderConnection::DataStream::compileAllMeshes(HMDLTopology topology,
                                                int skinSlotCount,
                                                float maxOctantLength,
                                                Mesh::SurfProgFunc surfProg)
{
    if (m_parent->m_loadedType != BlendType::Area)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an AREA blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    char req[128];
    snprintf(req, 128, "MESHCOMPILEALL %s %d %f",
             MeshOutputModeString(topology),
             skinSlotCount, maxOctantLength);
    m_parent->_writeLine(req);

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to cook all meshes: %s", readBuf);

    return Mesh(*m_parent, topology, skinSlotCount, surfProg);
}

std::vector<BlenderConnection::DataStream::Light> BlenderConnection::DataStream::compileLights()
{
    if (m_parent->m_loadedType != BlendType::Area)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an AREA blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    m_parent->_writeLine("LIGHTCOMPILEALL");

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to gather all lights: %s", readBuf);

    uint32_t lightCount;
    m_parent->_readBuf(&lightCount, 4);

    std::vector<BlenderConnection::DataStream::Light> ret;
    ret.reserve(lightCount);

    for (int i=0 ; i<lightCount ; ++i)
        ret.emplace_back(*m_parent);

    return ret;
}

BlenderConnection::DataStream::Actor BlenderConnection::DataStream::compileActor()
{
    if (m_parent->m_loadedType != BlendType::Actor)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an ACTOR blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    m_parent->_writeLine("ACTORCOMPILE");

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to compile actor: %s", readBuf);

    return Actor(*m_parent);
}

std::vector<std::string> BlenderConnection::DataStream::getArmatureNames()
{
    if (m_parent->m_loadedType != BlendType::Actor)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an ACTOR blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    m_parent->_writeLine("GETARMATURENAMES");

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to get armatures of actor: %s", readBuf);

    std::vector<std::string> ret;

    uint32_t armCount;
    m_parent->_readBuf(&armCount, 4);
    ret.reserve(armCount);
    for (uint32_t i=0 ; i<armCount ; ++i)
    {
        ret.emplace_back();
        std::string& name = ret.back();
        uint32_t bufSz;
        m_parent->_readBuf(&bufSz, 4);
        name.assign(bufSz, ' ');
        m_parent->_readBuf(&name[0], bufSz);
    }

    return ret;
}

std::vector<std::string> BlenderConnection::DataStream::getSubtypeNames()
{
    if (m_parent->m_loadedType != BlendType::Actor)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an ACTOR blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    m_parent->_writeLine("GETSUBTYPENAMES");

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to get subtypes of actor: %s", readBuf);

    std::vector<std::string> ret;

    uint32_t subCount;
    m_parent->_readBuf(&subCount, 4);
    ret.reserve(subCount);
    for (uint32_t i=0 ; i<subCount ; ++i)
    {
        ret.emplace_back();
        std::string& name = ret.back();
        uint32_t bufSz;
        m_parent->_readBuf(&bufSz, 4);
        name.assign(bufSz, ' ');
        m_parent->_readBuf(&name[0], bufSz);
    }

    return ret;
}

std::vector<std::string> BlenderConnection::DataStream::getActionNames()
{
    if (m_parent->m_loadedType != BlendType::Actor)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an ACTOR blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    m_parent->_writeLine("GETACTIONNAMES");

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to get actions of actor: %s", readBuf);

    std::vector<std::string> ret;

    uint32_t actCount;
    m_parent->_readBuf(&actCount, 4);
    ret.reserve(actCount);
    for (uint32_t i=0 ; i<actCount ; ++i)
    {
        ret.emplace_back();
        std::string& name = ret.back();
        uint32_t bufSz;
        m_parent->_readBuf(&bufSz, 4);
        name.assign(bufSz, ' ');
        m_parent->_readBuf(&name[0], bufSz);
    }

    return ret;
}

std::unordered_map<std::string, BlenderConnection::DataStream::Matrix3f>
BlenderConnection::DataStream::getBoneMatrices(const std::string& name)
{
    if (name.empty())
        return {};

    if (m_parent->m_loadedType != BlendType::Actor)
        BlenderLog.report(logvisor::Fatal, _S("%s is not an ACTOR blend"),
                          m_parent->m_loadedBlend.getAbsolutePath().c_str());

    char req[128];
    snprintf(req, 128, "GETBONEMATRICES %s", name.c_str());
    m_parent->_writeLine(req);

    char readBuf[256];
    m_parent->_readLine(readBuf, 256);
    if (strcmp(readBuf, "OK"))
        BlenderLog.report(logvisor::Fatal, "unable to get matrices of armature: %s", readBuf);

    std::unordered_map<std::string, Matrix3f> ret;

    uint32_t boneCount;
    m_parent->_readBuf(&boneCount, 4);
    ret.reserve(boneCount);
    for (uint32_t i=0 ; i<boneCount ; ++i)
    {
        std::string name;
        uint32_t bufSz;
        m_parent->_readBuf(&bufSz, 4);
        name.assign(bufSz, ' ');
        m_parent->_readBuf(&name[0], bufSz);

        Matrix3f matOut;
        for (int i=0 ; i<3 ; ++i)
        {
            for (int j=0 ; j<3 ; ++j)
            {
                float val;
                m_parent->_readBuf(&val, 4);
                matOut[i].vec[j] = val;
            }
            reinterpret_cast<atVec4f&>(matOut[i]).vec[3] = 0.f;
        }

        ret.emplace(std::make_pair(std::move(name), std::move(matOut)));
    }

    return ret;

}

void BlenderConnection::quitBlender()
{
    _writeLine("QUIT");
    char lineBuf[256];
    _readLine(lineBuf, sizeof(lineBuf));
}

BlenderConnection& BlenderConnection::SharedConnection()
{
    return SharedBlenderToken.getBlenderConnection();
}

void BlenderConnection::Shutdown()
{
    SharedBlenderToken.shutdown();
}

}
