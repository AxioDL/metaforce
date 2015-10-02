#ifndef BLENDERCONNECTION_HPP
#define BLENDERCONNECTION_HPP

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <functional>

#include "HECL/HECL.hpp"

namespace HECL
{

extern LogVisor::LogModule BlenderLog;
extern class BlenderConnection* SharedBlenderConnection;

class BlenderConnection
{
    bool m_lock = false;
#if _WIN32
    HANDLE m_blenderProc;
#else
    pid_t m_blenderProc;
#endif
    int m_readpipe[2];
    int m_writepipe[2];
    SystemString m_loadedBlend;
    std::string m_startupBlend;
    size_t _readLine(char* buf, size_t bufSz);
    size_t _writeLine(const char* buf);
    size_t _readBuf(void* buf, size_t len);
    size_t _writeBuf(const void* buf, size_t len);
    void _closePipe();
public:
    BlenderConnection(int verbosityLevel=1);
    ~BlenderConnection();

    enum BlendType
    {
        TypeNone,
        TypeMesh,
        TypeActor,
        TypeArea
    };

    bool createBlend(const SystemString& path, BlendType type);
    BlendType getBlendType();
    bool openBlend(const SystemString& path);
    bool saveBlend();
    void deleteBlend();

    class PyOutStream : public std::ostream
    {
        friend class BlenderConnection;
        BlenderConnection* m_parent;
        bool m_deleteOnError;
        struct StreamBuf : std::streambuf
        {
            PyOutStream& m_parent;
            std::string m_lineBuf;
            bool m_deleteOnError;
            StreamBuf(PyOutStream& parent, bool deleteOnError)
            : m_parent(parent), m_deleteOnError(deleteOnError) {}
            StreamBuf(const StreamBuf& other) = delete;
            StreamBuf(StreamBuf&& other) = default;
            int_type overflow(int_type ch)
            {
                if (!m_parent.m_parent || !m_parent.m_parent->m_lock)
                    BlenderLog.report(LogVisor::FatalError, "lock not held for PyOutStream writing");
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
                    BlenderLog.report(LogVisor::FatalError, "error sending '%s' to blender", m_lineBuf.c_str());
                }
                m_lineBuf.clear();
                return ch;
            }
        } m_sbuf;
        PyOutStream(BlenderConnection* parent, bool deleteOnError)
        : std::ostream(&m_sbuf),
          m_parent(parent),
          m_deleteOnError(deleteOnError),
          m_sbuf(*this, deleteOnError)
        {
            m_parent->m_lock = true;
            m_parent->_writeLine("PYBEGIN");
            char readBuf[16];
            m_parent->_readLine(readBuf, 16);
            if (strcmp(readBuf, "READY"))
                BlenderLog.report(LogVisor::FatalError, "unable to open PyOutStream with blender");
        }
    public:
        PyOutStream(const PyOutStream& other) = delete;
        PyOutStream(PyOutStream&& other)
        : std::ostream(&m_sbuf), m_parent(other.m_parent), m_sbuf(std::move(other.m_sbuf))
        {other.m_parent = nullptr;}
        ~PyOutStream() {close();}
        void close()
        {
            if (m_parent && m_parent->m_lock)
            {
                m_parent->_writeLine("PYEND");
                char readBuf[16];
                m_parent->_readLine(readBuf, 16);
                if (strcmp(readBuf, "DONE"))
                    BlenderLog.report(LogVisor::FatalError, "unable to close PyOutStream with blender");
                m_parent->m_lock = false;
            }
        }
#if __GNUC__
        __attribute__((__format__ (__printf__, 2, 3)))
#endif
        void format(const char* fmt, ...)
        {
            if (!m_parent || !m_parent->m_lock)
                BlenderLog.report(LogVisor::FatalError, "lock not held for PyOutStream::format()");
            va_list ap;
            va_start(ap, fmt);
            char* result = nullptr;
#ifdef _WIN32
            int length = _vscprintf(fmt, ap);
            result = (char*)malloc(length);
            vsnprintf(result, length, fmt, ap);
#else
            int length = vasprintf(&result, fmt, ap);
#endif
            va_end(ap);
            if (length > 0)
                this->write(result, length);
            free(result);
        }
        void linkBlend(const std::string& target, const std::string& objName, bool link=true);

