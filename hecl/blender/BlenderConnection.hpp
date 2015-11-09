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
#include <iostream>
#include <unordered_map>

#include "HECL/HECL.hpp"
#include <Athena/Types.hpp>

namespace HECL
{

extern LogVisor::LogModule BlenderLog;
extern class BlenderConnection* SharedBlenderConnection;

class BlenderConnection
{
public:
    enum BlendType
    {
        TypeNone,
        TypeMesh,
        TypeActor,
        TypeArea,
        TypeWorld,
        TypeMapArea,
        TypeMapUniverse
    };
private:
    bool m_lock = false;
#if _WIN32
    HANDLE m_blenderProc;
#else
    pid_t m_blenderProc;
#endif
    int m_readpipe[2];
    int m_writepipe[2];
    BlendType m_loadedType = TypeNone;
    ProjectPath m_loadedBlend;
    std::string m_startupBlend;
    size_t _readLine(char* buf, size_t bufSz);
    size_t _writeLine(const char* buf);
    size_t _readBuf(void* buf, size_t len);
    size_t _writeBuf(const void* buf, size_t len);
    void _closePipe();
public:
    BlenderConnection(int verbosityLevel=1);
    ~BlenderConnection();

    bool createBlend(const ProjectPath& path, BlendType type);
    BlendType getBlendType() const {return m_loadedType;}
    bool openBlend(const ProjectPath& path, bool force=false);
    bool saveBlend();
    void deleteBlend();

    enum ANIMCurveType
    {
        CurveRotate,
        CurveTranslate,
        CurveScale
    };

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
        void linkBlend(const char* target, const char* objName, bool link=true);
        void linkBackground(const char* target, const char* sceneName);

        void AABBToBMesh(const atVec3f& min, const atVec3f& max)
        {
            format("bm = bmesh.new()\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.new((%f,%f,%f))\n"
                   "bm.verts.ensure_lookup_table()\n"
                   "bm.edges.new((bm.verts[0], bm.verts[1]))\n"
                   "bm.edges.new((bm.verts[0], bm.verts[2]))\n"
                   "bm.edges.new((bm.verts[0], bm.verts[4]))\n"
                   "bm.edges.new((bm.verts[3], bm.verts[1]))\n"
                   "bm.edges.new((bm.verts[3], bm.verts[2]))\n"
                   "bm.edges.new((bm.verts[3], bm.verts[7]))\n"
                   "bm.edges.new((bm.verts[5], bm.verts[1]))\n"
                   "bm.edges.new((bm.verts[5], bm.verts[4]))\n"
                   "bm.edges.new((bm.verts[5], bm.verts[7]))\n"
                   "bm.edges.new((bm.verts[6], bm.verts[2]))\n"
                   "bm.edges.new((bm.verts[6], bm.verts[4]))\n"
                   "bm.edges.new((bm.verts[6], bm.verts[7]))\n",
                   min.vec[0], min.vec[1], min.vec[2],
                   max.vec[0], min.vec[1], min.vec[2],
                   min.vec[0], max.vec[1], min.vec[2],
                   max.vec[0], max.vec[1], min.vec[2],
                   min.vec[0], min.vec[1], max.vec[2],
                   max.vec[0], min.vec[1], max.vec[2],
                   min.vec[0], max.vec[1], max.vec[2],
                   max.vec[0], max.vec[1], max.vec[2]);
        }

        void centerView()
        {
            *this << "bpy.context.user_preferences.view.smooth_view = 0\n"
                     "for window in bpy.context.window_manager.windows:\n"
                     "    screen = window.screen\n"
                     "    for area in screen.areas:\n"
                     "        if area.type == 'VIEW_3D':\n"
                     "            for region in area.regions:\n"
                     "                if region.type == 'WINDOW':\n"
                     "                    override = {'scene': bpy.context.scene, 'window': window, 'screen': screen, 'area': area, 'region': region}\n"
                     "                    bpy.ops.view3d.view_all(override)\n"
                     "                    break\n";
        }

        class ANIMOutStream
        {
            BlenderConnection* m_parent;
            unsigned m_curCount = 0;
            unsigned m_totalCount = 0;
            bool m_inCurve = false;
        public:
            using CurveType = ANIMCurveType;
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
            for (uint32_t i=0 ; i<count ; ++i)
            {
                char name[128];
                m_parent->_readLine(name, 128);
                retval.push_back(name);
            }
            return retval;
        }

