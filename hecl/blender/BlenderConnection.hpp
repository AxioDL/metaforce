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
#include <float.h>
#include <string>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "hecl/hecl.hpp"
#include "hecl/HMDLMeta.hpp"
#include <athena/Types.hpp>
#include <athena/MemoryWriter.hpp>
#include <optional.hpp>

namespace hecl
{

extern logvisor::Module BlenderLog;
extern class BlenderToken SharedBlenderToken;
class HMDLBuffers;

class BlenderConnection
{
public:
    enum class BlendType
    {
        None,
        Mesh,
        Actor,
        Area,
        World,
        MapArea,
        MapUniverse,
        Frame
    };
private:
    bool m_lock = false;
#if _WIN32
    HANDLE m_blenderProc = 0;
    PROCESS_INFORMATION m_pinfo = {};
#else
    pid_t m_blenderProc = 0;
#endif
    int m_readpipe[2];
    int m_writepipe[2];
    bool m_hasSlerp;
    BlendType m_loadedType = BlendType::None;
    bool m_loadedRigged = false;
    ProjectPath m_loadedBlend;
    std::string m_startupBlend;
    hecl::SystemString m_errPath;
    size_t _readLine(char* buf, size_t bufSz);
    size_t _writeLine(const char* buf);
    size_t _readBuf(void* buf, size_t len);
    size_t _writeBuf(const void* buf, size_t len);
    void _closePipe();
    void _blenderDied();

public:
    BlenderConnection(int verbosityLevel=1);
    ~BlenderConnection();

    bool hasSLERP() const {return m_hasSlerp;}

    bool createBlend(const ProjectPath& path, BlendType type);
    BlendType getBlendType() const {return m_loadedType;}
    bool getRigged() const {return m_loadedRigged;}
    bool openBlend(const ProjectPath& path, bool force=false);
    bool saveBlend();
    void deleteBlend();