        class ANIMOutStream
        {
            BlenderConnection* m_parent;
            unsigned m_curCount = 0;
            unsigned m_totalCount = 0;
            bool m_inCurve = false;
        public:
            enum CurveType
            {
                CurveRotate,
                CurveTranslate,
                CurveScale
            };
            ANIMOutStream(BlenderConnection* parent)
            : m_parent(parent)
            {
                m_parent->_writeLine("PYANIM");
                char readBuf[16];
                m_parent->_readLine(readBuf, 16);
                if (strcmp(readBuf, "ANIMREADY"))
                    BlenderLog.report(LogVisor::FatalError, "unable to open ANIMOutStream");
            }
            ~ANIMOutStream()
            {
                char tp = -1;
                m_parent->_writeBuf(&tp, 1);
                char readBuf[16];
                m_parent->_readLine(readBuf, 16);
                if (strcmp(readBuf, "ANIMDONE"))
                    BlenderLog.report(LogVisor::FatalError, "unable to close ANIMOutStream");
            }
            void changeCurve(CurveType type, unsigned crvIdx, unsigned keyCount)
            {
                if (m_curCount != m_totalCount)
                    BlenderLog.report(LogVisor::FatalError, "incomplete ANIMOutStream for change");
                m_curCount = 0;
                m_totalCount = keyCount;
                char tp = char(type);
                m_parent->_writeBuf(&tp, 1);
                struct
                {
                    uint32_t ci;
                    uint32_t kc;
                } info = {uint32_t(crvIdx), uint32_t(keyCount)};
                m_parent->_writeBuf(reinterpret_cast<const char*>(&info), 8);
                m_inCurve = true;
            }
            void write(unsigned frame, float val)
            {
                if (!m_inCurve)
                    BlenderLog.report(LogVisor::FatalError, "changeCurve not called before write");
                if (m_curCount < m_totalCount)
                {
                    struct
                    {
                        uint32_t frm;
                        float val;
                    } key = {uint32_t(frame), val};
                    m_parent->_writeBuf(reinterpret_cast<const char*>(&key), 8);
                    ++m_curCount;
                }
                else
                    BlenderLog.report(LogVisor::FatalError, "ANIMOutStream keyCount overflow");
            }
        };
        ANIMOutStream beginANIMCurve()
        {
            return ANIMOutStream(m_parent);
        }
    };
    PyOutStream beginPythonOut(bool deleteOnError=false)
    {
        if (m_lock)
            BlenderLog.report(LogVisor::FatalError, "lock already held for BlenderConnection::beginPythonOut()");
        return PyOutStream(this, deleteOnError);
    }

    class DataStream
    {
        friend class BlenderConnection;
        BlenderConnection* m_parent;
        DataStream(BlenderConnection* parent)
        : m_parent(parent)
        {
            m_parent->m_lock = true;
            m_parent->_writeLine("DATABEGIN");
            char readBuf[16];
            m_parent->_readLine(readBuf, 16);
            if (strcmp(readBuf, "READY"))
                BlenderLog.report(LogVisor::FatalError, "unable to open DataStream with blender");
        }
    public:
        DataStream(const DataStream& other) = delete;
        DataStream(DataStream&& other)
        : m_parent(other.m_parent) {other.m_parent = nullptr;}
        ~DataStream() {close();}
        void close()
        {
            if (m_parent && m_parent->m_lock)
            {
                m_parent->_writeLine("DATAEND");
                char readBuf[16];
                m_parent->_readLine(readBuf, 16);
                if (strcmp(readBuf, "DONE"))
                    BlenderLog.report(LogVisor::FatalError, "unable to close DataStream with blender");
                m_parent->m_lock = false;
            }
        }

        std::vector<std::string> getMeshList()
        {
            m_parent->_writeLine("MESHLIST");
            uint32_t count;
            m_parent->_readBuf(&count, 4);
            std::vector<std::string> retval;
            retval.reserve(count);
            for (int i=0 ; i<count ; ++i)
            {
                char name[128];
                m_parent->_readLine(name, 128);
                retval.push_back(name);
            }
            return retval;
        }

        /* Intermediate mesh representation prepared by blender from a single mesh object */
        struct Mesh
        {
            /* HECL source of each material */
            std::vector<std::string> materials;