        /* Vector types with integrated stream reading constructor */
        struct Vector2f
        {
            atVec2f val;
            Vector2f() = default;
            void read(BlenderConnection& conn) {conn._readBuf(&val, 8);}
            Vector2f(BlenderConnection& conn) {read(conn);}
            operator const atVec2f&() const {return val;}
        };
        struct Vector3f
        {
            atVec3f val;
            Vector3f() = default;
            void read(BlenderConnection& conn) {conn._readBuf(&val, 12);}
            Vector3f(BlenderConnection& conn) {read(conn);}
            operator const atVec3f&() const {return val;}
        };
        struct Vector4f
        {
            atVec4f val;
            Vector4f() = default;
            void read(BlenderConnection& conn) {conn._readBuf(&val, 16);}
            Vector4f(BlenderConnection& conn) {read(conn);}
            operator const atVec4f&() const {return val;}
        };
        struct Index
        {
            uint32_t val;
            Index() = default;
            void read(BlenderConnection& conn) {conn._readBuf(&val, 4);}
            Index(BlenderConnection& conn) {read(conn);}
            operator const uint32_t&() const {return val;}
        };

        /** Intermediate mesh representation prepared by blender from a single mesh object */
        struct Mesh
        {
            enum OutputMode
            {
                OutputTriangles,
                OutputTriStrips,
            } outputMode;

            /* Cumulative AABB */
            Vector3f aabbMin;
            Vector3f aabbMax;

            /** HECL source and metadata of each material */
            struct Material
            {
                std::string name;
                std::string source;
                std::vector<ProjectPath> texs;
                std::unordered_map<std::string, int32_t> iprops;

                Material(BlenderConnection& conn);
            };
            std::vector<std::vector<Material>> materialSets;

            /* Vertex buffer data */
            std::vector<Vector3f> pos;
            std::vector<Vector3f> norm;
            uint32_t colorLayerCount = 0;
            std::vector<Vector3f> color;
            uint32_t uvLayerCount = 0;
            std::vector<Vector2f> uv;

            /* Skinning data */
            std::vector<std::string> boneNames;
            struct SkinBind
            {
                uint32_t boneIdx;
                float weight;
                SkinBind(BlenderConnection& conn) {conn._readBuf(&boneIdx, 8);}
            };
            std::vector<std::vector<SkinBind>> skins;
            std::vector<size_t> contiguousSkinVertCounts;

            /** Islands of the same material/skinBank are represented here */
            struct Surface
            {
                Vector3f centroid;
                Index materialIdx;
                Vector3f aabbMin;
                Vector3f aabbMax;
                Vector3f reflectionNormal;
                uint32_t skinBankIdx;

                /** Vertex indexing data (all primitives joined as degenerate tri-strip) */
                struct Vert
                {
                    uint32_t iPos;
                    uint32_t iNorm;
                    uint32_t iColor[4] = {uint32_t(-1)};
                    uint32_t iUv[8] = {uint32_t(-1)};
                    uint32_t iSkin;
                    uint32_t iBankSkin = -1;

                    Vert(BlenderConnection& conn, const Mesh& parent);
                };
                std::vector<Vert> verts;

                Surface(BlenderConnection& conn, Mesh& parent, int skinSlotCount);
            };
            std::vector<Surface> surfaces;

            struct SkinBanks
            {
                std::vector<std::vector<uint32_t>> banks;
                std::vector<std::vector<uint32_t>>::iterator addSkinBank(int skinSlotCount)
                {
                    banks.emplace_back();
                    if (skinSlotCount > 0)
                        banks.back().reserve(skinSlotCount);
                    return banks.end() - 1;
                }
                uint32_t addSurface(const Surface& surf, int skinSlotCount);
            } skinBanks;

            using SurfProgFunc = std::function<void(int)>;
            Mesh(BlenderConnection& conn, OutputMode outMode, int skinSlotCount, SurfProgFunc& surfProg);

            Mesh getContiguousSkinningVersion() const;
        };


        static const char* MeshOutputModeString(Mesh::OutputMode mode)
        {
            static const char* STRS[] = {"TRIANGLES", "TRISTRIPS"};
            return STRS[mode];
        }


        /** Compile mesh by context (MESH blends only) */
        Mesh compileMesh(Mesh::OutputMode outMode, int skinSlotCount=10,
                         Mesh::SurfProgFunc surfProg=[](int){})
        {
            if (m_parent->m_loadedType != TypeMesh)
                BlenderLog.report(LogVisor::FatalError, _S("%s is not a MESH blend"),
                                  m_parent->m_loadedBlend.getAbsolutePath().c_str());

            char req[128];
            snprintf(req, 128, "MESHCOMPILE %s %d",
                     MeshOutputModeString(outMode), skinSlotCount);
            m_parent->_writeLine(req);

            char readBuf[256];
            m_parent->_readLine(readBuf, 256);
            if (strcmp(readBuf, "OK"))
                BlenderLog.report(LogVisor::FatalError, "unable to cook mesh: %s", readBuf);

            return Mesh(*m_parent, outMode, skinSlotCount, surfProg);
        }