    enum class ANIMCurveType
    {
        Rotate,
        Translate,
        Scale
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
            int_type overflow(int_type ch);
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
                BlenderLog.report(logvisor::Fatal, "unable to open PyOutStream with blender");
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
                    BlenderLog.report(logvisor::Fatal, "unable to close PyOutStream with blender");
                m_parent->m_lock = false;
            }
        }
#if __GNUC__
        __attribute__((__format__ (__printf__, 2, 3)))
#endif
        void format(const char* fmt, ...)
        {
            if (!m_parent || !m_parent->m_lock)
                BlenderLog.report(logvisor::Fatal, "lock not held for PyOutStream::format()");
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
            *this << "for obj in bpy.context.scene.objects:\n"
                     "    if obj.type == 'CAMERA' or obj.type == 'LAMP':\n"
                     "        obj.hide = True\n"
                     "\n"
                     "bpy.context.user_preferences.view.smooth_view = 0\n"
                     "for window in bpy.context.window_manager.windows:\n"
                     "    screen = window.screen\n"
                     "    for area in screen.areas:\n"
                     "        if area.type == 'VIEW_3D':\n"
                     "            for region in area.regions:\n"
                     "                if region.type == 'WINDOW':\n"
                     "                    override = {'scene': bpy.context.scene, 'window': window, 'screen': screen, 'area': area, 'region': region}\n"
                     "                    bpy.ops.view3d.view_all(override)\n"
                     "                    break\n"
                     "\n"
                     "for obj in bpy.context.scene.objects:\n"
                     "    if obj.type == 'CAMERA' or obj.type == 'LAMP':\n"
                     "        obj.hide = False\n";
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
                    BlenderLog.report(logvisor::Fatal, "unable to open ANIMOutStream");
            }
            ~ANIMOutStream()
            {
                char tp = -1;
                m_parent->_writeBuf(&tp, 1);
                char readBuf[16];
                m_parent->_readLine(readBuf, 16);
                if (strcmp(readBuf, "ANIMDONE"))
                    BlenderLog.report(logvisor::Fatal, "unable to close ANIMOutStream");
            }
            void changeCurve(CurveType type, unsigned crvIdx, unsigned keyCount)
            {
                if (m_curCount != m_totalCount)
                    BlenderLog.report(logvisor::Fatal, "incomplete ANIMOutStream for change");
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
                    BlenderLog.report(logvisor::Fatal, "changeCurve not called before write");
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
                    BlenderLog.report(logvisor::Fatal, "ANIMOutStream keyCount overflow");
            }
        };
        ANIMOutStream beginANIMCurve()
        {
            return ANIMOutStream(m_parent);
        }
        BlenderConnection& getConnection() {return *m_parent;}
    };
    PyOutStream beginPythonOut(bool deleteOnError=false)
    {
        if (m_lock)
            BlenderLog.report(logvisor::Fatal, "lock already held for BlenderConnection::beginPythonOut()");
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
                BlenderLog.report(logvisor::Fatal, "unable to open DataStream with blender");
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
                    BlenderLog.report(logvisor::Fatal, "unable to close DataStream with blender");
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
        struct Matrix4f
        {
            atVec4f val[4];
            Matrix4f() = default;
            void read(BlenderConnection& conn) {conn._readBuf(&val, 64);}
            Matrix4f(BlenderConnection& conn) {read(conn);}
            const atVec4f& operator[] (size_t idx) const {return val[idx];}
        };
        struct Index
        {
            uint32_t val;
            Index() = default;
            void read(BlenderConnection& conn) {conn._readBuf(&val, 4);}
            Index(BlenderConnection& conn) {read(conn);}
            operator const uint32_t&() const {return val;}
        };

        static atVec3f MtxVecMul4RM(const Matrix4f& mtx, const Vector3f& vec);
        static atVec3f MtxVecMul3RM(const Matrix4f& mtx, const Vector3f& vec);

        /** Intermediate mesh representation prepared by blender from a single mesh object */
        struct Mesh
        {
            HMDLTopology topology;

            /* Object transform in scene */
            Matrix4f sceneXf;

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
                bool operator==(const Material& other) const
                {
                    return source == other.source && texs == other.texs && iprops == other.iprops;
                }
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

            void normalizeSkinBinds();

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

                    bool operator==(const Vert& other) const
                    {
                        if (iPos != other.iPos)
                            return false;
                        if (iNorm != other.iNorm)
                            return false;
                        for (int i=0 ; i<4 ; ++i)
                            if (iColor[i] != other.iColor[i])
                                return false;
                        for (int i=0 ; i<8 ; ++i)
                            if (iUv[i] != other.iUv[i])
                                return false;
                        if (iSkin != other.iSkin)
                            return false;
                        return true;
                    }
                };
                std::vector<Vert> verts;

                Surface(BlenderConnection& conn, Mesh& parent, int skinSlotCount);
            };
            std::vector<Surface> surfaces;

            std::unordered_map<std::string, std::string> customProps;

            struct SkinBanks
            {
                struct Bank
                {
                    std::vector<uint32_t> m_skinIdxs;
                    std::vector<uint32_t> m_boneIdxs;

                    void addSkins(const Mesh& parent, const std::vector<uint32_t>& skinIdxs)
                    {
                        for (uint32_t sidx : skinIdxs)
                        {
                            m_skinIdxs.push_back(sidx);
                            for (const SkinBind& bind : parent.skins[sidx])
                            {
                                bool found = false;
                                for (uint32_t bidx : m_boneIdxs)
                                {
                                    if (bidx == bind.boneIdx)
                                    {
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found)
                                    m_boneIdxs.push_back(bind.boneIdx);
                            }
                        }
                    }

                    size_t lookupLocalBoneIdx(uint32_t boneIdx) const
                    {
                        for (size_t i=0 ; i<m_boneIdxs.size() ; ++i)
                            if (m_boneIdxs[i] == boneIdx)
                                return i;
                        return -1;
                    }
                };
                std::vector<Bank> banks;
                std::vector<Bank>::iterator addSkinBank(int skinSlotCount)
                {
                    banks.emplace_back();
                    if (skinSlotCount > 0)
                        banks.back().m_skinIdxs.reserve(skinSlotCount);
                    return banks.end() - 1;
                }
                uint32_t addSurface(const Mesh& mesh, const Surface& surf, int skinSlotCount);
            } skinBanks;

            using SurfProgFunc = std::function<void(int)>;
            Mesh(BlenderConnection& conn, HMDLTopology topology, int skinSlotCount, SurfProgFunc& surfProg);

            Mesh getContiguousSkinningVersion() const;

            /** Prepares mesh representation for indexed access on modern APIs.
             *  Mesh must remain resident for accessing reference members
             */
            HMDLBuffers getHMDLBuffers(bool absoluteCoords) const;
        };

        /** Intermediate collision mesh representation prepared by blender from a single mesh object */
        struct ColMesh
        {
            /** HECL source and metadata of each material */
            struct Material
            {
                std::string name;
                uint32_t type;
                bool fireThrough;
                Material(BlenderConnection& conn);
            };
            std::vector<Material> materials;

            std::vector<Vector3f> verts;

            struct Edge
            {
                uint32_t verts[2];
                bool seam;
                Edge(BlenderConnection& conn);
            };
            std::vector<Edge> edges;

            struct Triangle
            {
                uint32_t edges[3];
                uint32_t matIdx;
                Triangle(BlenderConnection& conn);
            };
            std::vector<Triangle> trianges;

            ColMesh(BlenderConnection& conn);
        };

        /** Intermediate lamp representation */
        struct Light
        {
            /* Object transform in scene */
            Matrix4f sceneXf;
            Vector3f color;

            uint32_t layer;

            enum class Type : uint32_t
            {
                Ambient,
                Directional,
                Custom,
                Spot
            } type;

            float energy;
            float spotCutoff;
            float constant;
            float linear;
            float quadratic;
            bool shadow;

            Light(BlenderConnection& conn);
        };

        static const char* MeshOutputModeString(HMDLTopology topology)
        {
            static const char* STRS[] = {"TRIANGLES", "TRISTRIPS"};
            return STRS[int(topology)];
        }


        /** Compile mesh by context (MESH blends only) */
        Mesh compileMesh(HMDLTopology topology, int skinSlotCount=10,
                         Mesh::SurfProgFunc surfProg=[](int){});

        /** Compile mesh by name (AREA blends only) */
        Mesh compileMesh(const std::string& name, HMDLTopology topology, int skinSlotCount=10,
                         Mesh::SurfProgFunc surfProg=[](int){});

        /** Compile collision mesh by name (AREA blends only) */
        ColMesh compileColMesh(const std::string& name);

        /** Compile all meshes into one (AREA blends only) */
        Mesh compileAllMeshes(HMDLTopology topology, int skinSlotCount=10, float maxOctantLength=5.0,
                              Mesh::SurfProgFunc surfProg=[](int){});

        /** Gather all lights in scene (AREA blends only) */
        std::vector<Light> compileLights();

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
                const Bone* lookupBone(const char* name) const
                {
                    for (const Bone& b : bones)
                        if (!b.name.compare(name))
                            return &b;
                    return nullptr;
                }
                const Bone* getParent(const Bone* bone) const
                {
                    if (bone->parent < 0)
                        return nullptr;
                    return &bones[bone->parent];
                }
                const Bone* getChild(const Bone* bone, size_t child) const
                {
                    if (child >= bone->children.size())
                        return nullptr;
                    int32_t cIdx = bone->children[child];
                    if (cIdx < 0)
                        return nullptr;
                    return &bones[cIdx];
                }
                const Bone* getRoot() const
                {
                    for (const Bone& b : bones)
                        if (b.parent < 0)
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
                bool looping;
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

        Actor compileActor();
        std::vector<std::string> getArmatureNames();
        std::vector<std::string> getSubtypeNames();
        std::vector<std::string> getActionNames();

        struct Matrix3f
        {
            atVec3f m[3];
            inline atVec3f& operator[](size_t idx) {return m[idx];}
            inline const atVec3f& operator[](size_t idx) const {return m[idx];}
        };
        std::unordered_map<std::string, Matrix3f> getBoneMatrices(const std::string& name);
    };
    DataStream beginData()
    {
        if (m_lock)
            BlenderLog.report(logvisor::Fatal, "lock already held for BlenderConnection::beginDataIn()");
        return DataStream(this);
    }

    void quitBlender();

    void closeStream()
    {
        if (m_lock)
            deleteBlend();
    }

    static BlenderConnection& SharedConnection();
    static void Shutdown();
};