            /* Encapsulates mesh data up to maximum indexing space,
             * overflowing to additional Submeshes as needed */
            struct Submesh
            {
                /* Vertex buffer data */
                struct Vector2f
                {
                    float val[2];
                    Vector2f(BlenderConnection& conn) {conn._readBuf(val, 8);}
                };
                struct Vector3f
                {
                    float val[3];
                    Vector3f(BlenderConnection& conn) {conn._readBuf(val, 12);}
                };
                struct Vector4f
                {
                    float val[4];
                    Vector4f(BlenderConnection& conn) {conn._readBuf(val, 16);}
                };
                struct Index
                {
                    uint32_t val;
                    Index(BlenderConnection& conn) {conn._readBuf(&val, 4);}
                };
                std::vector<Vector3f> pos;
                std::vector<Vector3f> norm;
                uint32_t colorLayerCount = 0;
                std::vector<Vector4f> color[4];
                uint32_t uvLayerCount = 0;
                std::vector<Vector2f> uv[8];

                /* Skinning data */
                std::vector<std::string> boneNames;
                struct SkinBind
                {
                    uint32_t boneIdx;
                    float weight;
                    SkinBind(BlenderConnection& conn) {conn._readBuf(&boneIdx, 8);}
                };
                std::vector<std::vector<SkinBind>> skins;
                std::vector<std::vector<Index>> skinBanks;

                /* Islands of the same material/skinBank are represented here */
                struct Surface
                {
                    Vector3f centroid;
                    Index materialIdx;
                    Vector3f aabbMin;
                    Vector3f aabbMax;
                    Vector3f reflectionNormal;
                    Index skinBankIdx;

                    /* Vertex indexing data */
                    struct Vert
                    {
                        uint32_t iPos;
                        uint32_t iNorm;
                        uint32_t iColor[4] = {uint32_t(-1)};
                        uint32_t iUv[8] = {uint32_t(-1)};
                        uint32_t iSkin;

                        Vert(BlenderConnection& conn, const Submesh& parent);
                    };
                    std::vector<Vert> verts;

                    Surface(BlenderConnection& conn, const Submesh& parent);
                };
                std::vector<Surface> surfaces;

                Submesh(BlenderConnection& conn);
            };
            std::vector<Submesh> submeshes;

            Mesh(BlenderConnection& conn);
        };

        /* Compile mesh by name */
        Mesh compileMesh(const std::string& name, int maxIdx=65535, int maxSkinBanks=10)
        {
            char req[128];
            snprintf(req, 128, "MESHCOMPILE %s %d %d", name.c_str(), maxIdx, maxSkinBanks);
            m_parent->_writeLine(req);

            char readBuf[256];
            m_parent->_readLine(readBuf, 256);
            if (strcmp(readBuf, "OK"))
                BlenderLog.report(LogVisor::FatalError, "unable to cook mesh '%s': %s", name.c_str(), readBuf);

            return Mesh(*m_parent);
        }

        /* Compile all meshes into one */
        Mesh compileAllMeshes(int maxIdx=65535, int maxSkinBanks=10)
        {
            char req[128];
            snprintf(req, 128, "MESHCOMPILEALL %d %d", maxIdx, maxSkinBanks);
            m_parent->_writeLine(req);

            char readBuf[256];
            m_parent->_readLine(readBuf, 256);
            if (strcmp(readBuf, "OK"))
                BlenderLog.report(LogVisor::FatalError, "unable to cook all meshes: %s", readBuf);

            return Mesh(*m_parent);
        }
    };
    DataStream beginData()
    {
        if (m_lock)
            BlenderLog.report(LogVisor::FatalError, "lock already held for BlenderConnection::beginDataIn()");
        return DataStream(this);
    }

    void quitBlender();

    static BlenderConnection& SharedConnection()
    {
        if (!SharedBlenderConnection)
            SharedBlenderConnection = new BlenderConnection(HECL::VerbosityLevel);
        return *SharedBlenderConnection;
    }

    void closeStream()
    {
        if (m_lock)
            deleteBlend();
    }

    static void Shutdown()
    {
        if (SharedBlenderConnection)
        {
            SharedBlenderConnection->closeStream();
            SharedBlenderConnection->quitBlender();
            delete SharedBlenderConnection;
            SharedBlenderConnection = nullptr;
            BlenderLog.report(LogVisor::Info, "BlenderConnection Shutdown Successful");
        }
    }

};

}

#endif // BLENDERCONNECTION_HPP