        /** Compile mesh by name (AREA blends only) */
        Mesh compileMesh(const std::string& name, Mesh::OutputMode outMode, int skinSlotCount=10,
                         Mesh::SurfProgFunc surfProg=[](int){})
        {
            if (m_parent->m_loadedType != TypeArea)
                BlenderLog.report(LogVisor::FatalError, _S("%s is not an AREA blend"),
                                  m_parent->m_loadedBlend.getAbsolutePath().c_str());

            char req[128];
            snprintf(req, 128, "MESHCOMPILENAME %s %s %d", name.c_str(),
                     MeshOutputModeString(outMode), skinSlotCount);
            m_parent->_writeLine(req);

            char readBuf[256];
            m_parent->_readLine(readBuf, 256);
            if (strcmp(readBuf, "OK"))
                BlenderLog.report(LogVisor::FatalError, "unable to cook mesh '%s': %s", name.c_str(), readBuf);

            return Mesh(*m_parent, outMode, skinSlotCount, surfProg);
        }

        /** Compile all meshes into one (AREA blends only) */
        Mesh compileAllMeshes(Mesh::OutputMode outMode, int skinSlotCount=10, float maxOctantLength=5.0,
                              Mesh::SurfProgFunc surfProg=[](int){})
        {
            if (m_parent->m_loadedType != TypeArea)
                BlenderLog.report(LogVisor::FatalError, _S("%s is not an AREA blend"),
                                  m_parent->m_loadedBlend.getAbsolutePath().c_str());

            char req[128];
            snprintf(req, 128, "MESHCOMPILEALL %s %d %f",
                     MeshOutputModeString(outMode),
                     skinSlotCount, maxOctantLength);
            m_parent->_writeLine(req);

            char readBuf[256];
            m_parent->_readLine(readBuf, 256);
            if (strcmp(readBuf, "OK"))
                BlenderLog.report(LogVisor::FatalError, "unable to cook all meshes: %s", readBuf);

            return Mesh(*m_parent, outMode, skinSlotCount, surfProg);
        }

        /** Intermediate actor representation prepared by blender from a single HECL actor blend */
        struct Actor
        {
            struct Armature
            {
                std::string name;
                struct Bone
                {
                    std::string name;
                    Vector3f origin;
                    int32_t parent = -1;
                    std::vector<int32_t> children;
                    Bone(BlenderConnection& conn);
                };
                std::vector<Bone> bones;
                Bone* lookupBone(const char* name)
                {
                    for (Bone& b : bones)
                        if (!b.name.compare(name))
                            return &b;
                    return nullptr;
                }
                Armature(BlenderConnection& conn);
            };
            std::vector<Armature> armatures;

            struct Subtype
            {
                std::string name;
                ProjectPath mesh;
                int32_t armature = -1;
                std::vector<std::pair<std::string, ProjectPath>> overlayMeshes;
                Subtype(BlenderConnection& conn);
            };
            std::vector<Subtype> subtypes;

            struct Action
            {
                std::string name;
                float interval;
                bool additive;
                std::vector<int32_t> frames;
                struct Channel
                {
                    std::string boneName;
                    uint32_t attrMask;
                    struct Key
                    {
                        Vector4f rotation;
                        Vector3f position;
                        Vector3f scale;
                        Key(BlenderConnection& conn, uint32_t attrMask);
                    };
                    std::vector<Key> keys;
                    Channel(BlenderConnection& conn);
                };
                std::vector<Channel> channels;
                std::vector<std::pair<Vector3f, Vector3f>> subtypeAABBs;
                Action(BlenderConnection& conn);
            };
            std::vector<Action> actions;

            Actor(BlenderConnection& conn);
        };

        Actor compileActor()
        {
            if (m_parent->m_loadedType != TypeActor)
                BlenderLog.report(LogVisor::FatalError, _S("%s is not an ACTOR blend"),
                                  m_parent->m_loadedBlend.getAbsolutePath().c_str());

            m_parent->_writeLine("ACTORCOMPILE");

            char readBuf[256];
            m_parent->_readLine(readBuf, 256);
            if (strcmp(readBuf, "OK"))
                BlenderLog.report(LogVisor::FatalError, "unable to compile actor: %s", readBuf);

            return Actor(*m_parent);
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