class BlenderToken
{
    std::experimental::optional<BlenderConnection> m_conn;
public:
    BlenderConnection& getBlenderConnection()
    {
        if (!m_conn)
            m_conn.emplace(hecl::VerbosityLevel);
        return *m_conn;
    }
    void shutdown()
    {
        if (m_conn)
        {
            m_conn->quitBlender();
            m_conn = std::experimental::nullopt;
            BlenderLog.report(logvisor::Info, "BlenderConnection Shutdown Successful");
        }
    }
};

class HMDLBuffers
{
public:
    struct Surface;
private:
    friend struct BlenderConnection::DataStream::Mesh;
    HMDLBuffers(HMDLMeta&& meta,
                size_t vboSz, const std::vector<atUint32>& iboData,
                std::vector<Surface>&& surfaces,
                const BlenderConnection::DataStream::Mesh::SkinBanks& skinBanks)
    : m_meta(std::move(meta)),
      m_vboSz(vboSz), m_vboData(new uint8_t[vboSz]),
      m_iboSz(iboData.size()*4), m_iboData(new uint8_t[iboData.size()*4]),
      m_surfaces(std::move(surfaces)), m_skinBanks(skinBanks)
    {
        athena::io::MemoryWriter w(m_iboData.get(), m_iboSz);
        w.enumerateLittle(iboData);
    }
public:
    HMDLMeta m_meta;
    size_t m_vboSz;
    std::unique_ptr<uint8_t[]> m_vboData;
    size_t m_iboSz;
    std::unique_ptr<uint8_t[]> m_iboData;

    struct Surface
    {
        Surface(const BlenderConnection::DataStream::Mesh::Surface& origSurf,
                atUint32 start, atUint32 count)
        : m_origSurf(origSurf), m_start(start), m_count(count) {}
        const BlenderConnection::DataStream::Mesh::Surface& m_origSurf;
        atUint32 m_start;
        atUint32 m_count;
    };
    std::vector<Surface> m_surfaces;

    const BlenderConnection::DataStream::Mesh::SkinBanks& m_skinBanks;
};

}

#endif // BLENDERCONNECTION_HPP
